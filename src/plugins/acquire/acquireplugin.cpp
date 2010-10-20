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
#include <adplugin/adplugin.h>
#include <adplugin/orbmanager.h>
#include <adplugin/qreceiver_i.h>
#include <adplugin/qobserverevents_i.h>
#include <tao/Object.h>
# pragma warning(disable:4996)
# include <orbsvcs/CosNamingC.h>
# include <adcontroller/adcontroller.h>
# include <adinterface/controlserverC.h>
# include <adinterface/receiverC.h>
# include <adinterface/signalobserverC.h>
# pragma warning(default:4996)

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
#include <adcontrols/massspectrum.h>
#include <adcontrols/description.h>
#include <adportable/massspectrometer.h>
#include <adportable/array_wrapper.hpp>
#include <boost/format.hpp>
#include <adcontrols/centroidprocess.h>
#include <adcontrols/centroidmethod.h>
#include <algorithm>
#include <cmath>
#include <adportable/fft.h>

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

static bool reduceNoise( adcontrols::MassSpectrum& ms );

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

#if defined _DEBUG
  const adportable::MassSpectrometer& ms = adportable::MassSpectrometer::get( L"InfiTOF" );
  const adportable::MassSpectrometer::ScanLaw& scanLaw = ms.getScanLaw();
  double tof[ 100 ];
  double mass[ 100 ];
  int n = 0;
  for ( int i = 50; i < 500; i += 50 ) {
      tof[n] = scanLaw.getTime( i, 0.688 );
      mass[n] = scanLaw.getMass( tof[n], 0.688 );
      double d = mass[n] - i;
	  assert( fabs(d) < 10e-10 );
      n++;
  }
#endif

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

        toolBarLayout->addWidget( new QLabel( tr("  ") ), 10 );
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
        pImpl_->timePlot_ = new adwidgets::ui::ChromatogramWidget;
        pImpl_->spectrumPlot_ = new adwidgets::ui::SpectrumWidget;

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
		std::string name = acewrapper::constants::adcontroller::manager::_name();
        
        //CORBA::Object_var obj = adplugin::ORBManager::instance()->getObject( name );
        CORBA::Object_var obj = adplugin::ORBManager::instance()->string_to_object( adplugin::manager::iorBroker() );
        if ( ! CORBA::is_nil( obj ) ) {
            ControlServer::Manager_var manager;
            try { manager = ControlServer::Manager::_narrow( obj ); } catch ( CORBA::Exception& ) { /**/ }
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
                    /// connect observer event sink
					observer_ = session_->getObserver();
					if ( ! CORBA::is_nil( observer_.in() ) ) {
						SignalObserver::Observers_var siblings = observer_->getSiblings();
						size_t nsize = siblings->length();
						for ( size_t i = 0; i < nsize; ++i ) {
							SignalObserver::Observer_var var = SignalObserver::Observer::_duplicate( siblings[i] );
							boost::shared_ptr<adplugin::QObserverEvents_i> sink( new adplugin::QObserverEvents_i( var, L"acquire.ui" ) );
                            sinkVec_.push_back( sink );
							SignalObserver::Description_var pdesc = sink->ptr()->getDescription();
							res = connect( sink.get(), SIGNAL( signal_UpdateData( unsigned long, long ) ), this, SLOT( handle_update_data(unsigned long, long) ) );
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
AcquirePlugin::handle_update_data( unsigned long objId, long pos )
{
    ACE_UNUSED_ARG( objId );
    ACE_UNUSED_ARG( pos );

    SignalObserver::Observer_var tgt = observer_->findObserver( objId, true );
    if ( CORBA::is_nil( tgt.in() ) )
        return;

    SignalObserver::DataReadBuffer_var rb;
    if ( tgt->readData( pos, rb ) ) {
        // tgt->dataInterpreterClsid(); // <-- todo
		const adportable::MassSpectrometer& spectrometer = adportable::MassSpectrometer::get( L"InfiTOF" );
		const adportable::MassSpectrometer::ScanLaw& scanLaw = spectrometer.getScanLaw();
 
        std::wostringstream o;
        o << boost::wformat(L"Spectrum pos[%1%] EV:%2%") % rb->pos % rb->events;

        size_t delay = 0;
        size_t sampInterval = 500;
        SignalObserver::AveragerData *pavgr;
        if ( rb->method >>= pavgr ) {
            delay = pavgr->startDelay;
            sampInterval = pavgr->sampInterval;
            o << boost::wformat(L" delay:%1% nbrSamples: %2%") % pavgr->startDelay % pavgr->nbrSamples;
        }

        manager_->handle_debug_print( 0, 0, qtwrapper::qstring::copy(o.str()) );

        adcontrols::MassSpectrum ms;
        ms.addDescription( adcontrols::Description( L"acquire.title", o.str() ) );

        const size_t nsize = rb->array.length();
        ms.resize( nsize );
        boost::scoped_array<double> pX( new double [ nsize ] );
		boost::scoped_array<double> pY( new double [ nsize ] );
		for ( size_t i = 0; i < nsize; ++i ) {
			double tof = ( delay + double( sampInterval * i ) ) / 1000000; // ps -> us
			pX[i] = scanLaw.getMass( tof, 0.688 );
			pY[i] = rb->array[i];
		}
        ms.setMassArray( pX.get(), true ); // update acq range
        ms.setIntensityArray( pY.get() );
        // --
        adcontrols::CentroidMethod method;
        method.centroidAreaIntensity( false ); // take hight
		adcontrols::CentroidProcess peak_detector( method );
		peak_detector( ms );

        adcontrols::MassSpectrum ms2 = ms;
		do {
			unsigned int tic = ::GetTickCount();
			reduceNoise( ms2 );
			int time = GetTickCount() - tic;
			std::wostringstream o;
			o << L"fft " << time << L"ms for" << ms2.size() << L"pts";
			ms2.addDescription( adcontrols::Description( L"acquire.fft", o.str() ) );
		} while(0);

        adcontrols::MassSpectrum centroid;
		peak_detector.getCentroidSpectrum( centroid );

		// pImpl_->spectrumPlot_->setData( ms, centroid );
		//pImpl_->spectrumPlot_->setData( ms, ms2 );
		pImpl_->spectrumPlot_->setData( ms, ms2 );
   }
}

void
AcquirePlugin::handle_message( unsigned long /* Receiver::eINSTEVENT */ msg, unsigned long value )
{
    static int count;
	count;
    (void)value;
	(void)msg;
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


///////////////////

static bool
reduceNoise( adcontrols::MassSpectrum& ms )
{
    size_t totalSize = ms.size();
	(void)totalSize;
	size_t N = 32;
    while ( N < ms.size() )
		N *= 2;
	N /= 2;
    size_t NN = N * 2;

	const double * pMass = ms.getMassArray();
	adportable::array_wrapper<const double> pIntens( ms.getIntensityArray(), N );

	std::vector< std::complex<double> > power;
	std::vector< std::complex<double> > interferrogram;

	for ( size_t i = 0; i < N; ++i )
		power.push_back( std::complex<double>(pIntens[i]) );

	adportable::fft::fourier_transform( interferrogram, power, false );
	//adportable::fft::apodization( N/4, N/4, interferrogram );
	adportable::fft::apodization( N/2 - N/16, N / 16, interferrogram );
	adportable::fft::zero_filling( NN, interferrogram );
	adportable::fft::fourier_transform( power, interferrogram, true );

	std::vector<double> data;
	std::vector<double> mass;
	for ( int i = 0; i < int(power.size()); ++i ) {
		data.push_back( power[i].real() + 30 ); //* (NN / N) + 20 );
		//ms.setIntensity( i, power[i].real() * (NN / N) + 20 );
		// ms.setIntensity( i, power[i].real() + 10 );
	}

	for ( size_t i = 0; i < N; ++i ) {
		mass.push_back( pMass[i] );
        mass.push_back( pMass[i] + ( pMass[i + 1] - pMass[i] ) / 2 );
	}

	ms.resize( data.size() );
	ms.setIntensityArray( &data[0] );
	ms.setMassArray( &mass[0] );

	return true;
}