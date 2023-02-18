// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "basefilewizardfactory.h"

#include "basefilewizard.h"
#include "dialogs/promptoverwritedialog.h"
#include "editormanager/editormanager.h"
#include "icontext.h"
#include "icore.h"
#include "ifilewizardextension.h"
#include <extensionsystem/pluginmanager.h>
#include <utils/filewizardpage.h>
#include <utils/mimeutils.h>
#include <utils/qtcassert.h>
#include <utils/stringutils.h>
#include <utils/wizard.h>

#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QIcon>

enum { debugWizard = 0 };

using namespace Utils;

namespace Core {

static int indexOfFile(const GeneratedFiles &f, const FilePath &path)
{
    const int size = f.size();
    for (int i = 0; i < size; ++i)
        if (f.at(i).filePath() == path)
            return i;
    return -1;
}

/*!
    \class Core::BaseFileWizard
    \inheaderfile coreplugin/basefilewizardfactory.h
    \inmodule QtCreator

    \brief The BaseFileWizard class implements a is a convenience class for
    creating files.

    \sa Core::BaseFileWizardFactory
*/

Utils::Wizard *BaseFileWizardFactory::runWizardImpl(const FilePath &path, QWidget *parent,
                                                    Id platform,
                                                    const QVariantMap &extraValues,
                                                    bool showWizard)
{
    Q_UNUSED(showWizard);
    QTC_ASSERT(!path.isEmpty(), return nullptr);

    // Create dialog and run it. Ensure that the dialog is deleted when
    // leaving the func, but not before the IFileWizardExtension::process
    // has been called

    WizardDialogParameters::DialogParameterFlags dialogParameterFlags;

    if (flags().testFlag(ForceCapitalLetterForFileName))
        dialogParameterFlags |= WizardDialogParameters::ForceCapitalLetterForFileName;

    Wizard *wizard = create(parent, WizardDialogParameters(path,
                                                           platform,
                                                           requiredFeatures(),
                                                           dialogParameterFlags,
                                                           extraValues));
    QTC_CHECK(wizard);
    return wizard;
}

/*!
    \class Core::BaseFileWizardFactory
    \inheaderfile coreplugin/basefilewizardfactory.h
    \inmodule QtCreator

    \brief The BaseFileWizardFactory class implements a generic wizard for
    creating files.

    The following abstract functions must be implemented:
    \list
    \li create(): Called to create the QWizard dialog to be shown.
    \li generateFiles(): Generates file content.
    \endlist

    The behavior can be further customized by overwriting the virtual function
    postGenerateFiles(), which is called after generating the files.

    \note Instead of using this class, we recommend that you create JSON-based
    wizards, as instructed in \l{https://doc.qt.io/qtcreator/creator-project-wizards.html}
    {Adding New Custom Wizards}.

    \sa Core::GeneratedFile, Core::WizardDialogParameters, Core::BaseFileWizard
*/

/*!
    \fn Core::BaseFileWizard *Core::BaseFileWizardFactory::create(QWidget *parent,
                                                                  const Core::WizardDialogParameters &parameters) const

    Creates the wizard on the \a parent with the \a parameters.
*/

/*!
    \fn virtual Core::GeneratedFiles Core::BaseFileWizardFactory::generateFiles(const QWizard *w,
                                                                                QString *errorMessage) const
    Overwrite to query the parameters from the wizard \a w and generate the
    files.

    Possible errors are held in \a errorMessage.

    \note This does not generate physical files, but merely the list of
    Core::GeneratedFile.
*/

/*!
    Physically writes \a files.

    If the files cannot be written, returns \c false and sets \a errorMessage
    to the message that is displayed to users.

    Re-implement (calling the base implementation) to create files with
    GeneratedFile::CustomGeneratorAttribute set.
*/

bool BaseFileWizardFactory::writeFiles(const GeneratedFiles &files, QString *errorMessage) const
{
    const GeneratedFile::Attributes noWriteAttributes
        = GeneratedFile::CustomGeneratorAttribute|GeneratedFile::KeepExistingFileAttribute;
    for (const GeneratedFile &generatedFile : std::as_const(files))
        if (!(generatedFile.attributes() & noWriteAttributes ))
            if (!generatedFile.write(errorMessage))
                return false;
    return true;
}

/*!
    Overwrite to perform steps to be done by the wizard \a w after the files
    specified by \a l are actually created.

    The default implementation opens editors with the newly generated files
    that have GeneratedFile::OpenEditorAttribute set.

    Returns \a errorMessage if errors occur.
*/

bool BaseFileWizardFactory::postGenerateFiles(const QWizard *, const GeneratedFiles &l,
                                              QString *errorMessage) const
{
    return BaseFileWizardFactory::postGenerateOpenEditors(l, errorMessage);
}

/*!
    Opens the editors for the files \a l if their
    GeneratedFile::OpenEditorAttribute attribute
    is set accordingly.

    If the editorrs cannot be opened, returns \c false and dand sets
    \a errorMessage to the message that is displayed to users.
*/

bool BaseFileWizardFactory::postGenerateOpenEditors(const GeneratedFiles &l, QString *errorMessage)
{
    for (const GeneratedFile &file : std::as_const(l)) {
        if (file.attributes() & GeneratedFile::OpenEditorAttribute) {
            if (!EditorManager::openEditor(file.filePath(), file.editorId())) {
                if (errorMessage)
                    *errorMessage = tr("Failed to open an editor for \"%1\".").
                        arg(file.filePath().toUserOutput());
                return false;
            }
        }
    }
    return true;
}

/*!
    Performs an overwrite check on a set of \a files. Checks if the file exists and
    can be overwritten at all, and then prompts the user with a summary.

    Returns \a errorMessage if the file cannot be overwritten.
*/

BaseFileWizardFactory::OverwriteResult BaseFileWizardFactory::promptOverwrite(GeneratedFiles *files,
                                                                QString *errorMessage)
{
    if (debugWizard)
        qDebug() << Q_FUNC_INFO << files;

    QStringList existingFiles;
    bool oddStuffFound = false;

    static const QString readOnlyMsg = tr("[read only]");
    static const QString directoryMsg = tr("[folder]");
    static const QString symLinkMsg = tr("[symbolic link]");

    for (const GeneratedFile &file : std::as_const(*files)) {
        const FilePath path = file.filePath();
        if (path.exists())
            existingFiles.append(path.toString());
    }
    if (existingFiles.isEmpty())
        return OverwriteOk;
    // Before prompting to overwrite existing files, loop over files and check
    // if there is anything blocking overwriting them (like them being links or folders).
    // Format a file list message as ( "<file1> [readonly], <file2> [folder]").
    const QString commonExistingPath = Utils::commonPath(existingFiles);
    QString fileNamesMsgPart;
    for (const QString &fileName : std::as_const(existingFiles)) {
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
                .arg(QDir::toNativeSeparators(commonExistingPath), fileNamesMsgPart);
        return OverwriteError;
    }
    // Prompt to overwrite existing files.
    PromptOverwriteDialog overwriteDialog;
    // Scripts cannot handle overwrite
    overwriteDialog.setFiles(existingFiles);
    for (const GeneratedFile &file : std::as_const(*files))
        if (file.attributes() & GeneratedFile::CustomGeneratorAttribute)
            overwriteDialog.setFileEnabled(file.filePath().toString(), false);
    if (overwriteDialog.exec() != QDialog::Accepted)
        return OverwriteCanceled;
    const QStringList existingFilesToKeep = overwriteDialog.uncheckedFiles();
    if (existingFilesToKeep.size() == files->size()) // All exist & all unchecked->Cancel.
        return OverwriteCanceled;
    // Set 'keep' attribute in files
    for (const QString &keepFile : std::as_const(existingFilesToKeep)) {
        const int i = indexOfFile(*files, FilePath::fromString(keepFile).cleanPath());
        QTC_ASSERT(i != -1, return OverwriteCanceled);
        GeneratedFile &file = (*files)[i];
        file.setAttributes(file.attributes() | GeneratedFile::KeepExistingFileAttribute);
    }
    return OverwriteOk;
}

/*!
    Constructs a file name including \a path, adding the \a extension unless
    \a baseName already has one.
*/

FilePath BaseFileWizardFactory::buildFileName(const FilePath &path,
                                              const QString &baseName,
                                              const QString &extension)
{
    FilePath rc = path.pathAppended(baseName);
    // Add extension unless user specified something else
    const QChar dot = '.';
    if (!extension.isEmpty() && !baseName.contains(dot)) {
        if (!extension.startsWith(dot))
            rc = rc.stringAppended(dot);
        rc = rc.stringAppended(extension);
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
    QString rc;
    Utils::MimeType mt = Utils::mimeTypeForName(mimeType);
    if (mt.isValid())
        rc = mt.preferredSuffix();
    if (rc.isEmpty())
        qWarning("%s: WARNING: Unable to find a preferred suffix for %s.",
                 Q_FUNC_INFO, mimeType.toUtf8().constData());
    return rc;
}

/*!
    \class Core::WizardDialogParameters
    \inheaderfile coreplugin/basefilewizardfactory.h
    \inmodule QtCreator

    \brief The WizardDialogParameters class holds parameters for the new file
    wizard dialog.

    \sa Core::GeneratedFile, Core::BaseFileWizardFactory
*/

/*!
    \enum Core::WizardDialogParameters::DialogParameterEnum
    This enum type holds whether to force capital letters for file names.
    \value ForceCapitalLetterForFileName Forces capital letters for file names.
*/

} // namespace Core
