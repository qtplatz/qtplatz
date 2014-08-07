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

#include "basefilewizardfactory.h"

#include "basefilewizard.h"
#include "icore.h"
#include "ifilewizardextension.h"
#include "mimedatabase.h"
#include "editormanager/editormanager.h"
#include "dialogs/promptoverwritedialog.h"
#include <extensionsystem/pluginmanager.h>
#include <utils/filewizardpage.h>
#include <utils/wizard.h>
#include <utils/qtcassert.h>
#include <utils/stringutils.h>

#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QSharedData>
#include <QEventLoop>
#include <QScopedPointer>

#include <QMessageBox>
#include <QWizard>
#include <QIcon>

enum { debugWizard = 0 };

namespace Core {

static int indexOfFile(const GeneratedFiles &f, const QString &path)
{
    const int size = f.size();
    for (int i = 0; i < size; ++i)
        if (f.at(i).path() == path)
            return i;
    return -1;
}

/*!
    \class Core::Internal::WizardEventLoop
    \brief The WizardEventLoop class implements a special event
    loop that runs a QWizard and terminates if the page changes.

    Used by Core::BaseFileWizard to intercept the change from the standard wizard pages
    to the extension pages (as the latter require the list of Core::GeneratedFile generated).

    Synopsis:
    \code
    Wizard wizard(parent);
    WizardEventLoop::WizardResult wr;
    do {
        wr = WizardEventLoop::execWizardPage(wizard);
    } while (wr == WizardEventLoop::PageChanged);
    \endcode

    \sa Core::GeneratedFile, Core::BaseFileWizardParameters, Core::BaseFileWizard, Core::StandardFileWizard
*/

class WizardEventLoop : public QEventLoop
{
    Q_OBJECT
    WizardEventLoop(QObject *parent);

public:
    enum WizardResult { Accepted, Rejected , PageChanged };

    static WizardResult execWizardPage(QWizard &w);

private slots:
    void pageChanged(int);
    void accepted();
    void rejected();

private:
    WizardResult execWizardPageI();

    WizardResult m_result;
};

WizardEventLoop::WizardEventLoop(QObject *parent) :
    QEventLoop(parent),
    m_result(Rejected)
{
}

WizardEventLoop::WizardResult WizardEventLoop::execWizardPage(QWizard &wizard)
{
    /* Install ourselves on the wizard. Main trick is here to connect
     * to the page changed signal and quit() on it. */
    WizardEventLoop *eventLoop = wizard.findChild<WizardEventLoop *>();
    if (!eventLoop) {
        eventLoop = new WizardEventLoop(&wizard);
        connect(&wizard, SIGNAL(currentIdChanged(int)), eventLoop, SLOT(pageChanged(int)));
        connect(&wizard, SIGNAL(accepted()), eventLoop, SLOT(accepted()));
        connect(&wizard, SIGNAL(rejected()), eventLoop, SLOT(rejected()));
        wizard.setWindowFlags(wizard.windowFlags());
        wizard.show();
    }
    const WizardResult result = eventLoop->execWizardPageI();
    // Quitting?
    if (result != PageChanged)
        delete eventLoop;
    if (debugWizard)
        qDebug() << "WizardEventLoop::runWizard" << wizard.pageIds() << " returns " << result;

    return result;
}

WizardEventLoop::WizardResult WizardEventLoop::execWizardPageI()
{
    m_result = Rejected;
    exec();
    return m_result;
}

void WizardEventLoop::pageChanged(int /*page*/)
{
    m_result = PageChanged;
    quit(); // !
}

void WizardEventLoop::accepted()
{
    m_result = Accepted;
    quit();
}

void WizardEventLoop::rejected()
{
    m_result = Rejected;
    quit();
}

/*!
    \class Core::BaseFileWizard
    \brief The BaseFileWizard class implements a generic wizard for
    creating files.

    The following abstract functions must be implemented:
    \list
    \li create(): Called to create the QWizard dialog to be shown.
    \li generateFiles(): Generates file content.
    \endlist

    The behaviour can be further customized by overwriting the virtual function \c postGenerateFiles(),
    which is called after generating the files.

    \sa Core::GeneratedFile, Core::BaseFileWizardParameters, Core::StandardFileWizard
    \sa Core::Internal::WizardEventLoop
*/

void BaseFileWizardFactory::runWizard(const QString &path, QWidget *parent, const QString &platform, const QVariantMap &extraValues)
{
    QTC_ASSERT(!path.isEmpty(), return);

    QString errorMessage;
    // Compile extension pages, purge out unused ones
    QList<IFileWizardExtension *> extensionList = ExtensionSystem::PluginManager::getObjects<IFileWizardExtension>();
    WizardPageList  allExtensionPages;
    for (auto it = extensionList.begin(); it !=  extensionList.end(); ) {
        const WizardPageList extensionPages = (*it)->extensionPages(this);
        if (extensionPages.empty()) {
            it = extensionList.erase(it);
        } else {
            allExtensionPages += extensionPages;
            ++it;
        }
    }

    if (debugWizard)
        qDebug() << Q_FUNC_INFO <<  path << parent << "exs" <<  extensionList.size() << allExtensionPages.size();

    QWizardPage *firstExtensionPage = 0;
    if (!allExtensionPages.empty())
        firstExtensionPage = allExtensionPages.front();

    // Create dialog and run it. Ensure that the dialog is deleted when
    // leaving the func, but not before the IFileWizardExtension::process
    // has been called

    WizardDialogParameters::DialogParameterFlags dialogParameterFlags;

    if (flags().testFlag(ForceCapitalLetterForFileName))
        dialogParameterFlags |= WizardDialogParameters::ForceCapitalLetterForFileName;

    const QScopedPointer<QWizard> wizard(create(parent, WizardDialogParameters(path,
                                                                               allExtensionPages,
                                                                               platform,
                                                                               requiredFeatures(),
                                                                               dialogParameterFlags,
                                                                               extraValues)));
    QTC_ASSERT(!wizard.isNull(), return);

    GeneratedFiles files;
    // Run the wizard: Call generate files on switching to the first extension
    // page is OR after 'Accepted' if there are no extension pages
    while (true) {
        const WizardEventLoop::WizardResult wr = WizardEventLoop::execWizardPage(*wizard);
        if (wr == WizardEventLoop::Rejected) {
            files.clear();
            break;
        }
        const bool accepted = wr == WizardEventLoop::Accepted;
        const bool firstExtensionPageHit = wr == WizardEventLoop::PageChanged
                                           && wizard->page(wizard->currentId()) == firstExtensionPage;
        const bool needGenerateFiles = firstExtensionPageHit || (accepted && allExtensionPages.empty());
        if (needGenerateFiles) {
            QString errorMessage;
            files = generateFiles(wizard.data(), &errorMessage);
            if (files.empty()) {
                QMessageBox::critical(0, tr("File Generation Failure"), errorMessage);
                break;
            }
        }
        if (firstExtensionPageHit)
            foreach (IFileWizardExtension *ex, extensionList)
                ex->firstExtensionPageShown(files, extraValues);
        if (accepted)
            break;
    }
    if (files.empty())
        return;
    // Compile result list and prompt for overwrite
    switch (promptOverwrite(&files, &errorMessage)) {
    case OverwriteCanceled:
        return;
    case OverwriteError:
        QMessageBox::critical(0, tr("Existing files"), errorMessage);
        return;
    case OverwriteOk:
        break;
    }

    foreach (IFileWizardExtension *ex, extensionList) {
        for (int i = 0; i < files.count(); i++) {
            ex->applyCodeStyle(&files[i]);
        }
    }

    // Write
    if (!writeFiles(files, &errorMessage)) {
        QMessageBox::critical(parent, tr("File Generation Failure"), errorMessage);
        return;
    }

    bool removeOpenProjectAttribute = false;
    // Run the extensions
    foreach (IFileWizardExtension *ex, extensionList) {
        bool remove;
        if (!ex->processFiles(files, &remove, &errorMessage)) {
            if (!errorMessage.isEmpty())
                QMessageBox::critical(parent, tr("File Generation Failure"), errorMessage);
            return;
        }
        removeOpenProjectAttribute |= remove;
    }

    if (removeOpenProjectAttribute) {
        for (int i = 0; i < files.count(); i++) {
            if (files[i].attributes() & GeneratedFile::OpenProjectAttribute)
                files[i].setAttributes(GeneratedFile::OpenEditorAttribute);
        }
    }

    // Post generation handler
    if (!postGenerateFiles(wizard.data(), files, &errorMessage))
        if (!errorMessage.isEmpty())
            QMessageBox::critical(0, tr("File Generation Failure"), errorMessage);
}

/*!
    \fn virtual QWizard *Core::BaseFileWizard::create(QWidget *parent,
                                                      const WizardDialogParameters &parameters) const

    Creates the wizard on the \a parent with the \a parameters.
*/

/*!
    \fn virtual Core::GeneratedFiles Core::BaseFileWizard::generateFiles(const QWizard *w,
                                                                         QString *errorMessage) const = 0
    Overwrite to query the parameters from the dialog and generate the files.

    \note This does not generate physical files, but merely the list of
    Core::GeneratedFile.
*/

/*!
    Physically writes files.

    Re-implement (calling the base implementation) to create files with CustomGeneratorAttribute set.
*/

bool BaseFileWizardFactory::writeFiles(const GeneratedFiles &files, QString *errorMessage)
{
    const GeneratedFile::Attributes noWriteAttributes
        = GeneratedFile::CustomGeneratorAttribute|GeneratedFile::KeepExistingFileAttribute;
    foreach (const GeneratedFile &generatedFile, files)
        if (!(generatedFile.attributes() & noWriteAttributes ))
            if (!generatedFile.write(errorMessage))
                return false;
    return true;
}

/*!
    Overwrite to perform steps to be done after files are actually created.

    The default implementation opens editors with the newly generated files.
*/

bool BaseFileWizardFactory::postGenerateFiles(const QWizard *, const GeneratedFiles &l, QString *errorMessage)
{
    return BaseFileWizardFactory::postGenerateOpenEditors(l, errorMessage);
}

/*!
    Opens the editors for the files whose attribute is set accordingly.
*/

bool BaseFileWizardFactory::postGenerateOpenEditors(const GeneratedFiles &l, QString *errorMessage)
{
    foreach (const GeneratedFile &file, l) {
        if (file.attributes() & GeneratedFile::OpenEditorAttribute) {
            if (!EditorManager::openEditor(file.path(), file.editorId())) {
                if (errorMessage)
                    *errorMessage = tr("Failed to open an editor for \"%1\".").arg(QDir::toNativeSeparators(file.path()));
                return false;
            }
        }
    }
    return true;
}

/*!
    Performs an overwrite check on a set of \a files. Checks if the file exists and
    can be overwritten at all, and then prompts the user with a summary.
*/

BaseFileWizardFactory::OverwriteResult BaseFileWizardFactory::promptOverwrite(GeneratedFiles *files,
                                                                QString *errorMessage) const
{
    if (debugWizard)
        qDebug() << Q_FUNC_INFO << files;

    QStringList existingFiles;
    bool oddStuffFound = false;

    static const QString readOnlyMsg = tr("[read only]");
    static const QString directoryMsg = tr("[folder]");
    static const QString symLinkMsg = tr("[symbolic link]");

    foreach (const GeneratedFile &file, *files) {
        const QFileInfo fi(file.path());
        if (fi.exists())
            existingFiles.append(file.path());
    }
    if (existingFiles.isEmpty())
        return OverwriteOk;
    // Before prompting to overwrite existing files, loop over files and check
    // if there is anything blocking overwriting them (like them being links or folders).
    // Format a file list message as ( "<file1> [readonly], <file2> [folder]").
    const QString commonExistingPath = Utils::commonPath(existingFiles);
    QString fileNamesMsgPart;
    foreach (const QString &fileName, existingFiles) {
        const QFileInfo fi(fileName);
        if (fi.exists()) {
            if (!fileNamesMsgPart.isEmpty())
                fileNamesMsgPart += QLatin1String(", ");
            fileNamesMsgPart += QDir::toNativeSeparators(fileName.mid(commonExistingPath.size() + 1));
            do {
                if (fi.isDir()) {
                    oddStuffFound = true;
                    fileNamesMsgPart += QLatin1Char(' ') + directoryMsg;
                    break;
                }
                if (fi.isSymLink()) {
                    oddStuffFound = true;
                    fileNamesMsgPart += QLatin1Char(' ') + symLinkMsg;
                    break;
            }
                if (!fi.isWritable()) {
                    oddStuffFound = true;
                    fileNamesMsgPart += QLatin1Char(' ') + readOnlyMsg;
                }
            } while (false);
        }
    }

    if (oddStuffFound) {
        *errorMessage = tr("The project directory %1 contains files which cannot be overwritten:\n%2.")
                .arg(QDir::toNativeSeparators(commonExistingPath)).arg(fileNamesMsgPart);
        return OverwriteError;
    }
    // Prompt to overwrite existing files.
    Internal::PromptOverwriteDialog overwriteDialog;
    // Scripts cannot handle overwrite
    overwriteDialog.setFiles(existingFiles);
    foreach (const GeneratedFile &file, *files)
        if (file.attributes() & GeneratedFile::CustomGeneratorAttribute)
            overwriteDialog.setFileEnabled(file.path(), false);
    if (overwriteDialog.exec() != QDialog::Accepted)
        return OverwriteCanceled;
    const QStringList existingFilesToKeep = overwriteDialog.uncheckedFiles();
    if (existingFilesToKeep.size() == files->size()) // All exist & all unchecked->Cancel.
        return OverwriteCanceled;
    // Set 'keep' attribute in files
    foreach (const QString &keepFile, existingFilesToKeep) {
        const int i = indexOfFile(*files, keepFile);
        QTC_ASSERT(i != -1, return OverwriteCanceled);
        GeneratedFile &file = (*files)[i];
        file.setAttributes(file.attributes() | GeneratedFile::KeepExistingFileAttribute);
    }
    return OverwriteOk;
}

/*!
    Constructs a file name, adding the \a extension unless \a baseName already has
    one.
*/

QString BaseFileWizardFactory::buildFileName(const QString &path,
                                      const QString &baseName,
                                      const QString &extension)
{
    QString rc = path;
    if (!rc.isEmpty() && !rc.endsWith(QDir::separator()))
        rc += QDir::separator();
    rc += baseName;
    // Add extension unless user specified something else
    const QChar dot = QLatin1Char('.');
    if (!extension.isEmpty() && !baseName.contains(dot)) {
        if (!extension.startsWith(dot))
            rc += dot;
        rc += extension;
    }
    if (debugWizard)
        qDebug() << Q_FUNC_INFO << rc;
    return rc;
}

/*!
    Returns the preferred suffix for \a mimeType.
*/

QString BaseFileWizardFactory::preferredSuffix(const QString &mimeType)
{
    const QString rc = MimeDatabase::preferredSuffixByType(mimeType);
    if (rc.isEmpty())
        qWarning("%s: WARNING: Unable to find a preferred suffix for %s.",
                 Q_FUNC_INFO, mimeType.toUtf8().constData());
    return rc;
}

/*!
    \class Core::StandardFileWizard
    \brief The StandardFileWizard class is a convenience class for
    creating one file.

    It uses Utils::FileWizardDialog and introduces a new virtual to generate the
    files from path and name.

    \sa Core::GeneratedFile, Core::BaseFileWizardParameters, Core::BaseFileWizard
    \sa Core::Internal::WizardEventLoop
*/

/*!
    \fn Core::GeneratedFiles Core::StandardFileWizard::generateFilesFromPath(const QString &path,
                                                                             const QString &name,
                                                                             QString *errorMessage) const = 0
    Creates the files with the \a name under the \a path.
*/

/*!
    Creates a Utils::FileWizardDialog.
*/

BaseFileWizard *StandardFileWizardFactory::create(QWidget *parent, const WizardDialogParameters &parameters) const
{
    BaseFileWizard *wizard = new BaseFileWizard(parent);
    wizard->setWindowTitle(tr("New %1").arg(displayName()));

    Utils::FileWizardPage *page = new Utils::FileWizardPage;
    if (parameters.flags().testFlag(WizardDialogParameters::ForceCapitalLetterForFileName))
        page->setForceFirstCapitalLetterForFileName(true);
    page->setPath(parameters.defaultPath());
    wizard->addPage(page);

    foreach (QWizardPage *p, parameters.extensionPages())
        wizard->addPage(p);
    return wizard;
}

/*!
    Retrieves \a path and \a fileName and calls \c generateFilesFromPath().
*/

GeneratedFiles StandardFileWizardFactory::generateFiles(const QWizard *w,
                                                 QString *errorMessage) const
{
    const Utils::Wizard *wizard = qobject_cast<const Utils::Wizard *>(w);
    Utils::FileWizardPage *page = wizard->find<Utils::FileWizardPage>();
    QTC_ASSERT(page, return GeneratedFiles());

    return generateFilesFromPath(page->path(), page->fileName(), errorMessage);
}

} // namespace Core

#include "basefilewizardfactory.moc"
