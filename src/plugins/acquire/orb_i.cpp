// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include "orb_i.hpp"
#include "acquireplugin.hpp"
#include "brokerevent_i.hpp"
#include "constants.hpp"
#include "document.hpp"
#include "mastercontroller.hpp"
#include "mainwindow.hpp"
#if HAVE_CORBA
#include "orbconnection.hpp"
#include "qbroker.hpp"
#include "receiver_i.hpp"
#endif

#include <acewrapper/constants.hpp>
#include <acewrapper/ifconfig.hpp>
#include <adextension/isnapshothandler.hpp>

#if HAVE_CORBA
# include <adinterface/brokerC.h>
# include <adinterface/controlserverC.h>
# include <adinterface/receiverC.h>
# include <adinterface/signalobserverC.h>
# include <adinterface/observerevents_i.hpp>
# include <adinterface/eventlog_helper.hpp>
# include <adinterface/controlmethodhelper.hpp>
# include <adorbmgr/orbmgr.hpp>
# include <tao/Object.h>
#endif

#include <adcontrols/controlmethod.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/descriptions.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/mscalibrateresult.hpp>
#include <adcontrols/msreference.hpp>
#include <adcontrols/msreferences.hpp>
#include <adcontrols/mscalibration.hpp>
#include <adcontrols/msassignedmass.hpp>
#include <adcontrols/datainterpreter.hpp>
#include <adcontrols/centroidprocess.hpp>
#include <adcontrols/centroidmethod.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/samplerun.hpp>
#include <adcontrols/trace.hpp>
#include <adcontrols/traceaccessor.hpp>
#include <adcontrols/timeutil.hpp>
#include <adextension/imonitorfactory.hpp>
#include <adextension/icontroller.hpp>
#include <adicontroller/instrument.hpp>
#include <adicontroller/receiver.hpp>
#include <adicontroller/signalobserver.hpp>
#include <adportable/array_wrapper.hpp>
#include <adportable/configuration.hpp>
#include <adportable/configloader.hpp>
#include <adportable/date_string.hpp>
#include <adportable/profile.hpp>
#include <adportable/binary_serializer.hpp>
#include <adportable/debug.hpp>
#include <adplugin_manager/loader.hpp>
#include <adplugin_manager/manager.hpp>
#include <adplugin/plugin.hpp>
#include <adplugin/plugin_ptr.hpp>
#include <adplugin/orbbroker.hpp>

#include <adportable/debug_core.hpp>
#include <adlog/logging_handler.hpp>
#include <adlog/logger.hpp>

#include <adportable/date_string.hpp>
#include <adportable/fft.hpp>
#include <adportable/debug.hpp>
#include <adplot/chromatogramwidget.hpp>
#include <adplot/spectrumwidget.hpp>

#include <qtwrapper/application.hpp>
#include <qtwrapper/qstring.hpp>
#include <servant/servantplugin.hpp>
#include <utils/fancymainwindow.h>

#include <coreplugin/icore.h>
#include <coreplugin/id.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/minisplitter.h>
#include <coreplugin/modemanager.h>
#include <coreplugin/navigationwidget.h>
#include <coreplugin/outputpane.h>
#include <coreplugin/rightpane.h>
#include <extensionsystem/pluginmanager.h>
#include <utils/styledbar.h>

#include <QAction>
#include <QComboBox>
#include <QtCore/qplugin.h>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QBoxLayout>
#include <QToolButton>
#include <QLabel>
#include <QLineEdit>
#include <QTableWidget>
#include <QTextEdit>
#include <QToolButton>
#include <QMessageBox>
#include <qdebug.h>

#include <boost/exception/all.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/bind.hpp>
#include <boost/exception/all.hpp>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <functional>
#include <future>
#include <map>

namespace acquire {

    class orb_i::task {
    public:
        boost::asio::io_service io_service_;
        boost::asio::io_service::work work_;
        boost::asio::io_service::strand strand_;
        std::vector< adportable::asio::thread > threads_;
        std::mutex mutex_;

        task() : work_( io_service_ )
               , strand_( io_service_ ) {
        }

        void terminate() {
            io_service_.stop();
            for ( auto& t : threads_ )
                t.join();
        }
    };

    class orb_i::impl {
    public:
        Broker::Session_var brokerSession_;
        std::unique_ptr< brokerevent_i > brokerEvent_;
        static orb_i * instance_;
        std::map< unsigned long, std::shared_ptr< adcontrols::MassSpectrum > > rdmap_;
        std::deque< std::shared_ptr< adcontrols::MassSpectrum > > fifo_ms_;
        std::map< unsigned long, std::shared_ptr< adcontrols::TraceAccessor > > trace_accessors_;
        std::map< unsigned long, std::shared_ptr< adcontrols::MSCalibrateResult > > calibResults_;
        std::map< int, std::shared_ptr< adcontrols::Trace > > traces_;
        std::mutex mutex_;

        void initialize_broker_session() {
            brokerEvent_.reset( new brokerevent_i );
            brokerEvent_->assign_message( [=]( const std::string& text ){
                    handle_message( text );
                });
            brokerEvent_->assign_portfolio_created( [=]( const std::wstring& file ){
                    handle_portfolio_created( file );
                });
            brokerEvent_->assign_folium_added(
                [=]( const std::wstring& token
                     , const std::wstring& path
                     , const std::wstring& folderId ){
                    handle_folium_added( token, path, folderId );
                });
            brokerSession_->connect( "user", "pass", "acquire", brokerEvent_->_this() );
        }

        void terminate_broker_session() {
            // disconnect broker session
            if ( !CORBA::is_nil( brokerSession_ ) && brokerEvent_ ) {
                brokerSession_->disconnect( brokerEvent_->_this() );
            }
        }

        void handle_message( const std::string& msg ) {
            auto vec = ExtensionSystem::PluginManager::instance()->getObjects< adextension::iSnapshotHandler >();
            for ( auto handler: vec )
                handler->message( QString( msg.c_str() ) );
        }

        void handle_portfolio_created( const std::wstring& token ) {
            auto vec = ExtensionSystem::PluginManager::instance()->getObjects< adextension::iSnapshotHandler >();
            for ( auto handler: vec )
                handler->portfolio_created( qtwrapper::qstring( token ) );
        }
            
        void handle_folium_added( const std::wstring& token, const std::wstring& path, const std::wstring& id ) {
            auto vec = ExtensionSystem::PluginManager::instance()->getObjects< adextension::iSnapshotHandler >();
            for ( auto handler: vec )
                handler->folium_added( qtwrapper::qstring( token )
                                       , qtwrapper::qstring( path ), qtwrapper::qstring( id ) );
        }

        void populate( SignalObserver::Observer_var& observer )   {
            SignalObserver::Description_var topLevelDesc = observer->getDescription();
                
            std::string topLevelName = topLevelDesc->trace_display_name.in();
			//traceBox_->addItem( QString::fromStdString( topLevelName ) );
            //todo: set name on MainWindow's traceBox_
            
            SignalObserver::Observers_var children = observer->getSiblings();
            for ( CORBA::ULong i = 0; i < children->length(); ++i ) {
                SignalObserver::Description_var secondLevelDesc = children[i]->getDescription();
                CORBA::String_var secondLevelName = children[i]->getDescription()->trace_display_name.in();
				//traceBox_->addItem( QString( "   %1" ).arg( secondLevelName.in() ) );
            }
        }

        bool readMassSpectra( const SignalObserver::DataReadBuffer& rb
                              , const adcontrols::MassSpectrometer& spectrometer
                              , const adcontrols::DataInterpreter& dataInterpreter
                              , unsigned long objid );
        bool readTrace( const SignalObserver::Description& desc
                        , const SignalObserver::DataReadBuffer& rb
                        , const adcontrols::DataInterpreter& dataInterpreter
                        , unsigned long objid );
    };

    orb_i * orb_i::impl::instance_( 0 );
    
}

using namespace acquire;

orb_i::orb_i() : task_( new task() )
               , impl_( new impl() )
{
    impl::instance_ = this;
}

orb_i::~orb_i()
{
}

void
orb_i::actionConnect()
{
    if ( CORBA::is_nil( session_.in() ) ) {
        
        Broker::Manager_var broker = OrbConnection::instance()->brokerManager();

        if ( ! CORBA::is_nil( broker ) ) {

            using namespace acewrapper::constants;
            CORBA::Object_var obj = broker->find_object( adcontroller::manager::_name() );
            ::ControlServer::Manager_var manager = ::ControlServer::Manager::_narrow( obj );

            if ( !CORBA::is_nil( manager ) ) {
                session_ = manager->getSession( "acquire" );
                if ( !CORBA::is_nil( session_.in() ) ) {
                    
                    receiver_i_.reset( new receiver_i );

                    receiver_i_->assign_message( [this] ( ::Receiver::eINSTEVENT code, uint32_t value ){
                            emit onReceiverMessage( static_cast<unsigned long>(code), value );                            
                        });
                    
                    receiver_i_->assign_log( [this] ( const ::EventLog::LogMessage& log ){ handle_receiver_log( log ); } );
                    
                    receiver_i_->assign_shutdown( [this](){ handle_receiver_shutdown(); } );
                    
                    receiver_i_->assign_debug_print( [this]( int32_t pri, int32_t cat, std::string text ){ handle_receiver_debug_print( pri, cat, text ); } );

                    connect( this, &orb_i::onReceiverMessage, this, &orb_i::handle_controller_message );
                        
                    if ( session_->connect( receiver_i_->_this(), "acquire" ) ) {
                        MainWindow::instance()->handleInstState( adicontroller::Instrument::eStandBy );
                    }
                        
                    if ( session_->status() <= ControlServer::eConfigured )
                        session_->initialize();

                    // Master signal observer
                    observer_ = session_->getObserver();
                    if ( !CORBA::is_nil( observer_.in() ) ) {
                        if ( !sink_ ) {
                            sink_.reset( new adinterface::ObserverEvents_i );
                            
                            sink_->assignConfigChanged( [=] ( uint32_t oid, SignalObserver::eConfigStatus st ){
                                    handle_observer_config_changed( oid, st );
                                } );
                            
                            sink_->assignUpdateData( [=] ( uint32_t oid, int32_t pos ){
                                    handle_observer_update_data( oid, pos );
                                } );
                            
                            sink_->assignMethodChanged( [=] ( uint32_t oid, int32_t pos ){
                                    handle_observer_method_changed( oid, pos );
                                } );
                            sink_->assignEvent( [=] ( uint32_t oid, int32_t pos, int32_t ev ){
                                    handle_observer_event( oid, pos, ev );
                                } );
                            
                            observer_->connect( sink_->_this(), SignalObserver::Frequent, "acquireplugin" );

						}
                        
                        // connect to 1st layer siblings ( := top shadow(cache) observer for each instrument )
                        SignalObserver::Observers_var siblings = observer_->getSiblings();
                        size_t nsize = siblings->length();

                        for ( CORBA::ULong i = 0; i < nsize; ++i ) {
                            SignalObserver::Observer_var var = SignalObserver::Observer::_duplicate( siblings[ i ] );
                            impl_->populate( var );
                        }
                    }
                }
                document::instance()->actionInitRun();
            }
        }
    }

    if ( ! CORBA::is_nil( session_ ) ) {
        MainWindow::instance()->handleInstState( adicontroller::Instrument::eStandBy );        
        document::instance()->fsmStop();
    }
}

void
orb_i::actionDisconnect()
{
    if ( ! CORBA::is_nil( session_ ) ) {

        // disconnect from observer
        observer_ = session_->getObserver();
        if ( ! CORBA::is_nil( observer_.in() ) ) {
            observer_->disconnect( sink_->_this() );
			adorbmgr::orbmgr::deactivate( sink_->_this() );
        }

        // disconnect instrument control session
        session_->disconnect( receiver_i_.get()->_this() );
        adorbmgr::orbmgr::deactivate( receiver_i_->_this() );

        document::instance()->fsmStop(); // FSM
    }
}

void
orb_i::actionInitRun()
{
    if ( ! CORBA::is_nil( session_ ) ) {

        document::instance()->fsmActPrepareForRun(); // FSM
        
        std::wostringstream os;
        adcontrols::SampleRun::xml_archive( os, *document::instance()->sampleRun() );
        
        ControlMethod::Method m;
        adinterface::ControlMethodHelper::copy( m, *document::instance()->controlMethod() );

        session_->prepare_for_run( m, adportable::utf::to_utf8( os.str() ).c_str() );

        ADTRACE() << "adcontroller status: " << session_->status();
    }
}

void
orb_i::actionRun()
{
    actionInitRun();
    if ( ! CORBA::is_nil( session_ ) ) {
        session_->start_run();
        document::instance()->fsmActRun(); // FSM
    }
}

void
orb_i::actionStop()
{
    if ( ! CORBA::is_nil( session_ ) ) {
        session_->stop_run();
        document::instance()->fsmActStop(); // FSM
    }
}

void
orb_i::actionInject()
{
    if ( ! CORBA::is_nil( session_ ) ) {
		session_->event_out( ControlServer::event_InjectOut ); // simulate inject out to modules
    }
}

void
orb_i::actionSnapshot()
{
    if ( CORBA::is_nil( observer_ ) )
        return;
    SignalObserver::Observers_var siblings = observer_->getSiblings();
    for ( CORBA::ULong  i = 0; i < siblings->length(); ++i ) {
        SignalObserver::Description_var desc = siblings[i]->getDescription();
        if ( desc->trace_method == SignalObserver::eTRACE_SPECTRA ) {
            CORBA::ULongLong first, second;
            siblings[i]->uptime_range( first, second );
            double m1 = double(second) / 60.0e6;
            double m0 = double(second - 1000) / 60.0e6;  // 1s before
//			pThis_->selectRange( m0, m1, 0, 0 );
        }
    }
}

bool
orb_i::readCalibrations( observer_type& obs )
{
    CORBA::String_var dataClass;
    SignalObserver::Observer_ptr tgt = std::get<0>( obs ).in();
    auto spectrometer = std::get<4>( obs );

    SignalObserver::octet_array_var data;

    bool success = false;
    CORBA::ULong idx = 0;
    while ( tgt->readCalibration( idx++, data, dataClass ) ) {
        auto wdataClass = adportable::utf::to_wstring( dataClass.in() );
        if ( std::wcscmp( wdataClass.c_str(), adcontrols::MSCalibrateResult::dataClass() ) == 0 ) {
            adcontrols::MSCalibrateResult result;
            if ( adportable::binary::deserialize<>()(result, reinterpret_cast<const char *>(data->get_buffer()), data->length()) )
                success = true;
            if ( spectrometer )
                spectrometer->setCalibration( idx - 1, result );
        }
    }
    return success;
}

void
orb_i::handle_update_data( unsigned long objId, long pos )
{
    ACE_UNUSED_ARG( pos );

    try {
        std::lock_guard< std::mutex > lock( task_->mutex_ );
        
        if ( observerMap_.find( objId ) == observerMap_.end() ) {
            SignalObserver::Observer_var tgt = observer_->findObserver( objId, true );
            if ( CORBA::is_nil( tgt.in() ) )
                return;

            CORBA::String_var name = tgt->dataInterpreterClsid();
            if ( auto spectrometer = adcontrols::MassSpectrometer::create( name.in() ))  {

                SignalObserver::Description_var desc = tgt->getDescription();
                observerMap_[ objId ] = std::make_tuple( tgt, desc, adportable::utf::to_wstring( name.in() ), false, spectrometer );
                npos_map_[ objId ] = pos;
            } else {
                ADTRACE() << "receive data from unavilable spectrometer: " << name.in();
                return;
            }
        }
    } catch ( ... ) {
        ADDEBUG() << boost::current_exception_diagnostic_information();
    }
    try {
        long& npos = npos_map_[ objId ];

        if ( pos < npos )
            return;

        auto it = observerMap_.find( objId );
        SignalObserver::Observer_ptr tgt = std::get<0>( it->second ).in();
        SignalObserver::Description_var& desc = std::get<1>( it->second );
        auto spectrometer = std::get<4>( it->second );

        const adcontrols::DataInterpreter& dataInterpreter = spectrometer->getDataInterpreter();

        if ( desc->trace_method == SignalObserver::eTRACE_SPECTRA ) {

            // ADDEBUG() << "handle_updae_data( " << objId << ", " << pos << ") npos=" << npos;
            
            if ( !std::get<3>( it->second ) )
                std::get<3>( it->second ) = readCalibrations( it->second );

            try {
                SignalObserver::DataReadBuffer_var rb;
                while ( tgt->readData( npos, rb ) && npos <= pos ) {
                    ADDEBUG() << "\treadData( " << npos << " ) " << rb->pos;
                    ++npos;
                    impl_->readMassSpectra( rb, *spectrometer, dataInterpreter, objId );
                }
                emit onUpdateUIData( objId, pos );
            } catch ( CORBA::Exception& ex ) {
                ADTRACE() << "handle_update_data got an corba exception: " << ex._info().c_str();
            } catch ( ... ) {
                ADTRACE() << boost::current_exception_diagnostic_information();
            }

        } else if ( desc->trace_method == SignalObserver::eTRACE_TRACE ) {
            try {
                SignalObserver::DataReadBuffer_var rb;
                while ( tgt->readData( npos, rb ) ) {
                    npos = rb->pos + rb->ndata;
					impl_->readTrace( desc, rb, dataInterpreter, objId );
                    emit onUpdateUIData( objId, pos );
                    return;
                }
            } catch ( CORBA::Exception& ex ) {
                ADTRACE() << "handle_update_data got an corba exception: " << ex._info().c_str();
            }
        }
    } catch ( ... ) {
        ADDEBUG() << boost::current_exception_diagnostic_information();
    }
}

void
orb_i::handle_controller_message( unsigned long /* Receiver::eINSTEVENT */ msg, unsigned long value )
{
    try {
        using namespace ControlServer;

        if ( msg == Receiver::STATE_CHANGED ) {

            eStatus status = eStatus( value );
            MainWindow::instance()->handleInstState( status );
            
            if ( status == eWaitingForContactClosure ) {

                // pThis_->actionInject_->setEnabled( true );
                // pThis_->actionStop_->setEnabled( true );
                // pThis_->actionRun_->setEnabled( false );

                CORBA::String_var xml = session_->running_sample();
                document::instance()->notify_ready_for_run( xml );

            } else if ( status == ePreparingForRun ) {

                //pThis_->actionStop_->setEnabled( false );

            } else if ( status == eReadyForRun ) {
                // try {
                //     if ( pThis_ && pThis_->actionStop_ )
                //         pThis_->actionStop_->setEnabled( false );
                //     if ( pThis_ && pThis_->actionRun_ )
                //         pThis_->actionRun_->setEnabled( true );
                // } catch ( ... ) {
                //     QMessageBox::warning( MainWindow::instance()
                //                           , "AcquirePlugin"
                //                           , QString::fromStdString( boost::current_exception_diagnostic_information() ) );
                // }

            } else if ( status == eRunning ) {

                // pThis_->actionStop_->setEnabled( true );
                // pThis_->actionInject_->setEnabled( false );
                // pThis_->actionRun_->setEnabled( false );

            }
        }
    } catch ( ... ) {
        ADDEBUG() << boost::current_exception_diagnostic_information();
        assert( 0 );
    }
}

void
orb_i::handle_observer_config_changed( uint32_t objid, SignalObserver::eConfigStatus st )
{
    emit onObserverConfigChanged( objid, long( st ) );
}

void
orb_i::handle_observer_update_data( uint32_t objid, int32_t pos )
{
    task_->io_service_.post( task_->strand_.wrap( std::bind(&orb_i::handle_update_data, this, objid, pos ) ) );    
}

void
orb_i::handle_observer_method_changed( uint32_t objid, int32_t pos )
{
    emit onObserverMethodChanged( objid, pos );
}

void
orb_i::handle_observer_event( uint32_t objid, int32_t pos, int32_t events )
{
    emit onObserverEvent( objid, pos, events );
}

void
orb_i::handle_receiver_log( const ::EventLog::LogMessage& log )
{
    QString msg;
    try {
        msg = QString::fromStdString( adportable::date_string::utc_to_localtime_string( log.tv.sec, log.tv.usec ) + "\t" );
        msg += QString::fromStdWString( adinterface::EventLog::LogMessageHelper::toString( log ) );
        MainWindow::instance()->eventLog( msg ); // will emit signal
    } catch ( ... ) {
        msg += QString::fromStdString( boost::current_exception_diagnostic_information() );
        MainWindow::instance()->eventLog( msg );
    }
}

void
orb_i::handle_receiver_shutdown()
{
    // handle_shutdown
}

void
orb_i::handle_receiver_debug_print( int32_t, int32_t, std::string text )
{
    QString qtext( text.c_str() );
    MainWindow::instance()->eventLog( qtext ); // will emit
}

void
orb_i::initialize()
{
    Broker::Manager_var mgr = OrbConnection::instance()->brokerManager();

    if ( CORBA::is_nil( mgr ) ) {
        return;
    }

    impl_->brokerSession_ = mgr->getSession( "acquire" );
    if ( CORBA::is_nil( impl_->brokerSession_ ) ) {
        return;
    }

    impl_->initialize_broker_session();

    connect( this, &orb_i::onUpdateUIData, this, &orb_i::handle_update_ui_data );

    task_->threads_.push_back( adportable::asio::thread( [this](){ task_->io_service_.run(); } ) );

}

void
orb_i::shutdown()
{
    impl_->terminate_broker_session();
    
    ADTRACE() << "orb_i::shutdown_broker() ...";

	//OrbConnection::instance()->shutdown();

    task_->terminate();

    ADTRACE() << "orb_i::shutdown_broker() completed.";
}

orb_i *
orb_i::instance()
{
    return impl::instance_;
}

bool
orb_i::impl::readMassSpectra( const SignalObserver::DataReadBuffer& rb
                              , const adcontrols::MassSpectrometer& spectrometer
                              , const adcontrols::DataInterpreter& dataInterpreter
                              , unsigned long objid )
{
    if ( ! rdmap_[ objid ] )
        rdmap_[ objid ].reset( new adcontrols::MassSpectrum );

    adcontrols::MassSpectrum& ms = *rdmap_[ objid ];

    size_t idData = 0;
    adcontrols::translate_state state = 
        dataInterpreter.translate( ms
                                   , reinterpret_cast< const char *>(rb.xdata.get_buffer()), rb.xdata.length()
                                   , reinterpret_cast< const char *>(rb.xmeta.get_buffer()), rb.xmeta.length()
                                   , spectrometer, idData++, 0 );
    if ( state == adcontrols::translate_complete ) {
        std::lock_guard< std::mutex > lock( mutex_ );
        fifo_ms_.push_back( rdmap_[ objid ] );
        rdmap_[ objid ].reset( new adcontrols::MassSpectrum );
    } 
    return state != adcontrols::translate_error;
}

bool
orb_i::impl::readTrace( const SignalObserver::Description& desc
                         , const SignalObserver::DataReadBuffer& rb
                         , const adcontrols::DataInterpreter& dataInterpreter
                          , unsigned long objid )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    if ( trace_accessors_.find( objid ) == trace_accessors_.end() )
        trace_accessors_[ objid ] = std::shared_ptr< adcontrols::TraceAccessor >( new adcontrols::TraceAccessor );

    adcontrols::TraceAccessor& accessor = *trace_accessors_[ objid ];
    if ( dataInterpreter.translate( accessor
									, reinterpret_cast< const char *>( rb.xdata.get_buffer() ), rb.xdata.length()
									, reinterpret_cast< const char * >( rb.xmeta.get_buffer() ), rb.xmeta.length()
                                    , rb.events ) == adcontrols::translate_complete ) {
        return true;
    }
    return false;
}

void
orb_i::handle_update_ui_data( unsigned long objId, long pos )
{
    std::shared_ptr< adcontrols::MassSpectrum > ms;
    do {
        std::lock_guard< std::mutex > lock( task_->mutex_ );
        if ( ! impl_->fifo_ms_.empty() )
            ms = impl_->fifo_ms_.back();
        impl_->fifo_ms_.clear();
    } while(0);
    
    if ( ms ) {

        document::instance()->setData( deprecated_observer, ms );

    }
        
    do {
        std::lock_guard< std::mutex > lock( task_->mutex_ );

        if ( impl_->trace_accessors_.find( objId ) == impl_->trace_accessors_.end() )
            return;

        adcontrols::TraceAccessor& accessor = *impl_->trace_accessors_[ objId ];
        for ( int fcn = 0; fcn < static_cast<int>(accessor.nfcn()); ++fcn ) {

            document::instance()->setData( deprecated_observer, accessor, fcn );        

        }

        accessor.clear();

    } while ( 0 );

}

void
orb_i::handle_config_changed( unsigned long objid, long pos )
{
    (void)objid;
    (void)pos;
}

void
orb_i::handle_method_changed( unsigned long objid, long pos )
{
    (void)objid;
    (void)pos;
}

void
orb_i::handle_event( unsigned long objid, long pos, long flags )
{
    (void)objid;
    (void)pos;
}
