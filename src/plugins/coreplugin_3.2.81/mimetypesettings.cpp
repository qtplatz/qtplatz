/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#include "coreconstants.h"
#include "icore.h"
#include "mimedatabase.h"
#include "mimetypemagicdialog.h"
#include "mimetypesettings.h"
#include "ui_mimetypesettingspage.h"
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/editormanager/ieditorfactory.h>
#include <coreplugin/editormanager/iexternaleditor.h>
#include <utils/algorithm.h>
#include <utils/headerviewstretcher.h>

#include <QAbstractTableModel>
#include <QCoreApplication>
#include <QHash>
#include <QMessageBox>
#include <QPointer>
#include <QScopedPointer>
#include <QSet>
#include <QStringList>
#include <QSortFilterProxyModel>

namespace Core {
namespace Internal {

// MimeTypeSettingsModel
class MimeTypeSettingsModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    MimeTypeSettingsModel(QObject *parent = 0)
        : QAbstractTableModel(parent) {}
    virtual ~MimeTypeSettingsModel() {}

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const;
    virtual QVariant data(const QModelIndex &modelIndex, int role = Qt::DisplayRole) const;

    void load();
    void validatePatterns(QStringList *candidates, const MimeType &mimeType) const;
    void updateKnownPatterns(const QStringList &oldPatterns, const QStringList &newPatterns);

    QList<MimeType> m_mimeTypes;
    QSet<QString> m_knownPatterns;
    QHash<QString, QString> m_handlersByMimeType;
};

int MimeTypeSettingsModel::rowCount(const QModelIndex &) const
{
    return m_mimeTypes.size();
}

int MimeTypeSettingsModel::columnCount(const QModelIndex &) const
{
    return 2;
}

QVariant MimeTypeSettingsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
        return QVariant();

    if (section == 0)
        return tr("MIME Type");
    else
        return tr("Handler");
}

QVariant MimeTypeSettingsModel::data(const QModelIndex &modelIndex, int role) const
{
    if (!modelIndex.isValid())
        return QVariant();

    const int column = modelIndex.column();
    if (role == Qt::DisplayRole) {
        const QString &type = m_mimeTypes.at(modelIndex.row()).type();
        if (column == 0)
            return type;
        else
            return m_handlersByMimeType.value(type);
    }
    return QVariant();
}

void MimeTypeSettingsModel::load()
{
    m_mimeTypes = MimeDatabase::mimeTypes();
    Utils::sort(m_mimeTypes, [](const MimeType &a, const MimeType &b) {
        return a.type().compare(b.type(), Qt::CaseInsensitive) < 0;
    });
    m_knownPatterns = QSet<QString>::fromList(
        MimeDatabase::fromGlobPatterns(MimeDatabase::globPatterns()));

    foreach (const MimeType &mimeType, m_mimeTypes) {
        QString value;
        const QList<IEditorFactory *> factories =
            EditorManager::editorFactories(mimeType);
        if (!factories.isEmpty()) {
            value = factories.front()->displayName();
        } else {
            const QList<IExternalEditor *> externalEditors =
                EditorManager::externalEditors(mimeType);
            if (!externalEditors.isEmpty())
                value = externalEditors.front()->displayName();
            else
                value = tr("Undefined");
        }
        m_handlersByMimeType.insert(mimeType.type(), value);
    }
}

void MimeTypeSettingsModel::validatePatterns(QStringList *candidates,
                                             const MimeType &mimeType) const
{
    QSet<QString> oldPatterns =
        QSet<QString>::fromList(MimeDatabase::fromGlobPatterns(mimeType.globPatterns()));

    QStringList duplicates;
    QStringList::iterator it = candidates->begin();
    while (it != candidates->end()) {
        const QString &current = *it;
        if (!oldPatterns.contains(current) && m_knownPatterns.contains(current)) {
            duplicates.append(current);
            it = candidates->erase(it);
        } else {
            ++it;
        }
    }

    if (!duplicates.isEmpty()) {
        QMessageBox msgBox(QMessageBox::NoIcon, tr("Invalid MIME Type"),
                           tr("Conflicting pattern(s) will be discarded."),
                           QMessageBox::Ok, ICore::dialogParent());
        msgBox.setInformativeText(tr("%n pattern(s) already in use.", 0, duplicates.size()));
        msgBox.setDetailedText(duplicates.join(QLatin1String("\n")));
        msgBox.exec();
    }
}

void MimeTypeSettingsModel::updateKnownPatterns(const QStringList &oldPatterns,
                                                const QStringList &newPatterns)
{
    QStringList all = oldPatterns;
    all.append(newPatterns);
    all.removeDuplicates();
    foreach (const QString &pattern, all) {
        QSet<QString>::iterator it = m_knownPatterns.find(pattern);
        if (it == m_knownPatterns.end()) {
            // A pattern was added.
            m_knownPatterns.insert(pattern);
        } else {
            // A pattern was removed.
            m_knownPatterns.erase(it);
        }
    }
}

// MimeTypeSettingsPrivate
class MimeTypeSettingsPrivate : public QObject
{
    Q_OBJECT

public:
    MimeTypeSettingsPrivate();
    virtual ~MimeTypeSettingsPrivate();

    void configureUi(QWidget *w);

    bool checkSelectedMimeType() const;
    bool checkSelectedMagicHeader() const;

    void markMimeForPatternSync(int index);
    void markMimeForMagicSync(int index);
    void syncMimePattern();
    void syncMimeMagic();
    void clearSyncData();
    void markAsModified(int index);

    void addMagicHeaderRow(const MagicData &data);
    MagicData getMagicHeaderRowData(const int row) const;
    void editMagicHeaderRowData(const int row, const MagicData &data);

    void updateMimeDatabase();
    void resetState();

public slots:
    void syncData(const QModelIndex &current, const QModelIndex &previous);
    void handlePatternEdited();
    void addMagicHeader();
    void removeMagicHeader();
    void editMagicHeader();
    void resetMimeTypes();
    void updateMagicHeaderButtons();

private slots:
    void setFilterPattern(const QString &pattern);

public:
    static const QChar kSemiColon;

    MimeTypeSettingsModel *m_model;
    QSortFilterProxyModel *m_filterModel;
    int m_mimeForPatternSync;
    int m_mimeForMagicSync;
    bool m_reset;
    bool m_persist;
    QList<int> m_modifiedMimeTypes;
    QString m_filterPattern;
    Ui::MimeTypeSettingsPage m_ui;
    QPointer<QWidget> m_widget;
};

const QChar MimeTypeSettingsPrivate::kSemiColon(QLatin1Char(';'));

MimeTypeSettingsPrivate::MimeTypeSettingsPrivate()
    : m_model(new MimeTypeSettingsModel(this))
    , m_filterModel(new QSortFilterProxyModel(this))
    , m_mimeForPatternSync(-1)
    , m_mimeForMagicSync(-1)
    , m_reset(false)
    , m_persist(false)
{
    m_filterModel->setSourceModel(m_model);
    m_filterModel->setFilterKeyColumn(-1);
}

MimeTypeSettingsPrivate::~MimeTypeSettingsPrivate()
{}

void MimeTypeSettingsPrivate::configureUi(QWidget *w)
{
    m_ui.setupUi(w);
    m_ui.filterLineEdit->setText(m_filterPattern);

    m_model->load();
    connect(m_ui.filterLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(setFilterPattern(QString)));
    m_ui.mimeTypesTreeView->setModel(m_filterModel);

    new Utils::HeaderViewStretcher(m_ui.mimeTypesTreeView->header(), 1);

    connect(m_ui.mimeTypesTreeView->selectionModel(),
            SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this,
            SLOT(syncData(QModelIndex,QModelIndex)));
    connect(m_ui.patternsLineEdit, SIGNAL(textEdited(QString)),
            this, SLOT(handlePatternEdited()));
    connect(m_ui.addMagicButton, SIGNAL(clicked()), this, SLOT(addMagicHeader()));
    connect(m_ui.removeMagicButton, SIGNAL(clicked()), this, SLOT(removeMagicHeader()));
    connect(m_ui.editMagicButton, SIGNAL(clicked()), this, SLOT(editMagicHeader()));
    connect(m_ui.resetButton, SIGNAL(clicked()), this, SLOT(resetMimeTypes()));
    connect(m_ui.magicHeadersTreeWidget,
            SIGNAL(itemSelectionChanged()),
            SLOT(updateMagicHeaderButtons()));
    updateMagicHeaderButtons();
}

bool MimeTypeSettingsPrivate::checkSelectedMimeType() const
{
    const QModelIndex &modelIndex = m_ui.mimeTypesTreeView->currentIndex();
    if (!modelIndex.isValid()) {
        QMessageBox::critical(0, tr("Error"), tr("No MIME type selected."));
        return false;
    }
    return true;
}

bool MimeTypeSettingsPrivate::checkSelectedMagicHeader() const
{
    const QModelIndex &modelIndex = m_ui.magicHeadersTreeWidget->selectionModel()->currentIndex();
    if (!modelIndex.isValid()) {
        QMessageBox::critical(0, tr("Error"), tr("No magic header selected."));
        return false;
    }
    return true;
}

void MimeTypeSettingsPrivate::markMimeForPatternSync(int index)
{
    if (m_mimeForPatternSync != index) {
        m_mimeForPatternSync = index;
        markAsModified(index);
    }
}

void MimeTypeSettingsPrivate::markMimeForMagicSync(int index)
{
    if (m_mimeForMagicSync != index) {
        m_mimeForMagicSync = index;
        markAsModified(index);
    }
}

void MimeTypeSettingsPrivate::markAsModified(int index)
{
    // Duplicates are handled later.
    m_modifiedMimeTypes.append(index);
}

void MimeTypeSettingsPrivate::syncMimePattern()
{
    MimeType &mimeType = m_model->m_mimeTypes[m_mimeForPatternSync];
    QStringList patterns = m_ui.patternsLineEdit->text().split(kSemiColon);
    patterns.removeDuplicates();
    m_model->validatePatterns(&patterns, mimeType);
    m_model->updateKnownPatterns(MimeDatabase::fromGlobPatterns(mimeType.globPatterns()), patterns);
    mimeType.setGlobPatterns(MimeDatabase::toGlobPatterns(patterns));
}

void MimeTypeSettingsPrivate::syncMimeMagic()
{
    typedef MagicRuleMatcher::MagicRuleList MagicRuleList;
    typedef MagicRuleMatcher::MagicRuleSharedPointer MagicRuleSharedPointer;

    // Gather the magic rules.
    QHash<int, MagicRuleList> rulesByPriority;
    for (int row = 0; row < m_ui.magicHeadersTreeWidget->topLevelItemCount(); ++row) {
        const MagicData &data = getMagicHeaderRowData(row);
        // @TODO: Validate magic rule?
        MagicRule *magicRule;
        if (data.m_type == MagicStringRule::kMatchType)
            magicRule = new MagicStringRule(data.m_value, data.m_start, data.m_end);
        else
            magicRule = new MagicByteRule(data.m_value, data.m_start, data.m_end);
        rulesByPriority[data.m_priority].append(MagicRuleSharedPointer(magicRule));
    }

    const QList<QSharedPointer<IMagicMatcher> > &matchers =
        MagicRuleMatcher::createMatchers(rulesByPriority);
    m_model->m_mimeTypes[m_mimeForMagicSync].setMagicRuleMatchers(matchers);
}

void MimeTypeSettingsPrivate::clearSyncData()
{
    m_mimeForPatternSync = -1;
    m_mimeForMagicSync = -1;
}

void MimeTypeSettingsPrivate::syncData(const QModelIndex &current,
                                       const QModelIndex &previous)
{
    if (previous.isValid()) {
        if (m_mimeForPatternSync == m_filterModel->mapToSource(previous).row())
            syncMimePattern();
        if (m_mimeForMagicSync == m_filterModel->mapToSource(previous).row())
            syncMimeMagic();
        clearSyncData();

        m_ui.patternsLineEdit->clear();
        m_ui.magicHeadersTreeWidget->clear();
    }

    if (current.isValid()) {
        const MimeType &currentMimeType =
                m_model->m_mimeTypes.at(m_filterModel->mapToSource(current).row());

        QStringList formatedPatterns;
        foreach (const MimeGlobPattern &pattern, currentMimeType.globPatterns())
            formatedPatterns.append(pattern.pattern());
        m_ui.patternsLineEdit->setText(formatedPatterns.join(kSemiColon));

        // Consider only rule-based matchers.
        const QList<QSharedPointer<IMagicMatcher> > &matchers = currentMimeType.magicRuleMatchers();
        foreach (const QSharedPointer<IMagicMatcher> &matcher, matchers) {
            MagicRuleMatcher *ruleMatcher = static_cast<MagicRuleMatcher *>(matcher.data());
            const int priority = ruleMatcher->priority();
            const MagicRuleMatcher::MagicRuleList &rules = ruleMatcher->magicRules();
            foreach (const MagicRuleMatcher::MagicRuleSharedPointer &rule, rules)
                addMagicHeaderRow(MagicData(rule->matchValue(),
                                            rule->matchType(),
                                            rule->startPos(),
                                            rule->endPos(),
                                            priority));
        }
    }
}

void MimeTypeSettingsPrivate::handlePatternEdited()
{
    if (m_mimeForPatternSync == -1) {
        const QModelIndex &modelIndex = m_ui.mimeTypesTreeView->currentIndex();
        if (modelIndex.isValid())
            markMimeForPatternSync(m_filterModel->mapToSource(modelIndex).row());
    }
}

void MimeTypeSettingsPrivate::addMagicHeaderRow(const MagicData &data)
{
    const int row = m_ui.magicHeadersTreeWidget->topLevelItemCount();
    editMagicHeaderRowData(row, data);
}

MagicData MimeTypeSettingsPrivate::getMagicHeaderRowData(const int row) const
{
    MagicData data;
    data.m_value = m_ui.magicHeadersTreeWidget->topLevelItem(row)->text(0);
    data.m_type = m_ui.magicHeadersTreeWidget->topLevelItem(row)->text(1);
    QPair<int, int> startEnd =
        MagicRule::fromOffset(m_ui.magicHeadersTreeWidget->topLevelItem(row)->text(2));
    data.m_start = startEnd.first;
    data.m_end = startEnd.second;
    data.m_priority = m_ui.magicHeadersTreeWidget->topLevelItem(row)->text(3).toInt();

    return data;
}

void MimeTypeSettingsPrivate::editMagicHeaderRowData(const int row, const MagicData &data)
{
    QTreeWidgetItem *item = new QTreeWidgetItem;
    item->setText(0, data.m_value);
    item->setText(1, data.m_type);
    item->setText(2, MagicRule::toOffset(qMakePair(data.m_start, data.m_end)));
    item->setText(3, QString::number(data.m_priority));
    m_ui.magicHeadersTreeWidget->takeTopLevelItem(row);
    m_ui.magicHeadersTreeWidget->insertTopLevelItem(row, item);
    m_ui.magicHeadersTreeWidget->setCurrentItem(item);
}

void MimeTypeSettingsPrivate::addMagicHeader()
{
    if (!checkSelectedMimeType())
        return;

    MimeTypeMagicDialog dlg;
    if (dlg.exec()) {
        addMagicHeaderRow(dlg.magicData());
        markMimeForMagicSync(m_filterModel->mapToSource(
            m_ui.mimeTypesTreeView->currentIndex()).row());
    }
}

void MimeTypeSettingsPrivate::removeMagicHeader()
{
    if (!checkSelectedMagicHeader())
        return;

    m_ui.magicHeadersTreeWidget->takeTopLevelItem(m_ui.magicHeadersTreeWidget->indexOfTopLevelItem(m_ui.magicHeadersTreeWidget->currentItem()));
    markMimeForMagicSync(m_filterModel->mapToSource(
        m_ui.mimeTypesTreeView->currentIndex()).row());
}

void MimeTypeSettingsPrivate::editMagicHeader()
{
    if (!checkSelectedMagicHeader())
        return;

    MimeTypeMagicDialog dlg;
    dlg.setMagicData(getMagicHeaderRowData(m_ui.magicHeadersTreeWidget->indexOfTopLevelItem(m_ui.magicHeadersTreeWidget->currentItem())));
    if (dlg.exec()) {
        editMagicHeaderRowData(m_ui.magicHeadersTreeWidget->indexOfTopLevelItem(m_ui.magicHeadersTreeWidget->currentItem()), dlg.magicData());
        markMimeForMagicSync(m_filterModel->mapToSource(
            m_ui.mimeTypesTreeView->currentIndex()).row());
    }
}

void MimeTypeSettingsPrivate::updateMimeDatabase()
{
    if (m_modifiedMimeTypes.isEmpty())
        return;

    // For this case it is a better approach to simply use a list and to remove duplicates
    // afterwards than to keep a more complex data structure like a hash table.
    Utils::sort(m_modifiedMimeTypes);
    m_modifiedMimeTypes.erase(std::unique(m_modifiedMimeTypes.begin(), m_modifiedMimeTypes.end()),
                              m_modifiedMimeTypes.end());

    QList<MimeType> allModified;
    foreach (int index, m_modifiedMimeTypes) {
        const MimeType &mimeType = m_model->m_mimeTypes.at(index);
        MimeDatabase::setGlobPatterns(mimeType.type(), mimeType.globPatterns());
        MimeDatabase::setMagicMatchers(mimeType.type(), mimeType.magicMatchers());
        allModified.append(mimeType);
    }
    MimeDatabase::writeUserModifiedMimeTypes(allModified);
}

void MimeTypeSettingsPrivate::resetState()
{
    clearSyncData();
    m_modifiedMimeTypes.clear();
    m_reset = false;
    m_persist = false;
}

void MimeTypeSettingsPrivate::resetMimeTypes()
{
    QMessageBox::information(0,
                             tr("MIME Types"),
                             tr("Changes will take effect in the next time you start Qt Creator."));
    m_reset = true;
}

void MimeTypeSettingsPrivate::updateMagicHeaderButtons()
{
    const QModelIndex &modelIndex = m_ui.magicHeadersTreeWidget->currentIndex();
    const bool enabled = modelIndex.isValid();

    m_ui.removeMagicButton->setEnabled(enabled);
    m_ui.editMagicButton->setEnabled(enabled);
}

void MimeTypeSettingsPrivate::setFilterPattern(const QString &pattern)
{
    m_filterPattern = pattern;
    m_filterModel->setFilterWildcard(pattern);
}

// MimeTypeSettingsPage
MimeTypeSettings::MimeTypeSettings(QObject *parent)
    : IOptionsPage(parent)
    , d(new MimeTypeSettingsPrivate)
{
    setId(Core::Constants::SETTINGS_ID_MIMETYPES);
    setDisplayName(tr("MIME Types"));
    setCategory(Core::Constants::SETTINGS_CATEGORY_CORE);
    setDisplayCategory(QCoreApplication::translate("Core",
        Core::Constants::SETTINGS_TR_CATEGORY_CORE));
    setCategoryIcon(QLatin1String(Core::Constants::SETTINGS_CATEGORY_CORE_ICON));
}

MimeTypeSettings::~MimeTypeSettings()
{
    delete d;
}

QWidget *MimeTypeSettings::widget()
{
    if (!d->m_widget) {
        d->m_widget = new QWidget;
        d->configureUi(d->m_widget);
    }
    return d->m_widget;
}

void MimeTypeSettings::apply()
{
    if (!d->m_modifiedMimeTypes.isEmpty()) {
        const QModelIndex &modelIndex =
            d->m_ui.mimeTypesTreeView->currentIndex();
        if (modelIndex.isValid()) {
            if (d->m_mimeForPatternSync == d->m_filterModel->mapToSource(modelIndex).row())
                d->syncMimePattern();
            if (d->m_mimeForMagicSync == d->m_filterModel->mapToSource(modelIndex).row())
                d->syncMimeMagic();
        }
        d->clearSyncData();
    }

    if (!d->m_persist)
        d->m_persist = true;
}

void MimeTypeSettings::finish()
{
    if (d->m_persist) {
        if (d->m_reset)
            MimeDatabase::clearUserModifiedMimeTypes();
        else
            d->updateMimeDatabase();
    }
    d->resetState();
    delete d->m_widget;
}

} // Internal
} // Core

#include "mimetypesettings.moc"
