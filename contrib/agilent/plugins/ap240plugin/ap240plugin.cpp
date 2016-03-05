// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
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

#include "ap240plugin.hpp"
#include "ap240_constants.hpp"
#include "ap240mode.hpp"
#include "mainwindow.hpp"
#include "document.hpp"
#include "icontrollerimpl.hpp"
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

using namespace ap240::Internal;
using namespace ap240;

ap240Plugin::ap240Plugin() : mainWindow_( new MainWindow() )
                           , mode_( std::make_shared< ap240Mode >(this) )
{
    // Create your members
}

ap240Plugin::~ap240Plugin()
{
    // Unregister objects from the plugin manager's object pool
    // Delete members
    if ( mode_ )
        removeObject( mode_.get() );

    // if ( auto iExtension = document::instance()->iSequence() )
    //     removeObject( iExtension );

    // if ( auto iExtension = document::instance()->iController() )
    //     removeObject( iExtension );
}

bool
ap240Plugin::initialize( const QStringList &arguments, QString *errorString )
{
    adportable::core::debug_core::instance()->hook( adlog::logging_handler::log );

    Q_UNUSED(arguments)
    Q_UNUSED(errorString)

    mainWindow_->activateWindow();
    mainWindow_->createActions();

    //const Core::Context gc( ( Core::Id( Core::Constants::C_GLOBAL ) ) );
    const Core::Context context( ( "AP240.MainView" ) );
    mode_->setContext( context );
    if ( QWidget * widget = mainWindow_->createContents( mode_.get() ) )
        mode_->setWidget( widget );
    addObject( mode_.get() );

    // this may refer from MALPIXAcquire module
    if ( auto iExtension = document::instance()->iSequence() ) {
         mainWindow_->editor_factories( *iExtension );
         addObject( iExtension );
    }
    
    if ( auto iExtension = document::instance()->iController() ) {
        addObject( iExtension );
    }

    QAction *action = new QAction(tr("ap240 action"), this);

    if ( Core::ActionManager * am = Core::ActionManager::instance() ) {

        if ( Core::Command * cmd = am->registerAction( action, Constants::ACTION_ID, context ) ) {
            cmd->setDefaultKeySequence( QKeySequence( tr( "Ctrl+Alt+Meta+A" ) ) );
            connect( action, SIGNAL( triggered() ), this, SLOT( triggerAction() ) );

            Core::ActionContainer *menu = am->createMenu( Constants::MENU_ID );
            menu->menu()->setTitle( tr( "AP240" ) );
            menu->addAction( cmd );
            am->actionContainer( Core::Constants::M_TOOLS )->addMenu( menu );
        }
    }

    return true;
}

void
ap240Plugin::extensionsInitialized()
{
    document::instance()->initialSetup(); // load default control method
	mainWindow_->OnInitialUpdate();
}

ExtensionSystem::IPlugin::ShutdownFlag
ap240Plugin::aboutToShutdown()
{
    // Save settings
    // Disconnect from signals that are not needed during shutdown
    // Hide UI (if you add UI that is not in the main window directly)
    document::instance()->finalClose();

    if ( auto iExtension = document::instance()->iSequence() )
        removeObject( iExtension );

    if ( auto iExtension = document::instance()->iController() )
        removeObject( iExtension );
    
    return SynchronousShutdown;
}

void
ap240Plugin::triggerAction()
{
    QMessageBox::information(Core::ICore::instance()->mainWindow(),
                             tr("Action triggered"),
                             tr("This is an action from ap240."));
}

Q_EXPORT_PLUGIN2(ap240, ap240Plugin)

