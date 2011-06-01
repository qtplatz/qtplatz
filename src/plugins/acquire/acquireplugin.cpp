// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

#include "acquireplugin.hpp"
#include "constants.hpp"
#include "acquiremode.hpp"
#include "acquireuimanager.hpp"
#include "acquireactions.hpp"
#include <adwplot/chromatogramwidget.hpp>
#include <adwplot/spectrumwidget.hpp>
#include <adplugin/adplugin.hpp>
#include <adplugin/orbmanager.hpp>
#include <adplugin/qreceiver_i.hpp>
#include <adplugin/qobserverevents_i.hpp>
#include <tao/Object.h>
#include <ace/Singleton.h>
#include <adcontroller/adcontroller.hpp>

#if defined _MSC_VER
# pragma warning(disable:4996)
#endif
# include <adinterface/brokerC.h>
# include <adinterface/controlserverC.h>
# include <adinterface/receiverC.h>
# include <adinterface/signalobserverC.h>
#if defined _MSC_VER
# pragma warning(default:4996)
#endif
#include <acewrapper/constants.hpp>
#include <acewrapper/brokerhelper.hpp>
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
#include <coreplugin/modemanager.h>
#include <utils/styledbar.h>
#include <QtGui/QHBoxLayout>
#include <QtGui/QBoxLayout>
#include <QtGui/QToolButton>
#include <QtGui/QLabel>
#include <QTableWidget>
#include <QTextEdit>
#include <QToolButton>
#include <QMessageBox>
#include <qdebug.h>

#include <servant/servantplugin.hpp>
#include <qtwrapper/qstring.hpp>
#include <adinterface/eventlog_helper.hpp>

#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/datainterpreter.hpp>
#include <adportable/array_wrapper.hpp>
#include <boost/format.hpp>
#include <adcontrols/centroidprocess.hpp>
#include <adcontrols/centroidmethod.hpp>
#include <adcontrols/trace.hpp>
#include <adcontrols/traceaccessor.hpp>
#include <algorithm>
#include <cmath>
#include <map>
#include <adportable/fft.hpp>

#include <fstream>

using namespace Acquire;
using namespace Acquire::internal;

namespace Acquire {

    namespace internal {

        class AcquireImpl {
        public:
            ~AcquireImpl() {
            }
            AcquireImpl() : timePlot_(0), spectrumPlot_(0) {
            }

            Broker::Session_var brokerSession_;

            std::map< std::wstring, adcontrols::Trace > traces_;
            adwplot::ChromatogramWidget * timePlot_;
            adwplot::SpectrumWidget * spectrumPlot_;
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
                               , traceBox_(0) 
{
}

void
AcquirePlugin::initialize_actions()
{
    pImpl_->loadIcon();

    QIcon connIcon = QIcon( Constants::ICON_CONNECT_SMALL );
    connIcon.addFile( Constants::ICON_CONNECT );
  
    actionConnect_ = new QAction( connIcon, tr("Connect to control server..."), this);
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
        do {
            Core::ICore::instance()->modeManager()->addAction( cmd, 90 );
        } while(0);

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
                toolBarLayout->addWidget( new Utils::StyledSeparator );
                toolBarLayout->addWidget( new QLabel( tr("Traces:") ) );
                traceBox_ = new QComboBox;
                traceBox_->addItem( "-----------------------------" );
                connect( traceBox_, SIGNAL( currentIndexChanged(int) ), this, SLOT( handle_monitor_selected(int) ) );
                connect( traceBox_, SIGNAL( activated(int) ), this, SLOT( handle_monitor_activated(int) ) );
                toolBarLayout->addWidget( traceBox_ );
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
            pImpl_->timePlot_ = new adwplot::ChromatogramWidget;
            pImpl_->spectrumPlot_ = new adwplot::SpectrumWidget;

            splitter3->addWidget( pImpl_->timePlot_ );
            splitter3->addWidget( pImpl_->spectrumPlot_ );
            splitter3->setOrientation( Qt::Vertical );

            connect( pImpl_->timePlot_, SIGNAL( signalRButtonClick( double, double ) ), this, SLOT( handleRButtonClick( double, double ) ) );
            connect( pImpl_->timePlot_, SIGNAL( signalRButtonRange( double, double, double, double ) ), this, SLOT( handleRButtonRange( double, double, double, double ) ) );
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
    std::string ior = adplugin::manager::iorBroker();
	CORBA::ORB_var orb = adplugin::ORBManager::instance()->orb();
    Broker::Manager_var mgr = acewrapper::brokerhelper::getManager( orb, ior );
    if ( ! CORBA::is_nil( mgr ) )
        pImpl_->brokerSession_ = mgr->getSession( L"acquire" );

    do {
       
    } while(0);

    manager_->OnInitialUpdate();
}

void
AcquirePlugin::shutdown()
{
    actionDisconnect();
    manager_->OnFinalClose();
}

void
AcquirePlugin::actionConnect()
{
    if ( CORBA::is_nil( session_.in() ) ) {

        CORBA::Object_var obj
            = acewrapper::brokerhelper::name_to_object( adplugin::ORBManager::instance()->orb()
                                                      , acewrapper::constants::adcontroller::manager::_name()
                                                      , adplugin::manager::iorBroker() );

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

                    observer_ = session_->getObserver();
                    if ( ! CORBA::is_nil( observer_.in() ) ) {

                        // connect only to 1st layer siblings ( := top shadow(cache) observer for each instrument )
                        SignalObserver::Observers_var siblings = observer_->getSiblings();
                        size_t nsize = siblings->length();

                        for ( size_t i = 0; i < nsize; ++i ) {
                            SignalObserver::Observer_var var = SignalObserver::Observer::_duplicate( siblings[i] );
                            boost::shared_ptr<adplugin::QObserverEvents_i> sink( new adplugin::QObserverEvents_i( var, L"acquire.ui" ) );
                            sinkVec_.push_back( sink );
                            populate( var );
                            res = connect( sink.get(), SIGNAL( signal_UpdateData( unsigned long, long ) )
                                , this, SLOT( handle_update_data(unsigned long, long) ) );

                        }
                    }
                }
            }
        }
    }
}

void
AcquirePlugin::populate( SignalObserver::Observer_var& observer )
{
    SignalObserver::Description_var topLevelDesc = observer->getDescription();

    std::wstring topLevelName = topLevelDesc->trace_display_name.in();
    traceBox_->addItem( qtwrapper::qstring( topLevelName ) );

    SignalObserver::Observers_var children = observer->getSiblings();
    for ( size_t i = 0; i < children->length(); ++i ) {
        SignalObserver::Description_var secondLevelDesc = children[i]->getDescription();
        CORBA::WString_var secondLevelName = children[i]->getDescription()->trace_display_name.in();
        traceBox_->addItem( qtwrapper::qstring( L"   " + std::wstring( secondLevelName ) ) );
    }
}

void
AcquirePlugin::actionDisconnect()
{
    if ( ! CORBA::is_nil( session_ ) ) {

        observer_ = session_->getObserver();
        if ( ! CORBA::is_nil( observer_.in() ) ) {
            SignalObserver::Observers_var siblings = observer_->getSiblings();
            for ( size_t i = 0; i < sinkVec_.size(); ++i ) {
                disconnect( sinkVec_[i].get(), SIGNAL( signal_UpdateData( unsigned long, long ) ), this, SLOT( handle_update_data(unsigned long, long) ) );
                sinkVec_[i]->OnClose();
            }
        }
        session_->disconnect( receiver_i_.get()->_this() );
        adplugin::ORBManager::instance()->deactivate( receiver_i_->_this() );
    }
}
    
void
AcquirePlugin::actionRunStop()
{
}

void
AcquirePlugin::readMassSpectra( const SignalObserver::DataReadBuffer& rb
                               , const adcontrols::MassSpectrometer& spectrometer
                               , const adcontrols::DataInterpreter& dataInterpreter )
{
    adcontrols::MassSpectrum ms;
    size_t idData = 0;
    while ( dataInterpreter.translate( ms, rb, spectrometer, idData++ ) ) {
#ifdef CENTROID
        adcontrols::CentroidMethod method;
        method.centroidAreaIntensity( false ); // take hight
        adcontrols::CentroidProcess peak_detector( method );
        peak_detector( ms );
        adcontrols::MassSpectrum centroid;
        peak_detector.getCentroidSpectrum( centroid );
        pImpl_->spectrumPlot_->setData( ms, centroid );
#else
        pImpl_->spectrumPlot_->setData( ms, 0 );
#endif

#  ifdef FFT
        adcontrols::MassSpectrum ms2 = ms;
        do {
            unsigned int tic = ::GetTickCount();
            reduceNoise( ms2 );
            int time = GetTickCount() - tic;
            std::wostringstream o;
            o << L"fft " << time << L"ms for" << ms2.size() << L"pts";
            ms2.addDescription( adcontrols::Description( L"acquire.fft", o.str() ) );
        } while(0);
        pImpl_->spectrumPlot_->setData( ms, ms2 );
#  endif
    } 
}

void
AcquirePlugin::readTrace( const SignalObserver::Description& desc
                         , const SignalObserver::DataReadBuffer& rb
                         , const adcontrols::DataInterpreter& dataInterpreter )
{
    std::wstring traceId = static_cast<const CORBA::WChar *>( desc.trace_id );

    adcontrols::TraceAccessor accessor;
    if ( dataInterpreter.translate( accessor, rb ) ) {
        adcontrols::Trace& data = pImpl_->traces_[ std::wstring( desc.trace_id ) ];
        data += accessor;
        if ( data.size() >= 2 )        
            pImpl_->timePlot_->setData( data );
    }
}

void
AcquirePlugin::handle_update_data( unsigned long objId, long pos )
{
    ACE_UNUSED_ARG( pos );

    if ( observerMap_.find( objId ) == observerMap_.end() ) {
        SignalObserver::Observer_var tgt = observer_->findObserver( objId, true );
        if ( CORBA::is_nil( tgt.in() ) )
            return;
        observerMap_[ objId ] = tgt;
    }

    SignalObserver::Observer_ptr tgt = observerMap_[ objId ].in();

    SignalObserver::Description_var desc = tgt->getDescription();
    CORBA::WString_var clsid = tgt->dataInterpreterClsid();
    CORBA::WString_var name = tgt->dataInterpreterClsid();
    SignalObserver::DataReadBuffer_var rb;

    if ( tgt->readData( pos, rb ) ) {
        try {
            const adcontrols::MassSpectrometer& spectrometer = adcontrols::MassSpectrometer::get( name.in() ); // L"InfiTOF"
            const adcontrols::DataInterpreter& dataInterpreter = spectrometer.getDataInterpreter();
            if ( desc->trace_method == SignalObserver::eTRACE_SPECTRA 
                && desc->spectrometer == SignalObserver::eMassSpectrometer ) {
                    readMassSpectra( rb, spectrometer, dataInterpreter );
            } else if ( desc->trace_method == SignalObserver::eTRACE_TRACE ) {
                readTrace( *desc, rb, dataInterpreter );
            }
        } catch ( std::exception& ex ) {
            QMessageBox::critical( 0, "acquireplugin::handle_update_data", ex.what() );
            throw ex;
        }
    }
}

void
AcquirePlugin::handle_message( unsigned long /* Receiver::eINSTEVENT */ msg, unsigned long value )
{
    static int count;
    (void)count;
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

void
AcquirePlugin::handle_monitor_selected(int)
{
}

void
AcquirePlugin::handle_monitor_activated(int)
{
}

void
AcquirePlugin::handleRButtonClick( double x, double y )
{
    handleRButtonRange( x, x, y, y );
}

void
AcquirePlugin::handleRButtonRange( double x1, double x2, double y1, double y2 )
{
    (void)x1; (void)x2; (void)y1; (void)y2;

    SignalObserver::Observers_var siblings = observer_->getSiblings();
    size_t nsize = siblings->length();

    for ( size_t i = 0; i < nsize; ++i ) {
        SignalObserver::Description_var desc = siblings[i]->getDescription();

        if ( desc->trace_method == SignalObserver::eTRACE_SPECTRA && desc->spectrometer == SignalObserver::eMassSpectrometer ) {

            SignalObserver::Observer_var tgt = SignalObserver::Observer::_duplicate( siblings[i] );

            if ( pImpl_ && ! CORBA::is_nil( pImpl_->brokerSession_ ) ) {
                try {
                    pImpl_->brokerSession_->addSpectrum( tgt, x1, x2 );
                } catch ( std::exception& ex ) {
                    QMessageBox::critical( 0, "acquireplugin::handleRButtonRange", ex.what() );
                }
            }
        }
    }
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
