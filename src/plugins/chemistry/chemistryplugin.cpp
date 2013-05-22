/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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
#include "chemistrymainwindow.hpp"
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
#if QT_VERSION >= 0x050000
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QStackedWidget>
#else
#include <QtGui/QBoxLayout>
#include <QtGui/QStackedWidget>
#endif

#include <qtextedit.h>
#include <qwidget.h>

using namespace chemistry;

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
	mode_.reset( new ChemistryMode( this ) );
	if ( ! mode_ )
		return false;

    ChemEditorFactory * editorFactory(0);
    do {
       Core::MimeDatabase * mdb = Core::ICore::instance()->mimeDatabase();
       if ( ! mdb )
		   return false;
       if ( ! mdb->addMimeTypes( ":/chemistry/mimetype.xml", errorString ) )
		   adportable::debug( __FILE__, __LINE__ ) << "addMimeTypes: " << errorString;

       QStringList mtypes;
       mtypes << "application/sdf"
		      << "application/mdl";
	   if ( ! ( editorFactory = new ChemEditorFactory( this, mtypes ) ) )
		   return false;
	   addAutoReleasedObject( editorFactory );
	} while( 0 );

	//<-------
	do {
		mainWindow_.reset( new ChemistryMainWindow() );
		if ( ! mainWindow_ )
			return false;
	} while( 0 );

	Core::ModeManager::instance()->activateMode( mode_->uniqueModeName() );
	mainWindow_->activateLayout();
	mainWindow_->createActions();
	QWidget * widget = mainWindow_->createContents( mode_.get() );
	mode_->setWidget( widget );
    addObject( mode_.get() );

    return true;
}

void
ChemistryPlugin::extensionsInitialized()
{
	Core::ModeManager::instance()->activateMode( mode_->uniqueModeName() );
	mainWindow_->OnInitialUpdate();
}

void
ChemistryPlugin::triggerAction()
{
    QMessageBox::information(Core::ICore::instance()->mainWindow(),
                             tr("Action triggered"),
                             tr("This is an action from Chemistry."));
}

Q_EXPORT_PLUGIN2(Chemistry, ChemistryPlugin)

