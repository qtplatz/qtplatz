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
#include "massspecform.hpp"
#include <adportable/configuration.hpp>
#include <adportable/debug.hpp>
#include <adplugin/adplugin.hpp>
#include <qtwrapper/qstring.hpp>
#include <coreplugin/icore.h>
#include <coreplugin/icontext.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/modemanager.h>
#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/minisplitter.h>
#include <coreplugin/outputpane.h>

#include <QtCore>
#if QT_VERSION >= 0x050100
# include <QtWidgets/QAction>
# include <QtWidgets/QMessageBox>
# include <QtWidgets/QMainWindow>
# include <QtWidgets/QMenu>
# include <QtWidgets/QTextEdit>
# include <QtWidgets/QBoxLayout>
# include <QtWidgets/QStackedWidget>
#else
# include <QtGui/QAction>
# include <QtGui/QMessageBox>
# include <QtGui/QMainWindow>
# include <QtGui/QMenu>
# include <QtGui/QTextEdit>
# include <QtGui/QBoxLayout>
# include <QtGui/QStackedWidget>
#endif

#include <QtWebKit/QWebView>
#include <QtCore/QtPlugin>
#include <QDir>
#include <boost/filesystem.hpp>

using namespace ChemSpider::Internal;

ChemSpiderPlugin::ChemSpiderPlugin()
{
}

ChemSpiderPlugin::~ChemSpiderPlugin()
{
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
    Core::Command *cmd = am->registerAction(action, Constants::ACTION_ID, globalcontext );
    cmd->setDefaultKeySequence(QKeySequence(tr("Ctrl+Alt+Meta+A")));
    connect(action, SIGNAL(triggered()), this, SLOT(triggerAction()));
    
    Core::ActionContainer *menu = am->createMenu(Constants::MENU_ID);
    menu->menu()->setTitle(tr("ChemSpider"));
    menu->addAction(cmd);
    am->actionContainer(Core::Constants::M_TOOLS)->addMenu(menu);

    //-------------------------------------------------------------------------------------------
    pConfig_.reset( new adportable::Configuration() );

	boost::filesystem::path path( qtwrapper::wstring::copy( QCoreApplication::instance()->applicationDirPath() ) );
	std::wstring apppath = path.branch_path().wstring(); // .c_str();
	std::wstring cfile = adplugin::orbLoader::config_fullpath( apppath, L"/ScienceLiaison/ChemSpider.config.xml" );

	if ( ! adplugin::manager::instance()->loadConfig( *pConfig_, cfile, L"/ChemSpiderConfiguration/Configuration" ) ) {
		adportable::debug( __FILE__, __LINE__ ) << "loadConfig" << cfile << "failed";
	}
    //------------------------------------------------

	mode_.reset( new ChemSpiderMode( this ) );
	if ( ! mode_ )
		return false;

    manager_.reset( new ChemSpiderManager(0) );
	if ( manager_ )
		manager_->init( *pConfig_, apppath );
	else
		return false;
	//------------
	Core::MiniSplitter * splitter = new Core::MiniSplitter;
	if ( splitter ) {
		splitter->addWidget( manager_->mainWindow() );
		splitter->addWidget( new Core::OutputPanePlaceHolder( mode_.get() ) );
        splitter->setStretchFactor( 0, 10 );
		splitter->setStretchFactor( 1, 0 );
		splitter->setOrientation( Qt::Vertical );
	} else
		return false;

	//----------
	do {
		QWidget* centralWidget = new QWidget;
		manager_->mainWindow()->setCentralWidget( centralWidget );

		std::vector< QWidget * > wnd;
		Core::MiniSplitter * splitter3 = new Core::MiniSplitter;
		if ( splitter3 ) {
			QTabWidget * pTab = new QTabWidget;
			splitter3->addWidget( pTab );

			//--- (1)
#if ! defined __linux__
            QWebView * view = new QWebView;
			pTab->addTab( view, QIcon(":/chemspider/image/logo_cs7.png"), "http://www.chemspider.com/" );
			view->load( QUrl( "http://www.chemspider.com/" ) );
			view->show();
#endif
			//--- (2)
            MassSpecForm * form = new MassSpecForm;
			//wnd.push_back( new QFrame );
			pTab->addTab( form, QIcon(":/chemspider/image/logo_cs7.png"), "MassSpec Search" );
		}
		QBoxLayout * toolBarAddingLayout = new QVBoxLayout( centralWidget );
		toolBarAddingLayout->setMargin(0);
		toolBarAddingLayout->setSpacing(0);
		//toolBarAddingLayout->addWidget( toolBar );
		toolBarAddingLayout->addWidget( splitter3 );
		//toolBarAddingLayout->addWidget( toolBar2 );
	} while(0);

    manager_->setSimpleDockWidgetArrangement();

	mode_->setWidget( splitter );
	addObject( mode_.get() );

	return true;
}

void
ChemSpiderPlugin::extensionsInitialized()
{
    // Core::ModeManager::instance()->activateMode( mode_->uniqueModeName() );
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

