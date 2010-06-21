//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////

#include "acquireplugin.h"
#include "constants.h"
#include "acquiremode.h"
#include "acquireuimanager.h"
#include "acquireactions.h"
#include <utils/fancymainwindow.h>

#include <libadwidgets/tracewidget.h>
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
}

AcquirePlugin::AcquirePlugin() : manager_(0)
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
  action1_ = new QAction(this);
  action1_->setText( tr("Start and Debug External Application...") );
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

  /**
  const AcquireManagerActions& actions = manager_->acquireManagerActions();
  QList<int> globalcontext;
  globalcontext << Core::Constants::C_GLOBAL_ID;

  Core::ActionManager *am = Core::ICore::instance()->actionManager();
  if ( am ) {
    Core::Command * cmd = 0;
    cmd = am->registerAction( actions.stopAction, Constants::INTERRUPT, globalcontext );
  }
  **/
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

  initialize_actions();

  AcquireMode * mode = new AcquireMode(this);
  if ( mode )
    mode->setContext( context );
  else
    return false;

  manager_ = new AcquireUIManager(0);
  if ( manager_ )
    manager_->init();

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
        //toolBarLayout->addWidget(toolButton(am->command(Constants::INTERRUPT)->action()));
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
      
    QBoxLayout * toolBarAddingLayout = new QVBoxLayout( centralWidget );
    toolBarAddingLayout->setMargin(0);
    toolBarAddingLayout->setSpacing(0);
    //toolBarAddingLayout->addWidget( rightPaneSplitter );
    toolBarAddingLayout->addWidget( new adil::TraceWidget );
    toolBarAddingLayout->addWidget( toolBar );
    
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
