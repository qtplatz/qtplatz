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
#include "chemistrymode.hpp"
#include "mainwindow.hpp"
#include "constants.hpp"
#include <adlog/logger.hpp>
#include <adportable/debug_core.hpp>
#include <adlog/logging_handler.hpp>

#include <coreplugin/icore.h>
#include <coreplugin/icontext.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/mimedatabase.h>
#include <coreplugin/modemanager.h>
#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/minisplitter.h>
#include <coreplugin/outputpane.h>

#include <QAction>
#include <QMessageBox>
#include <QMainWindow>
#include <QMenu>
#include <QtPlugin>
#if QT_VERSION >= 0x050000
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QStackedWidget>
#else
#include <QtGui/QBoxLayout>
#include <QtGui/QStackedWidget>
#endif
#include <QTextEdit>
#include <QWidget>

using namespace chemistry;

ChemistryPlugin::ChemistryPlugin() : mode_( std::make_shared< ChemistryMode >( this ) )
                                   , mainWindow_( std::make_shared< MainWindow >() )
                                   , actSDFileOpen_(0)
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
    Q_UNUSED(arguments)
    Q_UNUSED(errorString)

    adportable::core::debug_core::instance()->hook( adlog::logging_handler::log );

    // 
    initialize_actions();

    mainWindow_->activateWindow();
    mainWindow_->createActions();

	const QList<int> gc = QList<int>() << Core::Constants::C_GLOBAL_ID;
    mode_->setContext( gc );
    if ( QWidget * widget = mainWindow_->createContents( mode_.get() ) )
        mode_->setWidget( widget );
    addObject( mode_.get() );

    return true;
}

void
ChemistryPlugin::initialize_actions()
{
    QIcon iconOpen;
    iconOpen.addFile( ":/dataproc/image/fileopen.png" );
    actSDFileOpen_ = new QAction( iconOpen, tr("SDFile open..."), this );
    connect( actSDFileOpen_, SIGNAL( triggered() ), mainWindow_.get(), SLOT( actSDFileOpen() ) );

	const QList<int> gc = QList<int>() << Core::Constants::C_GLOBAL_ID;
    if ( Core::ActionManager * am = Core::ICore::instance()->actionManager() ) {
        Core::ActionContainer * menu = am->createMenu( Constants::MENU_ID ); // Menu ID
        menu->menu()->setTitle( "Chemistry" );

        Core::Command * cmd = 0;

        cmd = am->registerAction( actSDFileOpen_, Constants::SDFILE_OPEN, gc );
        menu->addAction( cmd );

        am->actionContainer( Core::Constants::M_TOOLS )->addMenu( menu );
    }
}

void
ChemistryPlugin::extensionsInitialized()
{
	mainWindow_->OnInitialUpdate();
}

ExtensionSystem::IPlugin::ShutdownFlag
ChemistryPlugin::aboutToShutdown()
{ 
	return SynchronousShutdown;
}

void
ChemistryPlugin::triggerAction()
{
    QMessageBox::information(Core::ICore::instance()->mainWindow(),
                             tr("Action triggered"),
                             tr("This is an action from Chemistry."));
}

Q_EXPORT_PLUGIN2(Chemistry, ChemistryPlugin)

