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

#ifndef IWIZARDFACTORY_H
#define IWIZARDFACTORY_H

#include <coreplugin/core_global.h>
#include <coreplugin/id.h>
#include <coreplugin/featureprovider.h>

#include <QIcon>
#include <QObject>
#include <QVariantMap>

namespace Core {

namespace Internal { class CorePlugin; }

class CORE_EXPORT IWizardFactory
    : public QObject
{
    Q_OBJECT
public:
    enum WizardKind {
        FileWizard = 0x01,
        ClassWizard = 0x02,
        ProjectWizard = 0x04
    };
    Q_DECLARE_FLAGS(WizardKinds, WizardKind)
    enum WizardFlag {
        PlatformIndependent = 0x01,
        ForceCapitalLetterForFileName = 0x02
    };
    Q_DECLARE_FLAGS(WizardFlags, WizardFlag)

    IWizardFactory() : m_kind(FileWizard) { }

    QString id() const { return m_id; }
    WizardKind kind() const { return m_kind; }
    QIcon icon() const { return m_icon; }
    QString description() const { return m_description; }
    QString displayName() const { return m_displayName; }
    QString category() const { return m_category; }
    QString displayCategory() const { return m_displayCategory; }
    QString descriptionImage() const { return m_descriptionImage; }
    FeatureSet requiredFeatures() const { return m_requiredFeatures; }
    WizardFlags flags() const { return m_flags; }

    void setId(const QString &id) { m_id = id; }
    void setWizardKind(WizardKind kind) { m_kind = kind; }
    void setIcon(const QIcon &icon) { m_icon = icon; }
    void setDescription(const QString &description) { m_description = description; }
    void setDisplayName(const QString &displayName) { m_displayName = displayName; }
    void setCategory(const QString &category) { m_category = category; }
    void setDisplayCategory(const QString &displayCategory) { m_displayCategory = displayCategory; }
    void setDescriptionImage(const QString &descriptionImage) { m_descriptionImage = descriptionImage; }
    void setRequiredFeatures(const FeatureSet &featureSet) { m_requiredFeatures = featureSet; }
    void addRequiredFeature(const Feature &feature) { m_requiredFeatures |= feature; }
    void setFlags(WizardFlags flags) { m_flags = flags; }

    virtual void runWizard(const QString &path, QWidget *parent, const QString &platform, const QVariantMap &variables) = 0;

    bool isAvailable(const QString &platformName) const;
    QStringList supportedPlatforms() const;

    // Utility to find all registered wizards
    static QList<IWizardFactory*> allWizardFactories();
    // Utility to find all registered wizards of a certain kind
    static QList<IWizardFactory*> wizardFactoriesOfKind(WizardKind kind);
    static QStringList allAvailablePlatforms();
    static QString displayNameForPlatform(const QString &string);

    static void registerFeatureProvider(IFeatureProvider *provider);

private:
    static void destroyFeatureProvider();

    IWizardFactory::WizardKind m_kind;
    QIcon m_icon;
    QString m_description;
    QString m_displayName;
    QString m_id;
    QString m_category;
    QString m_displayCategory;
    FeatureSet m_requiredFeatures;
    WizardFlags m_flags;
    QString m_descriptionImage;

    friend class Internal::CorePlugin;
};

} // namespace Core

Q_DECLARE_OPERATORS_FOR_FLAGS(Core::IWizardFactory::WizardKinds)
Q_DECLARE_OPERATORS_FOR_FLAGS(Core::IWizardFactory::WizardFlags)

#endif // IWIZARDFACTORY_H
