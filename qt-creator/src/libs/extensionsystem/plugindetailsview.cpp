// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "plugindetailsview.h"

#include "extensionsystemtr.h"
#include "pluginmanager.h"
#include "pluginspec.h"

#include <utils/algorithm.h>
#include <utils/layoutbuilder.h>

#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QLabel>
#include <QListWidget>
#include <QRegularExpression>
#include <QTextEdit>

using namespace ExtensionSystem;

/*!
    \class ExtensionSystem::PluginDetailsView
    \inheaderfile extensionsystem/plugindetailsview.h
    \inmodule QtCreator

    \brief The PluginDetailsView class implements a widget that displays the
    contents of a PluginSpec.

    Can be used for integration in the application that
    uses the plugin manager.

    \sa ExtensionSystem::PluginView
*/

namespace ExtensionSystem::Internal {

class PluginDetailsViewPrivate
{
public:
    PluginDetailsViewPrivate(PluginDetailsView *detailsView)
        : q(detailsView)
        , name(createContentsLabel())
        , version(createContentsLabel())
        , compatVersion(createContentsLabel())
        , vendor(createContentsLabel())
        , component(createContentsLabel())
        , url(createContentsLabel())
        , location(createContentsLabel())
        , platforms(createContentsLabel())
        , description(createTextEdit())
        , copyright(createContentsLabel())
        , license(createTextEdit())
        , dependencies(new QListWidget(q))
    {
        using namespace Layouting;

        // clang-format off
        Form {
            Tr::tr("Name:"), name, br,
            Tr::tr("Version:"), version, br,
            Tr::tr("Compatibility version:"), compatVersion, br,
            Tr::tr("Vendor:"), vendor, br,
            Tr::tr("Group:"), component, br,
            Tr::tr("URL:"), url, br,
            Tr::tr("Location:"), location, br,
            Tr::tr("Platforms:"), platforms, br,
            Tr::tr("Description:"), description, br,
            Tr::tr("Copyright:"), copyright, br,
            Tr::tr("License:"), license, br,
            Tr::tr("Dependencies:"), dependencies,
            noMargin
        }.attachTo(q);
        // clang-format on
    }

    PluginDetailsView *q = nullptr;

    QLabel *name = nullptr;
    QLabel *version = nullptr;
    QLabel *compatVersion = nullptr;
    QLabel *vendor = nullptr;
    QLabel *component = nullptr;
    QLabel *url = nullptr;
    QLabel *location = nullptr;
    QLabel *platforms = nullptr;
    QTextEdit *description = nullptr;
    QLabel *copyright = nullptr;
    QTextEdit *license = nullptr;
    QListWidget *dependencies = nullptr;

private:
    QLabel *createContentsLabel() {
        QLabel *label = new QLabel(q);
        label->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::TextSelectableByMouse);
        label->setOpenExternalLinks(true);
        return label;
    }
    QTextEdit *createTextEdit() {
        QTextEdit *textEdit = new QTextEdit(q);
        textEdit->setTabChangesFocus(true);
        textEdit->setReadOnly(true);
        return textEdit;
    }
};

} // namespace ExtensionSystem::Internal

using namespace Internal;

/*!
    Constructs a new view with given \a parent widget.
*/
PluginDetailsView::PluginDetailsView(QWidget *parent)
    : QWidget(parent)
    , d(new PluginDetailsViewPrivate(this))
{
}

/*!
    \internal
*/
PluginDetailsView::~PluginDetailsView()
{
    delete d;
}

/*!
    Reads the given \a spec and displays its values
    in this PluginDetailsView.
*/
void PluginDetailsView::update(PluginSpec *spec)
{
    d->name->setText(spec->name());
    const QString revision = spec->revision();
    const QString versionString = spec->version()
            + (revision.isEmpty() ? QString() : " (" + revision + ")");
    d->version->setText(versionString);
    d->compatVersion->setText(spec->compatVersion());
    d->vendor->setText(spec->vendor());
    d->component->setText(spec->category().isEmpty() ? Tr::tr("None") : spec->category());
    d->url->setText(QString::fromLatin1("<a href=\"%1\">%1</a>").arg(spec->url()));
    d->location->setText(QDir::toNativeSeparators(spec->filePath()));
    const QString pattern = spec->platformSpecification().pattern();
    const QString platform = pattern.isEmpty() ? Tr::tr("All") : pattern;
    const QString platformString = Tr::tr("%1 (current: \"%2\")")
                                   .arg(platform, PluginManager::platformName());
    d->platforms->setText(platformString);
    QString description = spec->description();
    if (!description.isEmpty() && !spec->longDescription().isEmpty())
        description += "\n\n";
    description += spec->longDescription();
    d->description->setText(description);
    d->copyright->setText(spec->copyright());
    d->license->setText(spec->license());
    d->dependencies->addItems(Utils::transform<QList>(spec->dependencies(),
                                                      &PluginDependency::toString));
}

void PluginDetailsView::showModal(QWidget *parent, PluginSpec *spec)
{
    auto dialog = new QDialog(parent);
    dialog->setModal(true);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle(Tr::tr("Plugin Details of %1").arg(spec->name()));
    auto details = new ExtensionSystem::PluginDetailsView(dialog);
    details->update(spec);
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Close,
                                                     Qt::Horizontal,
                                                     dialog);

    // clang-format off
    using namespace Layouting;
    Column {
        details,
        buttons,
    }.attachTo(dialog);
    // clang-format on

    connect(buttons, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
    dialog->resize(400, 500);
    dialog->show();
}
