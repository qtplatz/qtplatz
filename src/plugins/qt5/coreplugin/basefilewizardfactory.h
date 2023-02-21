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

#ifndef BASEFILEWIZARDFACTORY_H
#define BASEFILEWIZARDFACTORY_H

#include "core_global.h"
#include "generatedfile.h"
#include "featureprovider.h"

#include <coreplugin/iwizardfactory.h>

#include <extensionsystem/iplugin.h>

#include <QSharedDataPointer>
#include <QList>

QT_BEGIN_NAMESPACE
class QIcon;
class QWizard;
class QWizardPage;
class QDebug;
QT_END_NAMESPACE

namespace Utils { class Wizard; }

namespace Core {

class BaseFileWizard;

class CORE_EXPORT WizardDialogParameters
{
public:
    typedef QList<QWizardPage *> WizardPageList;

    enum DialogParameterEnum {
        ForceCapitalLetterForFileName = 0x01
    };
    Q_DECLARE_FLAGS(DialogParameterFlags, DialogParameterEnum)

    explicit WizardDialogParameters(const QString &defaultPath, const WizardPageList &extensionPages,
                                    const QString &platform, const Core::FeatureSet &requiredFeatures,
                                    DialogParameterFlags flags,
                                    QVariantMap extraValues)
        : m_defaultPath(defaultPath),
          m_extensionPages(extensionPages),
          m_selectedPlatform(platform),
          m_requiredFeatures(requiredFeatures),
          m_parameterFlags(flags),
          m_extraValues(extraValues)
    {}

    QString defaultPath() const
    { return m_defaultPath; }

    WizardPageList extensionPages() const
    { return m_extensionPages; }

    QString selectedPlatform() const
    { return m_selectedPlatform; }

    Core::FeatureSet requiredFeatures() const
    { return m_requiredFeatures; }

    DialogParameterFlags flags() const
    { return m_parameterFlags; }

    QVariantMap extraValues() const
    { return m_extraValues; }

private:
    QString m_defaultPath;
    WizardPageList m_extensionPages;
    QString m_selectedPlatform;
    Core::FeatureSet m_requiredFeatures;
    DialogParameterFlags m_parameterFlags;
    QVariantMap m_extraValues;
};

class CORE_EXPORT BaseFileWizardFactory : public IWizardFactory
{
    Q_OBJECT

public:
    // IWizard
    void runWizard(const QString &path, QWidget *parent, const QString &platform, const QVariantMap &extraValues);

    static QString buildFileName(const QString &path, const QString &baseName, const QString &extension);

protected:
    typedef QList<QWizardPage *> WizardPageList;

    virtual BaseFileWizard *create(QWidget *parent, const WizardDialogParameters &parameters) const = 0;

    virtual GeneratedFiles generateFiles(const QWizard *w,
                                         QString *errorMessage) const = 0;

    virtual bool writeFiles(const GeneratedFiles &files, QString *errorMessage);

    virtual bool postGenerateFiles(const QWizard *w, const GeneratedFiles &l, QString *errorMessage);

    static QString preferredSuffix(const QString &mimeType);

    enum OverwriteResult { OverwriteOk,  OverwriteError,  OverwriteCanceled };
    OverwriteResult promptOverwrite(GeneratedFiles *files,
                                    QString *errorMessage) const;
    static bool postGenerateOpenEditors(const GeneratedFiles &l, QString *errorMessage = 0);
};

class CORE_EXPORT StandardFileWizardFactory : public BaseFileWizardFactory
{
    Q_OBJECT

protected:
    BaseFileWizard *create(QWidget *parent, const WizardDialogParameters &parameters) const;
    GeneratedFiles generateFiles(const QWizard *w, QString *errorMessage) const;
    virtual GeneratedFiles generateFilesFromPath(const QString &path, const QString &name,
                                                 QString *errorMessage) const = 0;
};

} // namespace Core

Q_DECLARE_OPERATORS_FOR_FLAGS(Core::GeneratedFile::Attributes)
Q_DECLARE_OPERATORS_FOR_FLAGS(Core::WizardDialogParameters::DialogParameterFlags)

#endif // BASEFILEWIZARDFACTORY_H
