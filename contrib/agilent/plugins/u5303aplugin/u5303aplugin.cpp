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

#include "u5303aplugin.hpp"
#include "u5303a_constants.hpp"
#include "u5303amode.hpp"
#include "mainwindow.hpp"
#include "icontrollerimpl.hpp"
#include "document.hpp"
#include <adcontrols/massspectrometerbroker.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adextension/iSequenceimpl.hpp>
#include <adportable/debug_core.hpp>
#include <adlog/logging_handler.hpp>
#include <u5303aspectrometer/massspectrometer.hpp>
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

using namespace u5303a::Internal;
using namespace u5303a;

u5303APlugin::u5303APlugin() : mainWindow_( new MainWindow() )
                             , mode_( std::make_shared< u5303AMode >(this) )
{
}

u5303APlugin::~u5303APlugin()
{
}

bool
u5303APlugin::initialize( const QStringList &arguments, QString *errorString )
{
    adportable::core::debug_core::instance()->hook( adlog::logging_handler::log );

    Q_UNUSED(arguments)
    Q_UNUSED(errorString)

    mainWindow_->activateWindow();
        
    const Core::Context context( ( "U5303A.MainView" ) );
    mode_->setContext( context );    

    mainWindow_->createActions( context );
    
    if ( QWidget * widget = mainWindow_->createContents( mode_.get() ) )
        mode_->setWidget( widget );

    addObject( mode_.get() );

    if ( auto iExtension = document::instance()->iController() ) {
        addObject( iExtension );
        connect( iExtension, &adextension::iController::connected, mainWindow_, &MainWindow::iControllerConnected );
    }

    if ( auto iExtension = document::instance()->iSequence() ) {
        MainWindow::instance()->getEditorFactories( *iExtension );
        addObject( iExtension );
    }
    
    QAction *action = new QAction(tr("u5303A action"), this);

    Core::ActionManager * am = Core::ActionManager::instance();
    Core::Command * cmd = am->registerAction(action, Constants::ACTION_ID, context );
    cmd->setDefaultKeySequence(QKeySequence(tr("Ctrl+Alt+Meta+A")));
    connect(action, SIGNAL(triggered()), this, SLOT(triggerAction()));

    Core::ActionContainer *menu = am->createMenu(Constants::MENU_ID);
    menu->menu()->setTitle(tr("U5303A"));
    menu->addAction(cmd);
    am->actionContainer(Core::Constants::M_TOOLS)->addMenu(menu);

    return true;
}

void
u5303APlugin::extensionsInitialized()
{
    auto factory = u5303aspectrometer::MassSpectrometer::instance();
	adcontrols::massSpectrometerBroker::register_factory( factory, factory->name() );
    document::instance()->initialSetup(); // load default control method
	mainWindow_->OnInitialUpdate();
}

ExtensionSystem::IPlugin::ShutdownFlag
u5303APlugin::aboutToShutdown()
{
    // Save settings
    // Disconnect from signals that are not needed during shutdown
    // Hide UI (if you add UI that is not in the main window directly)
    document::instance()->finalClose();

    if ( auto iExtension = document::instance()->iSequence() )
        removeObject( iExtension );
    
    if ( auto iExtension = document::instance()->iController() )
        removeObject( iExtension );

    if ( mode_ )
        removeObject( mode_.get() );

    return SynchronousShutdown;
}

void
u5303APlugin::triggerAction()
{
    QMessageBox::information(Core::ICore::instance()->mainWindow(),
                             tr("Action triggered"),
                             tr("This is an action from u5303a."));
}

Q_EXPORT_PLUGIN2(u5303a, u5303APlugin)

