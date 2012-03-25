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

#include "chemspiderplugin.hpp"
#include "chemspiderconstants.hpp"
#include "chemspidermode.hpp"
#include "chemspidermanager.hpp"
#include "ui_chemspidermode.h"

#include <coreplugin/icore.h>
#include <coreplugin/icontext.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/modemanager.h>
#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/minisplitter.h>

#include <QtGui/QAction>
#include <QtGui/QMessageBox>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QTextEdit>

#include <QtCore/QtPlugin>

using namespace ChemSpider::Internal;

ChemSpiderPlugin::ChemSpiderPlugin()
{
    // Create your members
}

ChemSpiderPlugin::~ChemSpiderPlugin()
{
    // Unregister objects from the plugin manager's object pool
    // Delete members
	if ( mode_ )
		removeObject( mode_.get() );
}

bool
ChemSpiderPlugin::initialize(const QStringList &arguments, QString *errorString)
{
    Q_UNUSED(arguments)
    Q_UNUSED(errorString)

    Core::ActionManager *am = Core::ICore::instance()->actionManager();
    
    QAction *action = new QAction(tr("ChemSpider action"), this);
    QList<int> globalcontext;
	globalcontext << Core::Constants::C_GLOBAL_ID;
    Core::Command *cmd = am->registerAction(action, Constants::ACTION_ID, globalcontext ); // Core::Context(Core::Constants::C_GLOBAL));
    cmd->setDefaultKeySequence(QKeySequence(tr("Ctrl+Alt+Meta+A")));
    connect(action, SIGNAL(triggered()), this, SLOT(triggerAction()));
    
    Core::ActionContainer *menu = am->createMenu(Constants::MENU_ID);
    menu->menu()->setTitle(tr("ChemSpider"));
    menu->addAction(cmd);
    am->actionContainer(Core::Constants::M_TOOLS)->addMenu(menu);

	mode_.reset( new ChemSpiderMode( this ) );
	if ( ! mode_ )
		return false;
    manager_.reset( new ChemSpiderManager(0) );
	if ( manager_ ) {
		QWidget * centralWidget = new QWidget;
		if ( centralWidget ) {
			ChemSpider::Ui_ChemSpiderMode * ui = new ChemSpider::Ui_ChemSpiderMode; // ( centralWidget );
			ui->setupUi( centralWidget );
			manager_->mainWindow()->setCentralWidget( centralWidget );
		}
	} else
		return false;

	Core::MiniSplitter * splitter = new Core::MiniSplitter;
	if ( splitter ) {
		splitter->addWidget( manager_->mainWindow() );
		splitter->addWidget( new QTextEdit );
        splitter->setStretchFactor( 0, 10 );
		splitter->setStretchFactor( 1, 0 );
		splitter->setOrientation( Qt::Vertical );
	} else
		return false;

    mode_->setWidget( splitter );
	addObject( mode_.get() );

    return true;
}

void ChemSpiderPlugin::extensionsInitialized()
{
    // Retrieve objects from the plugin manager's object pool
    // "In the extensionsInitialized method, a plugin can be sure that all
    //  plugins that depend on it are completely initialized."
	// mode_->initPlugins();
	Core::ModeManager::instance()->activateMode( mode_->uniqueModeName() );
	manager_->OnInitialUpdate();
}

void
ChemSpiderPlugin::triggerAction()
{
    QMessageBox::information(Core::ICore::instance()->mainWindow(),
                             tr("Action triggered"),
                             tr("This is an action from ChemSpider."));
}

Q_EXPORT_PLUGIN2(ChemSpider, ChemSpiderPlugin)

