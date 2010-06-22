//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////

#include "acquireplugin.h"
#include "constants.h"
#include "acquiremode.h"
#include "acquireuimanager.h"
#include "acquireactions.h"
#include <adwidgets/dataplot.h>
#include <adwidgets/axis.h>

#include <utils/fancymainwindow.h>

#include <coreplugin/icore.h>
#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/icore.h>
#include <QtCore/qplugin.h>
#include <coreplugin/minisplitter.h>
#include <coreplugin/outputpane.h>
#include <coreplugin/navigationwidget.h>
#include <coreplugin/rightpane.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <utils/styledbar.h>
#include <QtGui/QHBoxLayout>
#include <QtGui/QBoxLayout>
#include <QtGui/QToolButton>
#include <QtGui/QLabel>
#include <QTableWidget>
#include <QTextEdit>
#include <QToolButton>

using namespace Acquire;
using namespace Acquire::internal;

namespace Acquire {
  namespace internal {

    class AcquireImpl {
    public:
      ~AcquireImpl() {
      }
	  AcquireImpl() : timePlot_(0)
		            , spectrumPlot_(0) {
	  }
      adil::ui::Dataplot * timePlot_;
      adil::ui::Dataplot * spectrumPlot_;
      QIcon icon_;
	  void loadIcon() {
		  icon_.addFile( Constants::ICON_CONNECT );
		  icon_.addFile( Constants::ICON_CONNECT_SMALL );
	  }
    };

  }
}

// static
QToolButton * 
AcquirePlugin::toolButton( QAction * action )
{
  QToolButton * button = new QToolButton;
  if ( button )
    button->setDefaultAction( action );
  return button;
}

AcquirePlugin::~AcquirePlugin()
{
  delete manager_;
  delete pImpl_;
}

AcquirePlugin::AcquirePlugin() : manager_(0)
                               , pImpl_( new AcquireImpl() )
							   , action1_(0)
							   , action2_(0)
							   , action3_(0)
							   , action4_(0)
							   , action5_(0)
{
}

void
AcquirePlugin::initialize_actions()
{
	pImpl_->loadIcon();

	action1_ = new QAction(QIcon(Constants::ICON_CONNECT), tr("Connect"), this);
	// action1_->setText( tr("Start and Debug External Application...") );
	// action1_->setIcon( pImpl_->icon_ );
	connect( action1_, SIGNAL(triggered()), this, SLOT(action1()) );

	action2_ = new QAction(this);
	action2_->setText( tr("Start and Debug External Application...") );
	connect( action2_, SIGNAL(triggered()), this, SLOT(action2()) );

	action3_ = new QAction(this);
	action3_->setText( tr("Start and Debug External Application...") );
	connect( action3_, SIGNAL(triggered()), this, SLOT(action3()) );

	action4_ = new QAction(this);
	action4_->setText( tr("Start and Debug External Application...") );
	connect( action4_, SIGNAL(triggered()), this, SLOT(action4()) );

	action5_ = new QAction(this);
	action5_->setText( tr("Start and Debug External Application...") );
	connect( action5_, SIGNAL(triggered()), this, SLOT(action5()) );

	//const AcquireManagerActions& actions = manager_->acquireManagerActions();
	QList<int> globalcontext;
	globalcontext << Core::Constants::C_GLOBAL_ID;
	Core::ActionManager *am = Core::ICore::instance()->actionManager();
	if ( am ) {
		Core::Command * cmd = 0;
		cmd = am->registerAction( action1_, Constants::INTERRUPT, globalcontext );
	}
}

bool
AcquirePlugin::initialize(const QStringList &arguments, QString *error_message)
{
  Q_UNUSED(arguments);
  Q_UNUSED(error_message);
  Core::ICore * core = Core::ICore::instance();

  QList<int> context;
  if ( core ) {
    Core::UniqueIDManager * uidm = core->uniqueIDManager();
    if ( uidm ) {
      context.append( uidm->uniqueIdentifier( QLatin1String("Acquire.MainView") ) );
      context.append( uidm->uniqueIdentifier( Core::Constants::C_NAVIGATION_PANE ) );
    }
  } else
    return false;

  AcquireMode * mode = new AcquireMode(this);
  if ( mode )
    mode->setContext( context );
  else
    return false;

  manager_ = new AcquireUIManager(0);
  if ( manager_ )
    manager_->init();

  initialize_actions();

  do {
    
    //              [mainWindow]
    // splitter> ---------------------
    //              [OutputPane]
  
    Core::MiniSplitter * splitter = new Core::MiniSplitter;
    if ( splitter ) {
      splitter->addWidget( manager_->mainWindow() );
      splitter->addWidget( new Core::OutputPanePlaceHolder( mode ) );
      //splitter->addWidget( new QTextEdit("mainWindow") );
      //splitter->addWidget( new QTextEdit("This is edit" ) );
      
      splitter->setStretchFactor( 0, 10 );
      splitter->setStretchFactor( 1, 0 );
      splitter->setOrientation( Qt::Vertical ); // horizontal splitter bar
    }

    //
    //         <splitter2>         [mainWindow]
    // [Navigation] | [splitter ------------------- ]
    //                             [OutputPane]

    Core::MiniSplitter * splitter2 = new Core::MiniSplitter;
    if ( splitter2 ) {
      splitter2->addWidget( new Core::NavigationWidgetPlaceHolder( mode ) );
      splitter2->addWidget( splitter );
      splitter2->setStretchFactor( 0, 0 );
      splitter2->setStretchFactor( 1, 1 );
    }
      
    Utils::StyledBar * toolBar = new Utils::StyledBar;
    if ( toolBar ) {
      toolBar->setProperty( "topBorder", true );
      QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
      toolBarLayout->setMargin(0);
      toolBarLayout->setSpacing(0);
      Core::ActionManager *am = core->actionManager();
      if ( am ) {
		  Core::Command * cmd(0);
		  if ( cmd = am->command(Constants::INTERRUPT) )
			  toolBarLayout->addWidget(toolButton( cmd->action() ));
        //toolBarLayout->addWidget(toolButton(am->command(Constants::NEXT)->action()));
        //toolBarLayout->addWidget(toolButton(am->command(Constants::STEP)->action()));
        //toolBarLayout->addWidget(toolButton(am->command(Constants::STEPOUT)->action()));
           toolBarLayout->addWidget( new QLabel( tr("A") ) );
           toolBarLayout->addWidget( new Utils::StyledSeparator );
           toolBarLayout->addWidget( new QLabel( tr("B") ) );
           toolBarLayout->addWidget( new Utils::StyledSeparator );
           toolBarLayout->addWidget( new QLabel( tr("C") ) );
      }
      toolBarLayout->addWidget( new Utils::StyledSeparator );
      toolBarLayout->addWidget( new QLabel( tr("Threads:") ) );
    }
    Utils::StyledBar * toolBar2 = new Utils::StyledBar;
    if ( toolBar2 ) {
      toolBar2->setProperty( "topBorder", true );
      QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar2 );
      toolBarLayout->setMargin(0);
      toolBarLayout->setSpacing(0);
      Core::ActionManager *am = core->actionManager();
      if ( am ) {
        toolBarLayout->addWidget( new QLabel( tr("AA") ) );
        toolBarLayout->addWidget( new Utils::StyledSeparator );
        toolBarLayout->addWidget( new QLabel( tr("BB") ) );
        toolBarLayout->addWidget( new Utils::StyledSeparator );
        toolBarLayout->addWidget( new QLabel( tr("CC") ) );
      }
      toolBarLayout->addWidget( new Utils::StyledSeparator );
      toolBarLayout->addWidget( new QLabel( tr("Threads:") ) );
    }

    /*
    //  [TraceWidget] | [RightPanePlaceHolder]
    Core::MiniSplitter * rightPaneSplitter = new Core::MiniSplitter;
    if ( rightPaneSplitter ) {
      rightPaneSplitter->addWidget( new adil::TraceWidget );
      //rightPaneHSplitter->addWidget( new Core::RightPanePlaceHolder( mode ) );
      rightPaneSplitter->addWidget( new QTextEdit( "RightPanePlaceHolder" ) );
      rightPaneSplitter->setStretchFactor( 0, 1 );
      rightPaneSplitter->setStretchFactor( 1, 0 );
    }
    */

    QWidget* centralWidget = new QWidget;
    manager_->mainWindow()->setCentralWidget( centralWidget );

    Core::MiniSplitter * splitter3 = new Core::MiniSplitter;
    if ( splitter3 ) {
		if ( pImpl_->timePlot_ = new adil::ui::Dataplot ) {
			adil::ui::Axis axis = pImpl_->timePlot_->axisX();
			axis.text( L"Time(min)" );
		}

		if ( pImpl_->spectrumPlot_ = new adil::ui::Dataplot ) {
			adil::ui::Axis axis = pImpl_->spectrumPlot_->axisX();
			axis.text( L"m/z" );
		}

		splitter3->addWidget( pImpl_->timePlot_ );
		splitter3->addWidget( pImpl_->spectrumPlot_ );
		splitter3->setOrientation( Qt::Vertical );
    }

    QBoxLayout * toolBarAddingLayout = new QVBoxLayout( centralWidget );
    toolBarAddingLayout->setMargin(0);
    toolBarAddingLayout->setSpacing(0);
    //toolBarAddingLayout->addWidget( rightPaneSplitter );
    toolBarAddingLayout->addWidget( toolBar );
    toolBarAddingLayout->addWidget( splitter3 );
    toolBarAddingLayout->addWidget( toolBar2 );

    mode->setWidget( splitter2 );

  } while(0);
  
  manager_->setSimpleDockWidgetArrangement();
  addAutoReleasedObject(mode);

  return true;
}

void
AcquirePlugin::extensionsInitialized()
{
}

Q_EXPORT_PLUGIN( AcquirePlugin )
