/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qbs.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include "buildgraphloader.h"

#include "buildgraph.h"
#include "emptydirectoriesremover.h"
#include "productbuilddata.h"
#include "projectbuilddata.h"
#include "rulenode.h"
#include "rulecommands.h"
#include "transformer.h"

#include <buildgraph/rulesevaluationcontext.h>
#include <language/artifactproperties.h>
#include <language/language.h>
#include <language/propertymapinternal.h>
#include <language/qualifiedid.h>
#include <language/resolvedfilecontext.h>
#include <loader/projectresolver.h>
#include <logging/categories.h>
#include <logging/translator.h>
#include <tools/buildgraphlocker.h>
#include <tools/fileinfo.h>
#include <tools/jsliterals.h>
#include <tools/persistence.h>
#include <tools/profile.h>
#include <tools/profiling.h>
#include <tools/qbsassert.h>
#include <tools/qttools.h>
#include <tools/settings.h>
#include <tools/stlutils.h>
#include <tools/stringconstants.h>

#include <QtCore/qdir.h>
#include <QtCore/qfileinfo.h>

#include <algorithm>
#include <functional>
#include <memory>
#include <unordered_map>

namespace qbs {
namespace Internal {

BuildGraphLoader::BuildGraphLoader(Logger logger) :
    m_logger(std::move(logger))
{
}

BuildGraphLoader::~BuildGraphLoader()
{
    qDeleteAll(m_objectsToDelete);
}

static void restoreBackPointers(const ResolvedProjectPtr &project)
{
    for (const ResolvedProductPtr &product : project->products) {
        product->project = project;
        if (!product->buildData)
            continue;
        for (BuildGraphNode * const n : std::as_const(product->buildData->allNodes())) {
            if (n->type() == BuildGraphNode::ArtifactNodeType) {
                project->topLevelProject()->buildData
                        ->insertIntoLookupTable(static_cast<Artifact *>(n));
            }
        }
    }

    for (const ResolvedProjectPtr &subProject : std::as_const(project->subProjects)) {
        subProject->parentProject = project;
        restoreBackPointers(subProject);
    }
}

BuildGraphLoadResult BuildGraphLoader::load(const TopLevelProjectPtr &existingProject,
                                            const SetupProjectParameters &parameters,
                                            const RulesEvaluationContextPtr &evalContext)
{
    m_parameters = parameters;
    m_result = BuildGraphLoadResult();
    m_evalContext = evalContext;

    if (existingProject) {
        QBS_CHECK(existingProject->buildData);
        existingProject->buildData->evaluationContext = evalContext;
        if (!checkBuildGraphCompatibility(existingProject))
            return m_result;
        m_result.loadedProject = existingProject;
    } else {
        loadBuildGraphFromDisk();
    }
    if (!m_result.loadedProject)
        return m_result;
    if (parameters.restoreBehavior() == SetupProjectParameters::RestoreOnly) {
        for (const ErrorInfo &e : std::as_const(m_result.loadedProject->warningsEncountered))
            m_logger.printWarning(e);
        return m_result;
    }
    QBS_CHECK(parameters.restoreBehavior() == SetupProjectParameters::RestoreAndTrackChanges);

    if (m_parameters.logElapsedTime()) {
        m_wildcardExpansionEffort = 0;
        m_propertyComparisonEffort = 0;
    }
    trackProjectChanges();
    if (m_parameters.logElapsedTime()) {
        m_logger.qbsLog(LoggerInfo, true) << "\t"
                << Tr::tr("Wildcard expansion took %1.")
                   .arg(elapsedTimeString(m_wildcardExpansionEffort));
        m_logger.qbsLog(LoggerInfo, true) << "\t"
                << Tr::tr("Comparing property values took %1.")
                   .arg(elapsedTimeString(m_propertyComparisonEffort));
    }
    return m_result;
}

TopLevelProjectConstPtr BuildGraphLoader::loadProject(const QString &bgFilePath)
{
    class LogSink : public ILogSink {
        void doPrintMessage(LoggerLevel, const QString &, const QString &) override { }
    } dummySink;
    Logger dummyLogger(&dummySink);
    BuildGraphLocker bgLocker(bgFilePath, dummyLogger, false, nullptr);
    PersistentPool pool(dummyLogger);
    pool.load(bgFilePath);
    const TopLevelProjectPtr project = TopLevelProject::create();
    project->load(pool);
    project->setBuildConfiguration(pool.headData().projectConfig);
    return project;
}

void BuildGraphLoader::loadBuildGraphFromDisk()
{
    const QString projectId = TopLevelProject::deriveId(m_parameters.finalBuildConfigurationTree());
    const QString buildDir
            = TopLevelProject::deriveBuildDirectory(m_parameters.buildRoot(), projectId);
    const QString buildGraphFilePath
            = ProjectBuildData::deriveBuildGraphFilePath(buildDir, projectId);

    PersistentPool pool(m_logger);
    qCDebug(lcBuildGraph) << "trying to load:" << buildGraphFilePath;
    try {
        pool.load(buildGraphFilePath);
    } catch (const NoBuildGraphError &) {
        if (m_parameters.restoreBehavior() == SetupProjectParameters::RestoreOnly)
            throw;
        m_logger.qbsInfo()
                << Tr::tr("Build graph does not yet exist for configuration '%1'. "
                          "Starting from scratch.").arg(m_parameters.configurationName());
        return;
    } catch (const ErrorInfo &loadError) {
        if (!m_parameters.overrideBuildGraphData()) {
            ErrorInfo fullError = loadError;
            fullError.append(Tr::tr("Use the 'resolve' command to set up a new build graph."));
            throw fullError;
        }
        m_logger.qbsWarning() << loadError.toString();
        return;
    }

    const TopLevelProjectPtr project = TopLevelProject::create();

    // TODO: Store some meta data that will enable us to show actual progress (e.g. number of products).
    m_evalContext->initializeObserver(Tr::tr("Restoring build graph from disk"), 1);

    project->load(pool);
    project->buildData->evaluationContext = m_evalContext;
    project->setBuildConfiguration(pool.headData().projectConfig);
    project->buildDirectory = buildDir;
    if (!checkBuildGraphCompatibility(project))
        return;
    restoreBackPointers(project);
    project->buildData->setClean();
    project->location = CodeLocation(m_parameters.projectFilePath(), project->location.line(),
                                     project->location.column());
    m_result.loadedProject = project;
    m_evalContext->incrementProgressValue();
    doSanityChecks(project, m_logger);
}

bool BuildGraphLoader::checkBuildGraphCompatibility(const TopLevelProjectConstPtr &project)
{
    if (m_parameters.projectFilePath().isEmpty())
        m_parameters.setProjectFilePath(project->location.filePath());
    else
        m_parameters.finalizeProjectFilePath();
    if (QFileInfo(project->location.filePath()) == QFileInfo(m_parameters.projectFilePath()))
        return true;
    QString message = Tr::tr("Stored build graph at '%1' is for project file '%2', but "
                             "input file is '%3'.")
            .arg(QDir::toNativeSeparators(project->buildGraphFilePath()),
                 QDir::toNativeSeparators(project->location.filePath()),
                 QDir::toNativeSeparators(m_parameters.projectFilePath()));
    if (m_parameters.overrideBuildGraphData()) {
        m_logger.qbsInfo() << message;
        return false;
    }
    message.append(QLatin1Char('\n')).append(Tr::tr("Use the 'resolve' command to enforce "
                                                    "using a different project file."));
    throw ErrorInfo(message);
}

static bool checkProductForChangedDependency(std::vector<ResolvedProductPtr> &changedProducts,
        Set<ResolvedProductPtr> &seenProducts, const ResolvedProductPtr &product)
{
    if (seenProducts.contains(product))
        return false;
    if (contains(changedProducts, product))
        return true;
    for (const ResolvedProductPtr &dep : std::as_const(product->dependencies)) {
        if (checkProductForChangedDependency(changedProducts, seenProducts, dep)) {
            changedProducts << product;
            return true;
        }
    }
    seenProducts << product;
    return false;
}

// All products depending on changed products also become changed. Otherwise the output
// artifacts of the rules taking the artifacts from the dependency as inputs will be
// rebuilt due to their rule getting re-applied (as the rescued input artifacts will show
// up as newly added) and no rescue data being available.
static void makeChangedProductsListComplete(std::vector<ResolvedProductPtr> &changedProducts,
        const std::vector<ResolvedProductPtr> &allRestoredProducts)
{
    Set<ResolvedProductPtr> seenProducts;
    for (const ResolvedProductPtr &p : allRestoredProducts)
        checkProductForChangedDependency(changedProducts, seenProducts, p);
}

static void updateProductAndRulePointers(const ResolvedProductPtr &newProduct)
{
    std::unordered_map<RuleConstPtr, RuleConstPtr> ruleMap;
    for (BuildGraphNode *node : std::as_const(newProduct->buildData->allNodes())) {
        node->product = newProduct;
        const auto findNewRule = [&ruleMap, &newProduct]
                (const RuleConstPtr &oldRule) -> RuleConstPtr {
            const auto it = ruleMap.find(oldRule);
            if (it != ruleMap.cend())
                return it->second;
            for (const auto &r : std::as_const(newProduct->rules)) {
                if (*r == *oldRule) {
                    ruleMap.insert(std::make_pair(oldRule, r));
                    return r;
                }
            }
            QBS_CHECK(false);
            return {};
        };
        if (node->type() == BuildGraphNode::RuleNodeType) {
            const auto ruleNode = static_cast<RuleNode *>(node);
            ruleNode->setRule(findNewRule(ruleNode->rule()));
        } else {
            QBS_CHECK(node->type() == BuildGraphNode::ArtifactNodeType);
            const auto artifact = static_cast<const Artifact *>(node);
            if (artifact->artifactType == Artifact::Generated) {
                QBS_CHECK(artifact->transformer);
                artifact->transformer->rule = findNewRule(artifact->transformer->rule);
            }
        }
    }
}

void BuildGraphLoader::trackProjectChanges()
{
    TimedActivityLogger trackingTimer(m_logger, Tr::tr("Change tracking"),
                                      m_parameters.logElapsedTime());
    const TopLevelProjectPtr &restoredProject = m_result.loadedProject;
    Set<QString> buildSystemFiles = restoredProject->buildSystemFiles;
    std::vector<ResolvedProductPtr> allRestoredProducts = restoredProject->allProducts();
    std::vector<ResolvedProductPtr> changedProducts;
    bool reResolvingNecessary = false;
    if (!checkConfigCompatibility()) {
        m_logger.qbsInfo() << Tr::tr("One or more properties have changed.");
        reResolvingNecessary = true;
    }
    if (hasProductFileChanged(allRestoredProducts, restoredProject->lastStartResolveTime,
                              buildSystemFiles, changedProducts)) {
        reResolvingNecessary = true;
    }

    // "External" changes, e.g. in the environment or in a JavaScript file,
    // can make the list of source files in a product change without the respective file
    // having been touched. In such a case, the build data for that product will have to be set up
    // anew.
    if (probeExecutionForced(restoredProject, allRestoredProducts)
            || hasBuildSystemFileChanged(buildSystemFiles, restoredProject.get())
            || hasEnvironmentChanged(restoredProject)
            || hasCanonicalFilePathResultChanged(restoredProject)
            || hasFileExistsResultChanged(restoredProject)
            || hasDirectoryEntriesResultChanged(restoredProject)
            || hasFileLastModifiedResultChanged(restoredProject)) {
        reResolvingNecessary = true;
    }

    if (!reResolvingNecessary) {
        for (const ErrorInfo &e : std::as_const(restoredProject->warningsEncountered))
            m_logger.printWarning(e);
        return;
    }

    for (const QString &file : m_changedProjectFiles) {
        m_logger.qbsInfo() << Tr::tr("Project file '%1' has changed.")
                                  .arg(QDir::toNativeSeparators(file));
    }
    for (const QString &file : m_removedProjectFiles) {
        m_logger.qbsInfo() << Tr::tr("Project file '%1' was removed.")
                                  .arg(QDir::toNativeSeparators(file));
    }

    restoredProject->buildData->setDirty();
    markTransformersForChangeTracking(allRestoredProducts);
    if (!m_parameters.overrideBuildGraphData())
        m_parameters.setEnvironment(restoredProject->environment);
    ProjectResolver resolver(m_parameters, m_evalContext->engine(), m_logger);
    resolver.setProgressObserver(m_evalContext->observer());
    resolver.setOldProjectProbes(restoredProject->probes);
    if (!m_parameters.forceProbeExecution())
        resolver.setStoredModuleProviderInfo(restoredProject->moduleProviderInfo);
    resolver.setLastResolveTime(restoredProject->lastStartResolveTime);
    QHash<QString, std::vector<ProbeConstPtr>> restoredProbes;
    for (const auto &restoredProduct : std::as_const(allRestoredProducts))
        restoredProbes.insert(restoredProduct->uniqueName(), restoredProduct->probes);
    resolver.setOldProductProbes(restoredProbes);
    if (!m_parameters.overrideBuildGraphData())
        resolver.setStoredProfiles(restoredProject->profileConfigs);
    m_result.newlyResolvedProject = resolver.resolve();

    std::vector<ResolvedProductPtr> allNewlyResolvedProducts
            = m_result.newlyResolvedProject->allProducts();
    for (const ResolvedProductPtr &cp : std::as_const(allNewlyResolvedProducts))
        m_freshProductsByName.insert(cp->uniqueName(), cp);

    checkAllProductsForChanges(allRestoredProducts, changedProducts);

    std::shared_ptr<ProjectBuildData> oldBuildData;
    ChildListHash childLists;
    if (!changedProducts.empty()) {
        oldBuildData = std::make_shared<ProjectBuildData>(restoredProject->buildData.get());
        for (const auto &product : std::as_const(allRestoredProducts)) {
            if (!product->buildData)
                continue;

            // If the product gets temporarily removed, its artifacts will get disconnected
            // and this structural information will no longer be directly available from them.
            for (const Artifact *a : filterByType<Artifact>(product->buildData->allNodes())) {
                childLists.insert(a, ChildrenInfo(ArtifactSet::filtered(a->children),
                                                  a->childrenAddedByScanner));
            }
        }
    }

    makeChangedProductsListComplete(changedProducts, allRestoredProducts);

    // Set up build data from scratch for all changed products. This does not necessarily
    // mean that artifacts will have to get rebuilt; whether this is necesessary will be decided
    // an a per-artifact basis by the Executor on the next build.
    QHash<QString, AllRescuableArtifactData> rescuableArtifactData;
    for (const ResolvedProductPtr &product : std::as_const(changedProducts)) {
        const QString name = product->uniqueName();
        m_changedSourcesByProduct.erase(name);
        m_productsWhoseArtifactsNeedUpdate.remove(name);
        ResolvedProductPtr freshProduct = m_freshProductsByName.value(name);
        if (!freshProduct)
            continue;
        onProductRemoved(product, product->topLevelProject()->buildData.get(), false);
        if (product->buildData) {
            rescuableArtifactData.insert(product->uniqueName(),
                                         product->buildData->rescuableArtifactData());
        }
        removeOne(allRestoredProducts, product);
    }

    // Move over restored build data to newly resolved project.
    m_result.newlyResolvedProject->buildData.swap(restoredProject->buildData);
    QBS_CHECK(m_result.newlyResolvedProject->buildData);
    m_result.newlyResolvedProject->buildData->setDirty();

    for (auto it = allNewlyResolvedProducts.begin(); it != allNewlyResolvedProducts.end();) {
        const ResolvedProductPtr &newlyResolvedProduct = *it;
        auto k = std::find_if(allRestoredProducts.begin(), allRestoredProducts.end(),
                     [&newlyResolvedProduct](const ResolvedProductPtr &restoredProduct) {
            return newlyResolvedProduct->uniqueName() == restoredProduct->uniqueName();
        });
        if (k == allRestoredProducts.end()) {
            ++it;
        } else {
            const ResolvedProductPtr &restoredProduct = *k;
            if (newlyResolvedProduct->enabled)
                newlyResolvedProduct->buildData.swap(restoredProduct->buildData);
            if (newlyResolvedProduct->buildData)
                updateProductAndRulePointers(newlyResolvedProduct);

            // Keep in list if build data still needs to be resolved.
            if (!newlyResolvedProduct->enabled || newlyResolvedProduct->buildData)
                it = allNewlyResolvedProducts.erase(it);

            allRestoredProducts.erase(k);
        }
    }

    // Products still left in the list do not exist anymore.
    for (const ResolvedProductPtr &removedProduct : std::as_const(allRestoredProducts)) {
        removeOne(changedProducts, removedProduct);
        onProductRemoved(removedProduct, m_result.newlyResolvedProject->buildData.get());
    }

    // Products still left in the list need resolving, either because they are new
    // or because they are newly enabled.
    if (!allNewlyResolvedProducts.empty()) {
        BuildDataResolver bpr(m_logger);
        bpr.resolveProductBuildDataForExistingProject(m_result.newlyResolvedProject,
                                                      allNewlyResolvedProducts);
    }

    for (const auto &kv : m_changedSourcesByProduct) {
        const ResolvedProductPtr product = m_freshProductsByName.value(kv.first);
        QBS_CHECK(!!product);
        for (const SourceArtifactConstPtr &sa : kv.second)
            updateArtifactFromSourceArtifact(product, sa);
    }

    for (const QString &productName : m_productsWhoseArtifactsNeedUpdate) {
        const ResolvedProductPtr product = m_freshProductsByName.value(productName);
        QBS_CHECK(!!product);
        updateGeneratedArtifacts(product.get());
    }

    for (const auto &changedProduct : std::as_const(changedProducts)) {
        rescueOldBuildData(changedProduct,
                           m_freshProductsByName.value(changedProduct->uniqueName()),
                           childLists, rescuableArtifactData.value(changedProduct->uniqueName()));
    }

    EmptyDirectoriesRemover(m_result.newlyResolvedProject.get(), m_logger)
            .removeEmptyParentDirectories(m_artifactsRemovedFromDisk);

    for (FileResourceBase * const f : std::as_const(m_objectsToDelete)) {
        if (f->fileType() == FileResourceBase::FileTypeArtifact)
            static_cast<Artifact *>(f)->product.reset(); // To help with the sanity checks.
    }
    doSanityChecks(m_result.newlyResolvedProject, m_logger);
}

bool BuildGraphLoader::probeExecutionForced(
        const TopLevelProjectConstPtr &restoredProject,
        const std::vector<ResolvedProductPtr> &restoredProducts) const
{
    if (!m_parameters.forceProbeExecution())
        return false;

    if (!restoredProject->probes.empty())
        return true;

    return Internal::any_of(restoredProducts, [](const auto &p) {
        return !p->probes.empty();
    });
}

bool BuildGraphLoader::hasEnvironmentChanged(const TopLevelProjectConstPtr &restoredProject) const
{
    if (!m_parameters.overrideBuildGraphData())
        return false;
    QProcessEnvironment oldEnv = restoredProject->environment;
    QProcessEnvironment newEnv = m_parameters.adjustedEnvironment();

    static const QString ldPreloadEnvVar = QStringLiteral("LD_PRELOAD");
    // HACK. Valgrind screws up our null-build benchmarker otherwise.
    // TODO: Think about a (module-provided) whitelist.
    oldEnv.remove(ldPreloadEnvVar);
    newEnv.remove(ldPreloadEnvVar);

    if (oldEnv != newEnv) {
        m_logger.qbsInfo() << Tr::tr("Environment changed.");
        m_logger.qbsDebug() << Tr::tr("old: %1").arg(
            restoredProject->environment.toStringList().join(QLatin1Char('\n')));
        m_logger.qbsDebug() << Tr::tr("new: %2").arg(
            m_parameters.adjustedEnvironment().toStringList().join(QLatin1Char('\n')));
        return true;
    }
    return false;
}

bool BuildGraphLoader::hasCanonicalFilePathResultChanged(const TopLevelProjectConstPtr &restoredProject) const
{
    for (auto it = restoredProject->canonicalFilePathResults.constBegin();
         it != restoredProject->canonicalFilePathResults.constEnd(); ++it) {
        if (QFileInfo(it.key()).canonicalFilePath() != it.value()) {
            m_logger.qbsInfo() << Tr::tr("Canonical file path for file '%1' changed.")
                                      .arg(QDir::toNativeSeparators(it.key()));
            return true;
        }
    }

    return false;
}

bool BuildGraphLoader::hasFileExistsResultChanged(const TopLevelProjectConstPtr &restoredProject) const
{
    for (QHash<QString, bool>::ConstIterator it = restoredProject->fileExistsResults.constBegin();
         it != restoredProject->fileExistsResults.constEnd(); ++it) {
        if (FileInfo(it.key()).exists() != it.value()) {
            m_logger.qbsInfo() << Tr::tr("Existence check for file '%1' changed.")
                                      .arg(QDir::toNativeSeparators(it.key()));
            return true;
        }
    }

    return false;
}

bool BuildGraphLoader::hasDirectoryEntriesResultChanged(const TopLevelProjectConstPtr &restoredProject) const
{
    for (auto it = restoredProject->directoryEntriesResults.constBegin();
         it != restoredProject->directoryEntriesResults.constEnd(); ++it) {
        if (QDir(it.key().first).entryList(static_cast<QDir::Filters>(it.key().second), QDir::Name)
                != it.value()) {
            m_logger.qbsInfo() << Tr::tr("Entry list for directory '%1' changed.")
                                      .arg(QDir::toNativeSeparators(it.key().first));
            return true;
        }
    }

    return false;
}

bool BuildGraphLoader::hasFileLastModifiedResultChanged(const TopLevelProjectConstPtr &restoredProject) const
{
    for (QHash<QString, FileTime>::ConstIterator it
         = restoredProject->fileLastModifiedResults.constBegin();
         it != restoredProject->fileLastModifiedResults.constEnd(); ++it) {
        if (FileInfo(it.key()).lastModified() != it.value()) {
            m_logger.qbsInfo() << Tr::tr("Timestamp for file '%1' has changed.")
                                      .arg(QDir::toNativeSeparators(it.key()));
            return true;
        }
    }

    return false;
}

bool BuildGraphLoader::hasProductFileChanged(const std::vector<ResolvedProductPtr> &restoredProducts,
        const FileTime &referenceTime, Set<QString> &remainingBuildSystemFiles,
        std::vector<ResolvedProductPtr> &changedProducts)
{
    bool hasChanged = false;
    for (const ResolvedProductPtr &product : restoredProducts) {
        const QString filePath = product->location.filePath();
        const FileInfo pfi(filePath);
        remainingBuildSystemFiles.remove(filePath);
        if (!pfi.exists()) {
            m_removedProjectFiles << filePath;
            hasChanged = true;
        } else if (referenceTime < pfi.lastModified()) {
            m_changedProjectFiles << filePath;
            hasChanged = true;
        } else if (!contains(changedProducts, product)) {
            bool foundMissingSourceFile = false;
            for (const QString &file : std::as_const(product->missingSourceFiles)) {
                if (FileInfo(file).exists()) {
                    m_logger.qbsInfo()
                        << Tr::tr("Formerly missing file '%1' in product '%2' exists now.")
                               .arg(QDir::toNativeSeparators(filePath), product->fullDisplayName());
                    foundMissingSourceFile = true;
                    break;
                }
            }
            if (foundMissingSourceFile) {
                hasChanged = true;
                changedProducts.push_back(product);
                continue;
            }

            AccumulatingTimer wildcardTimer(m_parameters.logElapsedTime()
                                            ? &m_wildcardExpansionEffort : nullptr);
            for (const GroupPtr &group : product->groups) {
                if (group->wildcards && group->wildcards->hasChangedSinceExpansion()) {
                    m_logger.qbsInfo()
                        << Tr::tr("Must re-expand wildcards for group '%1' in product '%2'.")
                               .arg(group->name, product->fullDisplayName());
                    hasChanged = true;
                    changedProducts.push_back(product);
                    break;
                }
            }
        }
    }

    return hasChanged;
}

bool BuildGraphLoader::hasBuildSystemFileChanged(const Set<QString> &buildSystemFiles,
                                                 const TopLevelProject *restoredProject)
{
    for (const QString &file : buildSystemFiles) {
        const FileInfo fi(file);
        if (!fi.exists()) {
            m_removedProjectFiles << file;
            return true;
        }
        const auto generatedChecker = [&file, restoredProject](const ModuleProviderInfo &mpi) {
            return file.startsWith(mpi.outputDirPath(restoredProject->buildDirectory));
        };
        const bool fileWasCreatedByModuleProvider =
                any_of(restoredProject->moduleProviderInfo.providers, generatedChecker);
        const FileTime referenceTime = fileWasCreatedByModuleProvider
                ? restoredProject->lastEndResolveTime : restoredProject->lastStartResolveTime;
        if (referenceTime < fi.lastModified()) {
            m_changedProjectFiles << file;
            return true;
        }
    }
    return false;
}

void BuildGraphLoader::markTransformersForChangeTracking(
        const std::vector<ResolvedProductPtr> &restoredProducts)
{
    for (const ResolvedProductPtr &product : restoredProducts) {
        if (!product->buildData)
            continue;
        for (Artifact * const artifact : filterByType<Artifact>(product->buildData->allNodes())) {
            if (artifact->transformer) {
                artifact->transformer->prepareScriptNeedsChangeTracking = true;
                artifact->transformer->commandsNeedChangeTracking = true;
            }
        }
    }
}

void BuildGraphLoader::checkAllProductsForChanges(
        const std::vector<ResolvedProductPtr> &restoredProducts,
        std::vector<ResolvedProductPtr> &changedProducts)
{
    for (const ResolvedProductPtr &restoredProduct : restoredProducts) {
        const ResolvedProductPtr newlyResolvedProduct
                = m_freshProductsByName.value(restoredProduct->uniqueName());
        if (!newlyResolvedProduct)
            continue;
        if (newlyResolvedProduct->enabled != restoredProduct->enabled) {
            qCDebug(lcBuildGraph) << "Condition of product" << restoredProduct->uniqueName()
                                  << "was changed, must set up build data from scratch";
            if (!contains(changedProducts, restoredProduct))
                changedProducts << restoredProduct;
            continue;
        }

        if (checkProductForChanges(restoredProduct, newlyResolvedProduct)) {
            qCDebug(lcBuildGraph) << "Product" << restoredProduct->uniqueName()
                                  << "was changed, must set up build data from scratch";
            if (!contains(changedProducts, restoredProduct))
                changedProducts << restoredProduct;
            continue;
        }

        if (checkProductForChangesInSourceFiles(restoredProduct, newlyResolvedProduct)) {
            qCDebug(lcBuildGraph) << "File list of product" << restoredProduct->uniqueName()
                                  << "was changed.";
            if (!contains(changedProducts, restoredProduct))
                changedProducts << restoredProduct;
        }
    }
}

bool BuildGraphLoader::checkProductForChangesInSourceFiles(
        const ResolvedProductPtr &restoredProduct, const ResolvedProductPtr &newlyResolvedProduct)
{
    std::vector<SourceArtifactPtr> oldFiles = restoredProduct->allEnabledFiles();
    std::vector<SourceArtifactPtr> newFiles = newlyResolvedProduct->allEnabledFiles();
    // TODO: Also handle added and removed files in a fine-grained manner.
    if (oldFiles.size() != newFiles.size())
        return true;
    static const auto cmp = [](const SourceArtifactConstPtr &a1,
            const SourceArtifactConstPtr &a2) {
        return a1->absoluteFilePath < a2->absoluteFilePath;
    };
    std::sort(oldFiles.begin(), oldFiles.end(), cmp);
    std::sort(newFiles.begin(), newFiles.end(), cmp);
    std::vector<SourceArtifactConstPtr> changedFiles;
    for (int i = 0; i < int(oldFiles.size()); ++i) {
        const SourceArtifactConstPtr &oldFile = oldFiles.at(i);
        const SourceArtifactConstPtr &newFile = newFiles.at(i);
        if (oldFile->absoluteFilePath != newFile->absoluteFilePath)
            return true;
        if (*oldFile != *newFile) {
            qCDebug(lcBuildGraph) << "source artifact" << oldFile->absoluteFilePath << "changed";
            changedFiles.push_back(newFile);
        }
    }
    if (!changedFiles.empty()) {
        m_changedSourcesByProduct.insert(std::make_pair(restoredProduct->uniqueName(),
                                                        changedFiles));
    }
    return false;
}

static bool dependenciesAreEqual(const ResolvedProductConstPtr &p1,
                                 const ResolvedProductConstPtr &p2)
{
    if (p1->dependencies.size() != p2->dependencies.size())
        return false;
    Set<QString> names1;
    Set<QString> names2;
    for (const auto &dep : std::as_const(p1->dependencies))
        names1 << dep->uniqueName();
    for (const auto &dep : std::as_const(p2->dependencies))
        names2 << dep->uniqueName();
    return names1 == names2;
}

bool BuildGraphLoader::checkProductForChanges(const ResolvedProductPtr &restoredProduct,
                                              const ResolvedProductPtr &newlyResolvedProduct)
{
    // This check must come first, as it can prevent build data rescuing as a side effect.
    // TODO: Similar special checks must be done for Environment.getEnv() and File.exists() in
    // commands (or possibly it could be reasonable to just forbid such "dynamic" constructs
    // within commands).
    if (checkForPropertyChanges(restoredProduct, newlyResolvedProduct))
        return true;
    if (!ruleListsAreEqual(restoredProduct->rules, newlyResolvedProduct->rules))
        return true;
    if (!dependenciesAreEqual(restoredProduct, newlyResolvedProduct))
        return true;
    return false;
}

bool BuildGraphLoader::checkProductForInstallInfoChanges(const ResolvedProductPtr &restoredProduct,
        const ResolvedProductPtr &newlyResolvedProduct)
{
    // These are not requested from rules at build time, but we still need to take
    // them into account.
    const QStringList specialProperties = QStringList() << StringConstants::installProperty()
            << StringConstants::installDirProperty() << StringConstants::installPrefixProperty()
            << StringConstants::installRootProperty();
    for (const QString &key : specialProperties) {
        if (!qVariantsEqual(
                restoredProduct->moduleProperties->qbsPropertyValue(key),
                newlyResolvedProduct->moduleProperties->qbsPropertyValue(key))) {
            qCDebug(lcBuildGraph).noquote().nospace()
                << "Product property 'qbs." << key << "' changed.";
            return true;
        }
    }
    return false;
}

bool BuildGraphLoader::checkForPropertyChanges(const ResolvedProductPtr &restoredProduct,
                                               const ResolvedProductPtr &newlyResolvedProduct)
{
    AccumulatingTimer propertyComparisonTimer(m_parameters.logElapsedTime()
                                              ? &m_propertyComparisonEffort: nullptr);
    qCDebug(lcBuildGraph) << "Checking for changes in properties requested in prepare scripts for "
                             "product"  << restoredProduct->uniqueName();
    if (!restoredProduct->buildData)
        return false;

    if (restoredProduct->fileTags != newlyResolvedProduct->fileTags) {
        qCDebug(lcBuildGraph) << "Product type changed from" << restoredProduct->fileTags
                              << "to" << newlyResolvedProduct->fileTags;
        return true;
    }

    if (checkProductForInstallInfoChanges(restoredProduct, newlyResolvedProduct))
        return true;
    if (!artifactPropertyListsAreEqual(restoredProduct->artifactProperties,
                                       newlyResolvedProduct->artifactProperties)) {
        qCDebug(lcBuildGraph) << "a fileTagFilter group changed for product"
                              << restoredProduct->uniqueName();
        m_productsWhoseArtifactsNeedUpdate << restoredProduct->uniqueName();
    }
    if (restoredProduct->moduleProperties != newlyResolvedProduct->moduleProperties) {
        qCDebug(lcBuildGraph) << "module properties changed for product"
                              << restoredProduct->uniqueName();
        m_productsWhoseArtifactsNeedUpdate << restoredProduct->uniqueName();
    }
    return false;
}

void BuildGraphLoader::onProductRemoved(const ResolvedProductPtr &product,
        ProjectBuildData *projectBuildData, bool removeArtifactsFromDisk)
{
    qCDebug(lcBuildGraph) << "product" << product->uniqueName() << "removed.";

    removeOne(product->project->products, product);
    if (product->buildData) {
        for (BuildGraphNode * const node : std::as_const(product->buildData->allNodes())) {
            if (node->type() == BuildGraphNode::ArtifactNodeType) {
                const auto artifact = static_cast<Artifact *>(node);
                projectBuildData->removeArtifact(artifact, m_logger, removeArtifactsFromDisk,
                                                 false);
                if (removeArtifactsFromDisk && artifact->artifactType == Artifact::Generated)
                    m_artifactsRemovedFromDisk << artifact->filePath();
            } else {
                for (BuildGraphNode * const parent : std::as_const(node->parents))
                    parent->children.remove(node);
                node->parents.clear();
                for (BuildGraphNode * const child : std::as_const(node->children))
                    child->parents.remove(node);
                node->children.clear();
            }
        }
    }
}

void BuildGraphLoader::replaceFileDependencyWithArtifact(const ResolvedProductPtr &fileDepProduct,
        FileDependency *filedep, Artifact *artifact)
{
    qCDebug(lcBuildGraph) << "replace file dependency" << filedep->filePath()
                          << "with artifact of type" << toString(artifact->artifactType);
    for (const ResolvedProductPtr &product : fileDepProduct->topLevelProject()->allProducts()) {
        if (!product->buildData)
            continue;
        for (Artifact *artifactInProduct : filterByType<Artifact>(product->buildData->allNodes())) {
            if (artifactInProduct->fileDependencies.remove(filedep))
                connect(artifactInProduct, artifact);
        }
    }
    fileDepProduct->topLevelProject()->buildData->fileDependencies.remove(filedep);
    fileDepProduct->topLevelProject()->buildData->removeFromLookupTable(filedep);
    m_objectsToDelete << filedep;
}

bool BuildGraphLoader::checkConfigCompatibility()
{
    const TopLevelProjectConstPtr restoredProject = m_result.loadedProject;
    if (m_parameters.topLevelProfile().isEmpty()) {
        m_parameters.setTopLevelProfile(restoredProject->profile());
        m_parameters.expandBuildConfiguration();
    }
    if (!m_parameters.overrideBuildGraphData()) {
        if (!m_parameters.overriddenValues().empty()
            && !qVariantMapsEqual(
                m_parameters.overriddenValues(), restoredProject->overriddenValues)) {
            const auto toUserOutput = [](const QVariantMap &propMap) {
                QString o;
                for (auto it = propMap.begin(); it != propMap.end(); ++it) {
                    if (!o.isEmpty())
                        o += QLatin1Char(' ');
                    o.append(it.key()).append(QLatin1Char(':')).append(toJSLiteral(it.value()));
                }
                return o;
            };
            throw ErrorInfo(Tr::tr("Property values set on the command line differ from the "
                                   "ones used for the previous build.\n"
                                   "Old property values: %1\n"
                                   "New property values: %2\n"
                                   "Use the 'resolve' command if "
                                   "you really want to rebuild with the new properties.")
                            .arg(toUserOutput(restoredProject->overriddenValues),
                                     toUserOutput(m_parameters.overriddenValues())));
        }
        m_parameters.setOverriddenValues(restoredProject->overriddenValues);
        if (m_parameters.topLevelProfile() != restoredProject->profile()) {
            throw ErrorInfo(Tr::tr("The current profile is '%1', but profile '%2' was used "
                                   "when last building for configuration '%3'. Use  the "
                                   "'resolve' command if you really want to rebuild with a "
                                   "different profile.")
                            .arg(m_parameters.topLevelProfile(), restoredProject->profile(),
                                 m_parameters.configurationName()));
        }
        m_parameters.setTopLevelProfile(restoredProject->profile());
        m_parameters.expandBuildConfiguration();
    }
    if (!m_parameters.overrideBuildGraphData())
        return true;
    if (!qVariantMapsEqual(
            m_parameters.finalBuildConfigurationTree(), restoredProject->buildConfiguration()))
        return false;
    Settings settings(m_parameters.settingsDirectory());
    const QVariantMap profileConfigsTree = restoredProject->fullProfileConfigsTree();
    for (auto it = profileConfigsTree.begin(); it != profileConfigsTree.end(); ++it) {
        const Profile profile(it.key(), &settings);
        const QVariantMap buildConfig = SetupProjectParameters::expandedBuildConfiguration(
                    profile, m_parameters.configurationName());
        const QVariantMap newConfig = SetupProjectParameters::finalBuildConfigurationTree(
                    buildConfig, m_parameters.overriddenValues());
        if (newConfig != it.value())
            return false;
    }
    return true;
}

void BuildGraphLoader::rescueOldBuildData(const ResolvedProductConstPtr &restoredProduct,
        const ResolvedProductPtr &newlyResolvedProduct, const ChildListHash &childLists,
        const AllRescuableArtifactData &existingRad)
{
    QBS_CHECK(newlyResolvedProduct);
    if (!restoredProduct->buildData)
        return;
    if (!newlyResolvedProduct->buildData)
        newlyResolvedProduct->buildData = std::make_unique<ProductBuildData>();

    qCDebug(lcBuildGraph) << "rescue data of product" << restoredProduct->uniqueName();
    QBS_CHECK(newlyResolvedProduct->buildData);
    QBS_CHECK(newlyResolvedProduct->buildData->rescuableArtifactData().empty());
    newlyResolvedProduct->buildData->setRescuableArtifactData(existingRad);

    // This is needed for artifacts created by rules, which happens later in the executor.
    for (Artifact * const oldArtifact
         : filterByType<Artifact>(restoredProduct->buildData->allNodes())) {
        if (!oldArtifact->transformer)
            continue;
        Artifact * const newArtifact = lookupArtifact(newlyResolvedProduct, oldArtifact, false);
        if (!newArtifact) {
            RescuableArtifactData rad;
            rad.timeStamp = oldArtifact->timestamp();
            rad.knownOutOfDate = oldArtifact->transformer->markedForRerun;
            rad.fileTags = oldArtifact->fileTags();
            rad.properties = oldArtifact->properties;
            rad.commands = oldArtifact->transformer->commands;
            rad.propertiesRequestedInPrepareScript
                    = oldArtifact->transformer->propertiesRequestedInPrepareScript;
            rad.propertiesRequestedInCommands
                    = oldArtifact->transformer->propertiesRequestedInCommands;
            rad.propertiesRequestedFromArtifactInPrepareScript
                    = oldArtifact->transformer->propertiesRequestedFromArtifactInPrepareScript;
            rad.propertiesRequestedFromArtifactInCommands
                    = oldArtifact->transformer->propertiesRequestedFromArtifactInCommands;
            rad.importedFilesUsedInPrepareScript
                    = oldArtifact->transformer->importedFilesUsedInPrepareScript;
            rad.importedFilesUsedInCommands = oldArtifact->transformer->importedFilesUsedInCommands;
            rad.depsRequestedInPrepareScript
                    = oldArtifact->transformer->depsRequestedInPrepareScript;
            rad.depsRequestedInCommands = oldArtifact->transformer->depsRequestedInCommands;
            rad.artifactsMapRequestedInPrepareScript
                    = oldArtifact->transformer->artifactsMapRequestedInPrepareScript;
            rad.artifactsMapRequestedInCommands
                    = oldArtifact->transformer->artifactsMapRequestedInCommands;
            rad.exportedModulesAccessedInPrepareScript
                    = oldArtifact->transformer->exportedModulesAccessedInPrepareScript;
            rad.exportedModulesAccessedInCommands
                    = oldArtifact->transformer->exportedModulesAccessedInCommands;
            rad.lastCommandExecutionTime = oldArtifact->transformer->lastCommandExecutionTime;
            rad.lastPrepareScriptExecutionTime
                    = oldArtifact->transformer->lastPrepareScriptExecutionTime;
            const ChildrenInfo &childrenInfo = childLists.value(oldArtifact);
            for (Artifact * const child : std::as_const(childrenInfo.children)) {
                rad.children.emplace_back(child->product->name,
                        child->product->multiplexConfigurationId, child->filePath(),
                        childrenInfo.childrenAddedByScanner.contains(child));
                transform(oldArtifact->fileDependencies, rad.fileDependencies,
                          std::mem_fn(&FileDependency::filePath));
            }
            newlyResolvedProduct->buildData->addRescuableArtifactData(oldArtifact->filePath(), rad);
        }
    }
}

} // namespace Internal
} // namespace qbs
