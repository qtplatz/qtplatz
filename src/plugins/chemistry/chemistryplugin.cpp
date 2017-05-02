/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#include "chemistryplugin.hpp"
#include "chemistryconstants.hpp"
#include "mode.hpp"
#include "mainwindow.hpp"
#include "constants.hpp"
#include <adlog/logger.hpp>
#include <adportable/debug.hpp>
#include <adlog/logging_handler.hpp>

#include <coreplugin/icore.h>
#include <coreplugin/icontext.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/mimedatabase.h>
#include <coreplugin/modemanager.h>
#include <coreplugin/id.h>
#include <coreplugin/minisplitter.h>
#include <coreplugin/outputpane.h>

#include <QAction>
#include <QMessageBox>
#include <QMainWindow>
#include <QMenu>
#include <QtPlugin>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QStackedWidget>
#include <QTextEdit>
#include <QWidget>
#include <memory>

using namespace chemistry;

ChemistryPlugin::ChemistryPlugin() : mode_( std::make_shared< Mode >( this ) )
                                   , mainWindow_( std::make_shared< MainWindow >() )
{
    // Create your members
}

ChemistryPlugin::~ChemistryPlugin()
{
    // Unregister objects from the plugin manager's object pool
    // Delete members
    if ( mode_ )
		removeObject( mode_.get() );
}

bool
ChemistryPlugin::initialize(const QStringList &arguments, QString *errorString)
{
    // ADDEBUG() << "##### ChemistryPlugin initialize...";
    
    // 
    initialize_actions();

    mainWindow_->activateWindow();
    mainWindow_->createActions();

    if ( QWidget * widget = mainWindow_->createContents( mode_.get() ) )
        mode_->setWidget( widget );
    addObject( mode_.get() );
    
    // ADDEBUG() << "ChemistryPlugin initialized";    
    return true;
}

void
ChemistryPlugin::initialize_actions()
{
    const Core::Context gc( (Core::Id( Core::Constants::C_GLOBAL )) );
    
	if ( Core::ActionManager *am = Core::ActionManager::instance() ) {

        // File->Processing
        if ( Core::ActionContainer * menu = am->createMenu( "chemistry.menu" ) ) {
            do {
                QIcon iconOpen;
                iconOpen.addFile( ":/dataproc/image/fileopen.png" );
                if ( auto p = new QAction( iconOpen, tr("SDFile open..."), this ) ) {
                    am->registerAction( p, Constants::SDFILE_OPEN, gc );
                    connect( p, &QAction::triggered, mainWindow_.get(), &MainWindow::actSDFileOpen );
                }
            } while ( 0 );

            menu->menu()->setTitle( tr("Chemistry") );
            menu->addAction( am->command( Constants::SDFILE_OPEN ) );
            am->actionContainer( Core::Constants::M_TOOLS )->addMenu( menu );
        }
        
    }
}

void
ChemistryPlugin::extensionsInitialized()
{
	mainWindow_->OnInitialUpdate();
    // ADDEBUG() << "ChemistryPlugin extensionsInitialized.";
}

ExtensionSystem::IPlugin::ShutdownFlag
ChemistryPlugin::aboutToShutdown()
{ 
	return SynchronousShutdown;
	mainWindow_->OnClose();
}

void
ChemistryPlugin::triggerAction()
{
    QMessageBox::information(Core::ICore::instance()->mainWindow(),
                             tr("Action triggered"),
                             tr("This is an action from Chemistry."));
}

Q_EXPORT_PLUGIN2(Chemistry, ChemistryPlugin)
