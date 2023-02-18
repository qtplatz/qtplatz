// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "plugininstallwizard.h"

#include "coreplugin.h"
#include "icore.h"

#include <extensionsystem/pluginspec.h>

#include <utils/archive.h>
#include <utils/fileutils.h>
#include <utils/hostosinfo.h>
#include <utils/infolabel.h>
#include <utils/pathchooser.h>
#include <utils/qtcassert.h>
#include <utils/qtcprocess.h>
#include <utils/runextensions.h>
#include <utils/temporarydirectory.h>
#include <utils/wizard.h>
#include <utils/wizardpage.h>

#include <app/app_version.h>

#include <QButtonGroup>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QTextEdit>
#include <QVBoxLayout>

#include <memory>

using namespace ExtensionSystem;
using namespace Utils;

struct Data
{
    FilePath sourcePath;
    FilePath extractedPath;
    bool installIntoApplication = false;
};

static QStringList libraryNameFilter()
{
    if (HostOsInfo::isWindowsHost())
        return {"*.dll"};
    if (HostOsInfo::isLinuxHost())
        return {"*.so"};
    return {"*.dylib"};
}

static bool hasLibSuffix(const FilePath &path)
{
    return (HostOsInfo::isWindowsHost() && path.endsWith(".dll"))
           || (HostOsInfo::isLinuxHost() && path.completeSuffix().startsWith(".so"))
           || (HostOsInfo::isMacHost() && path.endsWith(".dylib"));
}

static FilePath pluginInstallPath(bool installIntoApplication)
{
    return FilePath::fromString(installIntoApplication ? Core::ICore::pluginPath()
                                                       : Core::ICore::userPluginPath());
}

namespace Core {
namespace Internal {

class SourcePage : public WizardPage
{
public:
    SourcePage(Data *data, QWidget *parent)
        : WizardPage(parent)
        , m_data(data)
    {
        setTitle(PluginInstallWizard::tr("Source"));
        auto vlayout = new QVBoxLayout;
        setLayout(vlayout);

        auto label = new QLabel(
            "<p>"
            + PluginInstallWizard::tr(
                "Choose source location. This can be a plugin library file or a zip file.")
            + "</p>");
        label->setWordWrap(true);
        vlayout->addWidget(label);

        auto chooser = new PathChooser;
        chooser->setExpectedKind(PathChooser::Any);
        vlayout->addWidget(chooser);
        connect(chooser, &PathChooser::textChanged, this, [this, chooser] {
            m_data->sourcePath = chooser->filePath();
            updateWarnings();
        });

        m_info = new InfoLabel;
        m_info->setType(InfoLabel::Error);
        m_info->setVisible(false);
        vlayout->addWidget(m_info);
    }

    void updateWarnings()
    {
        m_info->setVisible(!isComplete());
        emit completeChanged();
    }

    bool isComplete() const final
    {
        const FilePath path = m_data->sourcePath;
        if (!QFile::exists(path.toString())) {
            m_info->setText(PluginInstallWizard::tr("File does not exist."));
            return false;
        }
        if (hasLibSuffix(path))
            return true;

        QString error;
        if (!Archive::supportsFile(path, &error)) {
            m_info->setText(error);
            return false;
        }
        return true;
    }

    int nextId() const final
    {
        if (hasLibSuffix(m_data->sourcePath))
            return WizardPage::nextId() + 1; // jump over check archive
        return WizardPage::nextId();
    }

    InfoLabel *m_info = nullptr;
    Data *m_data = nullptr;
};

class CheckArchivePage : public WizardPage
{
public:
    struct ArchiveIssue
    {
        QString message;
        InfoLabel::InfoType type;
    };

    CheckArchivePage(Data *data, QWidget *parent)
        : WizardPage(parent)
        , m_data(data)
    {
        setTitle(PluginInstallWizard::tr("Check Archive"));
        auto vlayout = new QVBoxLayout;
        setLayout(vlayout);

        m_label = new InfoLabel;
        m_label->setElideMode(Qt::ElideNone);
        m_label->setWordWrap(true);
        m_cancelButton = new QPushButton(PluginInstallWizard::tr("Cancel"));
        m_output = new QTextEdit;
        m_output->setReadOnly(true);

        auto hlayout = new QHBoxLayout;
        hlayout->addWidget(m_label, 1);
        hlayout->addStretch();
        hlayout->addWidget(m_cancelButton);

        vlayout->addLayout(hlayout);
        vlayout->addWidget(m_output);
    }

    void initializePage() final
    {
        m_isComplete = false;
        emit completeChanged();
        m_canceled = false;

        m_tempDir = std::make_unique<TemporaryDirectory>("plugininstall");
        m_data->extractedPath = m_tempDir->path();
        m_label->setText(PluginInstallWizard::tr("Checking archive..."));
        m_label->setType(InfoLabel::None);

        m_cancelButton->setVisible(true);
        m_output->clear();

        m_archive.reset(new Archive(m_data->sourcePath, m_tempDir->path()));
        if (!m_archive->isValid()) {
            m_label->setType(InfoLabel::Error);
            m_label->setText(PluginInstallWizard::tr("The file is not an archive."));
            return;
        }
        QObject::connect(m_archive.get(), &Archive::outputReceived, this,
                         [this](const QString &output) {
            m_output->append(output);
        });
        QObject::connect(m_archive.get(), &Archive::finished, this, [this](bool success) {
            m_archive.release()->deleteLater();
            handleFinished(success);
        });
        QObject::connect(m_cancelButton, &QPushButton::clicked, this, [this] {
            m_canceled = true;
            m_archive.reset();
            handleFinished(false);
        });
        m_archive->unarchive();
    }

    void handleFinished(bool success)
    {
        m_cancelButton->disconnect();
        if (!success) { // unarchiving failed
            m_cancelButton->setVisible(false);
            if (m_canceled) {
                m_label->setType(InfoLabel::Information);
                m_label->setText(PluginInstallWizard::tr("Canceled."));
            } else {
                m_label->setType(InfoLabel::Error);
                m_label->setText(
                    PluginInstallWizard::tr("There was an error while unarchiving."));
            }
        } else { // unarchiving was successful, run a check
            m_archiveCheck = Utils::runAsync(
                [this](QFutureInterface<ArchiveIssue> &fi) { return checkContents(fi); });
            Utils::onFinished(m_archiveCheck, this, [this](const QFuture<ArchiveIssue> &f) {
                m_cancelButton->setVisible(false);
                m_cancelButton->disconnect();
                const bool ok = f.resultCount() == 0 && !f.isCanceled();
                if (f.isCanceled()) {
                    m_label->setType(InfoLabel::Information);
                    m_label->setText(PluginInstallWizard::tr("Canceled."));
                } else if (ok) {
                    m_label->setType(InfoLabel::Ok);
                    m_label->setText(PluginInstallWizard::tr("Archive is OK."));
                } else {
                    const ArchiveIssue issue = f.result();
                    m_label->setType(issue.type);
                    m_label->setText(issue.message);
                }
                m_isComplete = ok;
                emit completeChanged();
            });
            QObject::connect(m_cancelButton, &QPushButton::clicked, this, [this] {
                m_archiveCheck.cancel();
            });
        }
    }

    // Async. Result is set if any issue was found.
    void checkContents(QFutureInterface<ArchiveIssue> &fi)
    {
        QTC_ASSERT(m_tempDir.get(), return );

        PluginSpec *coreplugin = CorePlugin::instance()->pluginSpec();

        // look for plugin
        QDirIterator it(m_tempDir->path().path(),
                        libraryNameFilter(),
                        QDir::Files | QDir::NoSymLinks,
                        QDirIterator::Subdirectories);
        while (it.hasNext()) {
            if (fi.isCanceled())
                return;
            it.next();
            PluginSpec *spec = PluginSpec::read(it.filePath());
            if (spec) {
                // Is a Qt Creator plugin. Let's see if we find a Core dependency and check the
                // version
                const QVector<PluginDependency> dependencies = spec->dependencies();
                const auto found = std::find_if(dependencies.constBegin(),
                                                dependencies.constEnd(),
                                                [coreplugin](const PluginDependency &d) {
                                                    return d.name == coreplugin->name();
                                                });
                if (found != dependencies.constEnd()) {
                    if (!coreplugin->provides(found->name, found->version)) {
                        fi.reportResult({PluginInstallWizard::tr(
                                             "Plugin requires an incompatible version of %1 (%2).")
                                             .arg(Constants::IDE_DISPLAY_NAME)
                                             .arg(found->version),
                                         InfoLabel::Error});
                        return;
                    }
                }
                return; // successful / no error
            }
        }
        fi.reportResult(
            {PluginInstallWizard::tr("Did not find %1 plugin.").arg(Constants::IDE_DISPLAY_NAME),
             InfoLabel::Error});
    }

    void cleanupPage() final
    {
        // back button pressed
        m_cancelButton->disconnect();
        m_archive.reset();
        if (m_archiveCheck.isRunning()) {
            m_archiveCheck.cancel();
            m_archiveCheck.waitForFinished();
        }
        m_tempDir.reset();
    }

    bool isComplete() const final { return m_isComplete; }

    std::unique_ptr<TemporaryDirectory> m_tempDir;
    std::unique_ptr<Archive> m_archive;
    QFuture<ArchiveIssue> m_archiveCheck;
    InfoLabel *m_label = nullptr;
    QPushButton *m_cancelButton = nullptr;
    QTextEdit *m_output = nullptr;
    Data *m_data = nullptr;
    bool m_isComplete = false;
    bool m_canceled = false;
};

class InstallLocationPage : public WizardPage
{
public:
    InstallLocationPage(Data *data, QWidget *parent)
        : WizardPage(parent)
        , m_data(data)
    {
        setTitle(PluginInstallWizard::tr("Install Location"));
        auto vlayout = new QVBoxLayout;
        setLayout(vlayout);

        auto label = new QLabel("<p>" + PluginInstallWizard::tr("Choose install location.")
                                + "</p>");
        label->setWordWrap(true);
        vlayout->addWidget(label);
        vlayout->addSpacing(10);

        auto localInstall = new QRadioButton(PluginInstallWizard::tr("User plugins"));
        localInstall->setChecked(!m_data->installIntoApplication);
        auto localLabel = new QLabel(
            PluginInstallWizard::tr("The plugin will be available to all compatible %1 "
                                    "installations, but only for the current user.")
                .arg(Constants::IDE_DISPLAY_NAME));
        localLabel->setWordWrap(true);
        localLabel->setAttribute(Qt::WA_MacSmallSize, true);

        vlayout->addWidget(localInstall);
        vlayout->addWidget(localLabel);
        vlayout->addSpacing(10);

        auto appInstall = new QRadioButton(
            PluginInstallWizard::tr("%1 installation").arg(Constants::IDE_DISPLAY_NAME));
        appInstall->setChecked(m_data->installIntoApplication);
        auto appLabel = new QLabel(
            PluginInstallWizard::tr("The plugin will be available only to this %1 "
                                    "installation, but for all users that can access it.")
                .arg(Constants::IDE_DISPLAY_NAME));
        appLabel->setWordWrap(true);
        appLabel->setAttribute(Qt::WA_MacSmallSize, true);
        vlayout->addWidget(appInstall);
        vlayout->addWidget(appLabel);

        auto group = new QButtonGroup(this);
        group->addButton(localInstall);
        group->addButton(appInstall);

        connect(appInstall, &QRadioButton::toggled, this, [this](bool toggled) {
            m_data->installIntoApplication = toggled;
        });
    }

    Data *m_data = nullptr;
};

class SummaryPage : public WizardPage
{
public:
    SummaryPage(Data *data, QWidget *parent)
        : WizardPage(parent)
        , m_data(data)
    {
        setTitle(PluginInstallWizard::tr("Summary"));

        auto vlayout = new QVBoxLayout;
        setLayout(vlayout);

        m_summaryLabel = new QLabel(this);
        m_summaryLabel->setWordWrap(true);
        vlayout->addWidget(m_summaryLabel);
    }

    void initializePage() final
    {
        m_summaryLabel->setText(
            PluginInstallWizard::tr("\"%1\" will be installed into \"%2\".")
                .arg(m_data->sourcePath.toUserOutput(),
                     pluginInstallPath(m_data->installIntoApplication).toUserOutput()));
    }

private:
    QLabel *m_summaryLabel;
    Data *m_data = nullptr;
};

static std::function<void(FilePath)> postCopyOperation()
{
    return [](const FilePath &filePath) {
        if (!HostOsInfo::isMacHost())
            return;
        // On macOS, downloaded files get a quarantine flag, remove it, otherwise it is a hassle
        // to get it loaded as a plugin in Qt Creator.
        QtcProcess xattr;
        xattr.setTimeoutS(1);
        xattr.setCommand({"/usr/bin/xattr", {"-d", "com.apple.quarantine", filePath.absoluteFilePath().toString()}});
        xattr.runBlocking();
    };
}

static bool copyPluginFile(const FilePath &src, const FilePath &dest)
{
    const FilePath destFile = dest.pathAppended(src.fileName());
    if (destFile.exists()) {
        QMessageBox box(QMessageBox::Question,
                        PluginInstallWizard::tr("Overwrite File"),
                        PluginInstallWizard::tr("The file \"%1\" exists. Overwrite?")
                            .arg(destFile.toUserOutput()),
                        QMessageBox::Cancel,
                        ICore::dialogParent());
        QPushButton *acceptButton = box.addButton(PluginInstallWizard::tr("Overwrite"),
                                                  QMessageBox::AcceptRole);
        box.setDefaultButton(acceptButton);
        box.exec();
        if (box.clickedButton() != acceptButton)
            return false;
        destFile.removeFile();
    }
    dest.parentDir().ensureWritableDir();
    if (!src.copyFile(destFile)) {
        QMessageBox::warning(ICore::dialogParent(),
                             PluginInstallWizard::tr("Failed to Write File"),
                             PluginInstallWizard::tr("Failed to write file \"%1\".")
                                 .arg(destFile.toUserOutput()));
        return false;
    }
    postCopyOperation()(destFile);
    return true;
}

bool PluginInstallWizard::exec()
{
    Wizard wizard(ICore::dialogParent());
    wizard.setWindowTitle(tr("Install Plugin"));

    Data data;

    auto filePage = new SourcePage(&data, &wizard);
    wizard.addPage(filePage);

    auto checkArchivePage = new CheckArchivePage(&data, &wizard);
    wizard.addPage(checkArchivePage);

    auto installLocationPage = new InstallLocationPage(&data, &wizard);
    wizard.addPage(installLocationPage);

    auto summaryPage = new SummaryPage(&data, &wizard);
    wizard.addPage(summaryPage);

    if (wizard.exec()) {
        const FilePath installPath = pluginInstallPath(data.installIntoApplication);
        if (hasLibSuffix(data.sourcePath)) {
            return copyPluginFile(data.sourcePath, installPath);
        } else {
            QString error;
            if (!FileUtils::copyRecursively(data.extractedPath,
                                            installPath,
                                            &error,
                                            FileUtils::CopyAskingForOverwrite(ICore::dialogParent(),
                                                                              postCopyOperation()))) {
                QMessageBox::warning(ICore::dialogParent(),
                                     PluginInstallWizard::tr("Failed to Copy Plugin Files"),
                                     error);
                return false;
            }
            return true;
        }
    }
    return false;
}

} // namespace Internal
} // namespace Core
