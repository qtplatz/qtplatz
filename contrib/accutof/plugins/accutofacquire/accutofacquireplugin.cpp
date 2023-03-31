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

#include "accutofacquireplugin.hpp"
#include "constants.hpp"
#include "mode.hpp"
#include "mainwindow.hpp"
#include "iu5303afacade.hpp"
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
#include <adportable/debug.hpp>
#include <boost/dll/runtime_symbol_info.hpp>
#include <boost/filesystem/path.hpp>


namespace accutof { namespace acquire {
        class Mode;
    }
}

using namespace accutof::acquire;

acquirePlugin::acquirePlugin() : mainWindow_( new MainWindow() )
                             , mode_( std::make_shared< Mode >(this) )
{
}

acquirePlugin::~acquirePlugin()
{
}

bool
acquirePlugin::initialize( const QStringList &arguments, QString *errorString )
{
    Q_UNUSED(arguments)
    Q_UNUSED(errorString)

    mainWindow_->activateWindow();
    mainWindow_->createActions();

    const Core::Context context( ( "AccuTOFAcquire.MainView" ) );

    mode_->setId( "AccuTOFAcquire.MainView" );
    mode_->setContext( context );

    if ( QWidget * widget = mainWindow_->createContents( mode_.get() ) )
        mode_->setWidget( widget );

    addObject( mode_.get() );

    for ( auto iExtension : document::instance()->iControllers() ) {
        addObject( iExtension );
        connect( iExtension, &adextension::iController::connected, mainWindow_, &MainWindow::iControllerConnected );
    }

#if 0
    // no time function supported.
    if ( auto iExtension = document::instance()->iSequence() ) {
        MainWindow::instance()->getEditorFactories( *iExtension );
        addObject( iExtension );
    }
#endif

    QAction *action = new QAction(tr("AccuTOF action"), this);

    Core::ActionManager * am = Core::ActionManager::instance();
    Core::Command * cmd = am->registerAction(action, Constants::ACTION_ID, context );
    cmd->setDefaultKeySequence(QKeySequence(tr("Ctrl+Alt+Meta+A")));
    connect(action, SIGNAL(triggered()), this, SLOT(triggerAction()));

    Core::ActionContainer *menu = am->createMenu(Constants::MENU_ID);
    menu->menu()->setTitle(tr("AccuTOF"));
    menu->addAction(cmd);
    am->actionContainer(Core::Constants::M_TOOLS)->addMenu(menu);

    return true;
}

void
acquirePlugin::extensionsInitialized()
{
    document::instance()->initialSetup(); // load default control method
	mainWindow_->OnInitialUpdate();
}

ExtensionSystem::IPlugin::ShutdownFlag
acquirePlugin::aboutToShutdown()
{
    // Save settings
    // Disconnect from signals that are not needed during shutdown
    // Hide UI (if you add UI that is not in the main window directly)
    document::instance()->finalClose();

    if ( auto iExtension = document::instance()->iSequence() )
        removeObject( iExtension );

    for ( auto iExtension : document::instance()->iControllers() )
        removeObject( iExtension );

    if ( mode_ )
        removeObject( mode_.get() );

#if ! defined NDEBUG && 0
    ADDEBUG() << "## Shutdown "
              << boost::dll::this_line_location().stem()  << "\n\t" << boost::dll::this_line_location();
#endif

    return SynchronousShutdown;
}

void
acquirePlugin::triggerAction()
{
    QMessageBox::information(Core::ICore::instance()->mainWindow(),
                             tr("Action triggered"),
                             tr("This is an action from u5303a."));
}
