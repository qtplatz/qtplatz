// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/

#include "acquireplugin.hpp"
#include "constants.hpp"
#include "mode.hpp"
#include "mainwindow.hpp"
#include "idgmodimpl.hpp"
#include "document.hpp"
#include <adcontrols/massspectrometerbroker.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adextension/isequenceimpl.hpp>
#include <adportable/debug_core.hpp>
#include <adlog/logging_handler.hpp>
#include <coreplugin/icore.h>
#include <coreplugin/icontext.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/id.h>
#include <coreplugin/modemanager.h>

#include <QAction>
#include <QMessageBox>
#include <QMainWindow>
#include <QMenu>
#include <QtPlugin>

using namespace acquire;

acquireplugin::acquireplugin() : mainWindow_( new MainWindow() )
                               , mode_( std::make_unique< Mode >(this) )
{
}

acquireplugin::~acquireplugin()
{
}

bool
acquireplugin::initialize( const QStringList &arguments, QString *errorString )
{
    Q_UNUSED(arguments)
    Q_UNUSED(errorString)

    mainWindow_->activateWindow();
    mainWindow_->createActions();

    const Core::Context context( ( "ACQUIRE.MainView" ) );

    mode_->setId( "ACQUIRE.MainView" );
    mode_->setContext( context );

    if ( QWidget * widget = mainWindow_->createContents( mode_.get() ) )
        mode_->setWidget( widget );

    addObject( mode_.get() );

    // add instrument controller
    for ( auto iController : document::instance()->iControllers() ) {
        addObject( iController );
        connect( iController, &adextension::iController::connected, mainWindow_, &MainWindow::iControllerConnected );
    }

    // no time function supported.
    if ( auto iExtension = document::instance()->iSequence() ) {
        MainWindow::instance()->getEditorFactories( *iExtension );
        addObject( iExtension );
    }

    QAction *action = new QAction(tr("acquire action"), this);

    Core::ActionManager * am = Core::ActionManager::instance();
    Core::Command * cmd = am->registerAction(action, Constants::ACTION_ID, context );
    cmd->setDefaultKeySequence(QKeySequence(tr("Ctrl+Alt+Meta+A")));
    connect(action, SIGNAL(triggered()), this, SLOT(triggerAction()));

    Core::ActionContainer *menu = am->createMenu(Constants::MENU_ID);
    menu->menu()->setTitle(tr("ACQUIRE"));
    menu->addAction(cmd);
    am->actionContainer(Core::Constants::M_TOOLS)->addMenu(menu);

    return true;
}

void
acquireplugin::extensionsInitialized()
{
    document::instance()->initialSetup(); // load default control method
	mainWindow_->OnInitialUpdate();
}

ExtensionSystem::IPlugin::ShutdownFlag
acquireplugin::aboutToShutdown()
{
    // Save settings
    // Disconnect from signals that are not needed during shutdown
    // Hide UI (if you add UI that is not in the main window directly)
    document::instance()->finalClose();

    if ( auto iExtension = document::instance()->iSequence() )
        removeObject( iExtension );

    for ( auto iExtension: document::instance()->iControllers() )
        removeObject( iExtension );

    if ( mode_ )
        removeObject( mode_.get() );

    return SynchronousShutdown;
}

void
acquireplugin::triggerAction()
{
    QMessageBox::information(Core::ICore::instance()->mainWindow(),
                             tr("Action triggered"),
                             tr("This is an action from acquire."));
}

Q_EXPORT_PLUGIN2(acquire, acquireplugin)
