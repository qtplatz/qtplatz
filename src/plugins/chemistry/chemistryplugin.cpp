/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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
#include "chemistrymanager.hpp"
#include "chemeditorfactory.hpp"
#include "sdfileview.hpp"
#include "constants.hpp"
#include <adportable/debug.hpp>

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
#include <QtGui/QBoxLayout>
#include <QtGui/QStackedWidget>
#include <qtextedit.h>

using namespace Chemistry::Internal;

ChemistryPlugin::ChemistryPlugin()
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

    Core::ActionManager *am = Core::ICore::instance()->actionManager();
    
    QAction *action = new QAction(tr("Chemistry action"), this);
	QList<int> globalcontext;
	globalcontext << Core::Constants::C_GLOBAL_ID;
    Core::Command *cmd = am->registerAction(action, Constants::ACTION_ID, globalcontext );
    cmd->setDefaultKeySequence(QKeySequence(tr("Ctrl+Alt+Meta+A")));
    connect(action, SIGNAL(triggered()), this, SLOT(triggerAction()));
    
    Core::ActionContainer *menu = am->createMenu(Constants::MENU_ID);
    menu->menu()->setTitle(tr("Chemistry"));
    menu->addAction(cmd);
    am->actionContainer(Core::Constants::M_TOOLS)->addMenu(menu);

	//--- add configuration xml loading if necessary
    //---
    //---
	do {
		mode_.reset( new ChemistryMode( this ) );
		if ( ! mode_ )
			return false;
		QList<int> context;
		Core::UniqueIDManager * uidm = Core::ICore::instance()->uniqueIDManager();
		if ( ! uidm )
			return false;
		context.append( uidm->uniqueIdentifier( Constants::C_CHEM_EDITOR ) );
		context.append( uidm->uniqueIdentifier( Core::Constants::C_EDITORMANAGER ) );
        mode_->setContext( context );
	} while( 0 );
    do {
       Core::MimeDatabase * mdb = Core::ICore::instance()->mimeDatabase();
       if ( ! mdb )
		   return false;
       if ( ! mdb->addMimeTypes( ":/chemistry/mimetype.xml", errorString ) )
		   adportable::debug( __FILE__, __LINE__ ) << "addMimeTypes: " << errorString;

       QStringList mtypes;
       mtypes << "application/sdf" << "application/mdl";
       addAutoReleasedObject( new ChemEditorFactory( this, mtypes ) );
	} while( 0 );
	//<-------
	do {
		manager_.reset( new ChemistryManager( 0 ) );
		if ( ! manager_ )
			return false;
		manager_->init();
	} while( 0 );

	//-----------------
	Core::MiniSplitter * splitter = new Core::MiniSplitter;
    if ( ! splitter )
        return false;
	do {
        splitter->addWidget( manager_->mainWindow() );
        splitter->addWidget( new Core::OutputPanePlaceHolder( mode_.get() ) );
        splitter->setStretchFactor( 0, 10 );
        splitter->setStretchFactor( 1, 0 );
        splitter->setOrientation( Qt::Vertical );
	} while( 0 );

	do {
		QWidget* centralWidget = new QWidget;
		manager_->mainWindow()->setCentralWidget( centralWidget );
        
		std::vector< QWidget * > wnd;
		Core::MiniSplitter * splitter3 = new Core::MiniSplitter;
		if ( splitter3 ) {
			QTabWidget * pTab = new QTabWidget;
			splitter3->addWidget( pTab );
			SDFileView * view = new SDFileView;
			pTab->addTab( view, tr("SDFileView") ); // tab[0]
			view->show();
		}
		QBoxLayout * toolBarAddingLayout = new QVBoxLayout( centralWidget );
		toolBarAddingLayout->setMargin(0);
		toolBarAddingLayout->setSpacing(0);
		toolBarAddingLayout->addWidget( splitter3 );
    } while(0);

	manager_->setSimpleDockWidgetArrangement();
	mode_->setWidget( splitter );
    addObject( mode_.get() );

    return true;
}

void
ChemistryPlugin::extensionsInitialized()
{
    Core::ModeManager::instance()->activateMode( mode_->uniqueModeName() );
    manager_->OnInitialUpdate();
}

void
ChemistryPlugin::triggerAction()
{
    QMessageBox::information(Core::ICore::instance()->mainWindow(),
                             tr("Action triggered"),
                             tr("This is an action from Chemistry."));
}

Q_EXPORT_PLUGIN2(Chemistry, ChemistryPlugin)

