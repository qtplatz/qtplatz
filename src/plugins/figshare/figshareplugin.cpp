// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "figshareplugin.hpp"
#include "mainwindow.hpp"
#include <adportable/debug.hpp>

#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/icore.h>
#include <coreplugin/imode.h>
#include <coreplugin/modemanager.h>

#include <QDebug>
#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>

namespace figshare {

/*!  A mode with a push button based on BaseMode.  */

    class Mode : public Core::IMode
    {
    public:
        Mode()
            {
                setContext(Core::Context("figshare.MainView"));
                setDisplayName(tr("figshare"));
                setIcon(QIcon(":/figshare/image/figshare.png"));
                setPriority(0);
                setId("figshare.figshareMode");
            }
    };

    class FigsharePlugin::impl {
    public:
        std::unique_ptr< Mode > mode_;
        std::unique_ptr< MainWindow > mainWindow_;
        impl() : mode_( nullptr )
               , mainWindow_( nullptr ) {}
        ~impl() {}
    };

}

using namespace figshare;

FigsharePlugin::FigsharePlugin() : impl_( new FigsharePlugin::impl() )
{
}


FigsharePlugin::~FigsharePlugin()
{
    delete impl_;
}

bool
FigsharePlugin::initialize(const QStringList &arguments, QString *errorMessage)
{
    if (( impl_->mainWindow_ = std::make_unique< MainWindow >() )) {
        impl_->mainWindow_->activateWindow();
        impl_->mainWindow_->createActions();

        if ( QWidget * widget = impl_->mainWindow_->createContents() ) {
            if (( impl_->mode_ = std::make_unique< Mode >() )) {
                impl_->mode_->setWidget( widget );
                // ExtensionSystem::PluginManager::addObject( mode_.get() );
            }
        }
    }
    // Create a unique context for our own view, that will be used for the
    // menu entry later.
    Core::Context context("figshare.MainView");

    // Create an action to be triggered by a menu entry
    auto action = new QAction(tr("Say \"&Hello World!\""), this);
    connect(action, &QAction::triggered, this, &FigsharePlugin::sayHello );

    // Register the action with the action manager
    Core::Command *command =
        Core::ActionManager::registerAction( action, "figshare.figshareAction", context);

    // Create our own menu to place in the Tools menu
    Core::ActionContainer *restMenu =  Core::ActionManager::createMenu("figshare.figshareMenu");
    QMenu *menu = restMenu->menu();
    menu->setTitle(tr("&figshare"));
    menu->setEnabled( true );

    // Add the Hello World action command to the menu
    restMenu->addAction(command);

    // Request the Tools menu and add the Hello World menu to it
    Core::ActionContainer *toolsMenu =
        Core::ActionManager::actionContainer(Core::Constants::M_TOOLS);
    toolsMenu->addMenu(restMenu);

    return true;
}

void
FigsharePlugin::extensionsInitialized()
{
	impl_->mainWindow_->OnInitialUpdate();
}

void
FigsharePlugin::sayHello()
{
    // When passing nullptr for the parent, the message box becomes an
    // application-global modal dialog box
    QMessageBox::information(
            nullptr, tr("Hello World!"), tr("Hello World! Beautiful day today, isn't it?"));
}
