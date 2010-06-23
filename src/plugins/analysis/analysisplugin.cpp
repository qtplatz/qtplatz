//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "analysisplugin.h"
#include "analysismode.h"
#include "analysismanager.h"

#include <QtCore/qplugin.h>
#include <coreplugin/icore.h>
#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>
#include <QStringList>

#include <coreplugin/minisplitter.h>
#include <coreplugin/outputpane.h>
#include <coreplugin/navigationwidget.h>
#include <coreplugin/rightpane.h>
#include <coreplugin/actionmanager/actionmanager.h>

#include <utils/styledbar.h>
#include <utils/fancymainwindow.h>

#include <QtGui/QHBoxLayout>
#include <QtGui/QBoxLayout>
#include <QtGui/QToolButton>
#include <QtGui/QLabel>
#include <QTableWidget>
#include <QTextEdit>
#include <QToolButton>


using namespace Analysis;
using namespace Analysis::internal;

AnalysisPlugin::~AnalysisPlugin()
{
}

AnalysisPlugin::AnalysisPlugin()
{
}

bool
AnalysisPlugin::initialize(const QStringList& arguments, QString* error_message)
{
  Q_UNUSED( arguments );
  Q_UNUSED( error_message );

  Core::ICore * core = Core::ICore::instance();
  
  QList<int> context;
  if ( core ) {
    Core::UniqueIDManager * uidm = core->uniqueIDManager();
    if ( uidm ) {
      context.append( uidm->uniqueIdentifier( QLatin1String("Analysis.MainView") ) );
      context.append( uidm->uniqueIdentifier( Core::Constants::C_NAVIGATION_PANE ) );
    }
  } else
    return false;

  AnalysisMode * mode = new AnalysisMode(this);
  if ( mode )
    mode->setContext( context );
  else
    return false;

  manager_.reset( new AnalysisManager(0) );
  if ( manager_ )
    manager_->init();

  // initialize_actions();

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
	/*
	  toolBarLayout->addWidget(toolButton(am->command(Constants::CONNECT)->action()));
	  toolBarLayout->addWidget(toolButton(am->command(Constants::INITIALRUN)->action()));
	  toolBarLayout->addWidget(toolButton(am->command(Constants::RUN)->action()));
	  toolBarLayout->addWidget(toolButton(am->command(Constants::STOP)->action()));
	  toolBarLayout->addWidget(toolButton(am->command(Constants::ACQUISITION)->action()));
	*/
      }
      toolBarLayout->addWidget( new Utils::StyledSeparator );
      toolBarLayout->addWidget( new QLabel( tr("Sequence:") ) );
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
      splitter3->addWidget( new QTextEdit );
      /*
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
      */
    }

    QBoxLayout * toolBarAddingLayout = new QVBoxLayout( centralWidget );
    toolBarAddingLayout->setMargin(0);
    toolBarAddingLayout->setSpacing(0);
    toolBarAddingLayout->addWidget( toolBar );
    toolBarAddingLayout->addWidget( splitter3 );
    toolBarAddingLayout->addWidget( toolBar2 );

    mode->setWidget( splitter2 );

  } while(0);
  
  //manager_->setSimpleDockWidgetArrangement();
  addAutoReleasedObject(mode);


  return true;
}


void
AnalysisPlugin::extensionsInitialized()
{
}

Q_EXPORT_PLUGIN( AnalysisPlugin )
