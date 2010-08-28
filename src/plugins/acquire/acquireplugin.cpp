//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////

#include "acquireplugin.h"
#include "constants.h"
#include "acquiremode.h"
#include "acquireuimanager.h"
#include "acquireactions.h"
#include <adwidgets/dataplotwidget.h>
#include <adwidgets/spectrumwidget.h>
#include <adwidgets/chromatogramwidget.h>
#include <adwidgets/axis.h>
//#include <acewrapper/orbmanager.h>
#include <adplugin/orbmanager.h>
#include <tao/Object.h>
#include <orbsvcs/CosNamingC.h>
#include <adcontroller/adcontroller.h>
#include <adinterface/controlserverC.h>
#include <adinterface/receiverC.h>
#include <acewrapper/constants.h>
#include <utils/fancymainwindow.h>

#include <coreplugin/icore.h>
#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>
#include <extensionsystem/pluginmanager.h>

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
#include <ace/Singleton.h>
#include <servant/servantplugin.h>
#include <qtwrapper/qstring.h>
#include <adinterface/eventlog_helper.h>

#include <adwidgets/titles.h>
#include <adwidgets/title.h>
#include <adwidgets/dataplot.h>
#include <adwidgets/traces.h>
#include <adwidgets/trace.h>
#include <adwidgets/colors.h>

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
	  adwidgets::ui::ChromatogramWidget * timePlot_;
	  adwidgets::ui::SpectrumWidget * spectrumPlot_;
      QIcon icon_;
	  void loadIcon() {
		  icon_.addFile( Constants::ICON_CONNECT );
		  icon_.addFile( Constants::ICON_CONNECT_SMALL );
	  }
    };

    template<class T> class marchal {
    public:
        static T get( const ACE_Message_Block * mb ) {
            TAO_InputCDR in( mb );
            T t;
            in >> t;
            return t;
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
							   , actionConnect_(0)
							   , actionRunStop_(0)
							   , action3_(0)
							   , action4_(0)
							   , action5_(0)
{
}

void
AcquirePlugin::initialize_actions()
{
  pImpl_->loadIcon();
  
  actionConnect_ = new QAction(QIcon(Constants::ICON_CONNECT), tr("Connect to control server..."), this);
  connect( actionConnect_, SIGNAL(triggered()), this, SLOT(actionConnect()) );
  
  actionRunStop_ = new QAction(QIcon(Constants::ICON_RUN_SMALL), tr("Run / stop control..."), this);
  connect( actionRunStop_, SIGNAL(triggered()), this, SLOT(actionRunStop()) );
  
  action3_ = new QAction(QIcon(Constants::ICON_INTERRUPT_SMALL), tr("Interrupt sequence..."), this);
  connect( action3_, SIGNAL(triggered()), this, SLOT(action3()) );
  
  action4_ = new QAction(QIcon(Constants::ICON_START_SMALL), tr("Start initial condition..."), this);
  connect( action4_, SIGNAL(triggered()), this, SLOT(action4()) );
  
  action5_ = new QAction(QIcon(Constants::ICON_STOP_SMALL), tr("Stop inlet..."), this);
  connect( action5_, SIGNAL(triggered()), this, SLOT(action5()) );
  
  //const AcquireManagerActions& actions = manager_->acquireManagerActions();
  QList<int> globalcontext;
  globalcontext << Core::Constants::C_GLOBAL_ID;
  Core::ActionManager *am = Core::ICore::instance()->actionManager();
  if ( am ) {
    Core::Command * cmd = 0;
    cmd = am->registerAction( actionConnect_, Constants::CONNECT, globalcontext );
    cmd = am->registerAction( actionRunStop_, Constants::INITIALRUN, globalcontext );
    cmd = am->registerAction( action3_, Constants::RUN, globalcontext );
    cmd = am->registerAction( action4_, Constants::STOP, globalcontext );
    cmd = am->registerAction( action5_, Constants::ACQUISITION, globalcontext );
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
		  toolBarLayout->addWidget(toolButton(am->command(Constants::CONNECT)->action()));
		  toolBarLayout->addWidget(toolButton(am->command(Constants::INITIALRUN)->action()));
		  toolBarLayout->addWidget(toolButton(am->command(Constants::RUN)->action()));
		  toolBarLayout->addWidget(toolButton(am->command(Constants::STOP)->action()));
		  toolBarLayout->addWidget(toolButton(am->command(Constants::ACQUISITION)->action()));
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
      rightPaneSplitter->addWidget( new adwidgets::TraceWidget );
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
        if ( pImpl_->timePlot_ = new adwidgets::ui::ChromatogramWidget ) {
			//adwidgets::ui::Axis axis = pImpl_->timePlot_->axisX();
			//axis.text( L"Time(min)" );
		}

		if ( pImpl_->spectrumPlot_ = new adwidgets::ui::SpectrumWidget ) {
			//adwidgets::ui::Axis axis = pImpl_->spectrumPlot_->axisX();
			//axis.text( L"m/z" );
			this->handle_message(0, 0);
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
    manager_->OnInitialUpdate();
}

void
AcquirePlugin::shutdown()
{
    manager_->OnFinalClose();
}

void
AcquirePlugin::actionConnect()
{
    int argc = 1;
    char * argv[1] = { "" };
    if ( adplugin::ORBManager::instance()->init( argc, argv ) >= 0 ) {
        // CosNaming::Name name = adcontroller::name();
        CosNaming::Name name = acewrapper::constants::adcontroller::manager::name();
        
        CORBA::Object_var obj = adplugin::ORBManager::instance()->getObject( name );
        if ( ! CORBA::is_nil( obj ) ) {
            ControlServer::Manager_var manager = ControlServer::Manager::_narrow( obj );
            if ( ! CORBA::is_nil( manager ) ) {
                session_ = manager->getSession( L"acquire" );
                if ( ! CORBA::is_nil( session_.in() ) ) {
                    receiver_i_.reset( new adplugin::QReceiver_i() );
                    session_->connect( receiver_i_.get()->_this(), L"acquire" );

                    int res;
                    res = connect( receiver_i_.get()
						           , SIGNAL( signal_message( unsigned long, unsigned long ) )
								   , this, SLOT( handle_message( unsigned long, unsigned long ) ) );
                    res = connect( receiver_i_.get(), SIGNAL( signal_log( QByteArray ) ), this, SLOT( handle_log( QByteArray ) ) );
                    res = connect( receiver_i_.get(), SIGNAL( signal_shutdown() ), this, SLOT( handle_shutdown() ) );
                    res = connect( receiver_i_.get(), SIGNAL( signal_debug_print( unsigned long, unsigned long, QString ) )
                                   , this, SLOT( handle_debug_print( unsigned long, unsigned long, QString ) ) );
					if ( session_->status() <= ControlServer::eConfigured )
						session_->initialize();
					/////////////////////////
                    /// qick test
					observer_ = session_->getObserver();
					if ( ! CORBA::is_nil( observer_.in() ) ) {
						SignalObserver::Observers_var siblings = observer_->getSiblings();
						size_t nsize = siblings->length();
						if ( nsize >= 1 ) {
							SignalObserver::Observer_var var = SignalObserver::Observer::_duplicate( siblings[0uL] );
                            tofCache_ = var;
						}
					}
                }
            }
        }
    }
}

void
AcquirePlugin::actionRunStop()
{
}

void
AcquirePlugin::handle_message( unsigned long /* Receiver::eINSTEVENT */ msg, unsigned long value )
{
    static int count;
	count;
	// quick debug hack
	if ( pImpl_->timePlot_ ) {
		adwidgets::ui::Dataplot * plot = pImpl_->timePlot_;
	}
	if ( pImpl_->spectrumPlot_ ) {
		adwidgets::ui::Dataplot * plot = pImpl_->spectrumPlot_;
		adwidgets::ui::Titles titles = plot->titles();
		size_t n = titles.count();
		adwidgets::ui::Title title = titles[0];
        title.text( L"Title..." );
        titles.visible( true );
		title.visible(true);

		adwidgets::ui::Traces traces = plot->traces();
		adwidgets::ui::Trace trace;
		if ( traces.size() < 1 ) {
			trace = traces.add();
			adwidgets::ui::Colors colors = plot->colors();
		} else {
			trace = traces[0];
		}

		const size_t nsize = 100; //100 * 1024;
		boost::scoped_array<double> pX( new double [ nsize ] );
		boost::scoped_array<double> pY( new double [ nsize ] );
		for ( int i = 0; i < nsize; ++i ) {
			pX[i] = count + i;
			pY[i] = count + i;
		}
		trace.colorIndex(2);
		trace.setXYDirect( nsize, pX.get(), pY.get() );
		trace.visible( true );
		traces.visible( true );
	}
	// manager_->handle_message( msg, value );
}

void
AcquirePlugin::handle_log( QByteArray qmsg )
{
    TAO_InputCDR cdr( qmsg.data(), qmsg.size() );
    ::EventLog::LogMessage msg;
    cdr >> msg;
    manager_->handle_eventLog( msg );
}

void
AcquirePlugin::handle_shutdown()
{
    manager_->handle_shutdown();
}

void
AcquirePlugin::handle_debug_print( unsigned long priority, unsigned long category, QString text )
{
    manager_->handle_debug_print( priority, category, text );
}

Q_EXPORT_PLUGIN( AcquirePlugin )
