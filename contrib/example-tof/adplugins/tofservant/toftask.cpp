/**************************************************************************
** Copyright (C) 2013 MS-Cheminformatics LLC
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
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

#include "toftask.hpp"
#include "tofservant.hpp"
#include "tofmgr_i.hpp"
#include "logger.hpp"
#include "profileobserver_i.hpp"
#include "traceobserver_i.hpp"
#include "devicefacade.hpp"
#include "processwaveform.hpp"
#include <adinterface/signalobserverC.h>
#include <adportable/debug.hpp>
#include <boost/format.hpp>
#include <boost/bind.hpp>
#include <functional>

namespace tofservant {

    struct observer_events_data {
        ~observer_events_data() {
        }

        observer_events_data() : cb_( 0 ), freq_( SignalObserver::Realtime ), failed_( false ) {
        }

        observer_events_data( SignalObserver::ObserverEvents_ptr ptr
                              , const std::string& token
                              , SignalObserver::eUpdateFrequency freq = SignalObserver::Realtime )
            : cb_( SignalObserver::ObserverEvents::_duplicate( ptr ) )
            , freq_( freq )
            , token_( token )
            , failed_( false ) {
        }

        observer_events_data( const observer_events_data& t )
            : cb_( SignalObserver::ObserverEvents::_duplicate( t.cb_.in() ) )
            , token_( t.token_ )
            , freq_( t.freq_ )
            , failed_( t.failed_ ) {
            /**/
        }

        SignalObserver::ObserverEvents_var cb_;
        SignalObserver::eUpdateFrequency freq_;
        std::string token_;
        bool failed_;
    };
    
    struct receiver_data {
        ~receiver_data() { /**/ }
        receiver_data() : failed_( false ) { /**/ }
        receiver_data( Receiver_ptr receiver
                       , const std::string& token ) : receiver_( Receiver::_duplicate( receiver ) )
                                                    , token_( token ) {
        }
        receiver_data( const receiver_data& t ) : receiver_( Receiver::_duplicate( t.receiver_.in() ) )
                                                , token_( t.token_ )
                                                , failed_( t.failed_ ) {
        }
        Receiver_var receiver_;
        std::string token_;
        bool failed_;
    };

    // class worker {
    //     bool enable_;
    // public:
    //     worker() : enable_( true ) {}
    //     void run() {
    //         while( enable_ ) {
    //             Sleep( 1000 );
    //             std::cout << "worker: " << std::this_thread::get_id().hash() << " is running..." << std::endl;
    //             toftask::instance()->io_service().post( std::bind(&toftask::handle_post, toftask::instance()) );
    //         }
    //     }
    //     void stop() { enable_ = false; }
    // };

}
// worker __worker;

using namespace tofservant;

toftask * toftask::instance_ = 0;
std::mutex toftask::mutex_;

toftask::toftask() : work_( io_service_ )
                   , timer_( io_service_ )
                   , device_facade_( new DeviceFacade )
{
}

toftask::~toftask()
{
    task_close();
}

toftask *
toftask::instance()
{
    if ( instance_ == 0 ) {
        std::lock_guard< std::mutex > lock( mutex_ );
        if ( instance_ == 0 )
            instance_ = new toftask;
    }
    return instance_;
}

bool
toftask::setConfiguration( const char * )
{
    return true;
}

SignalObserver::Observer *
toftask::getObserver()
{
	PortableServer::POA_var poa = tofServantPlugin::instance()->poa();

    if ( ! pObserver_ ) {
        std::lock_guard< std::mutex > lock( mutex_ );

        if ( ! pObserver_ )
            pObserver_.reset( new profileObserver_i );

        // add Traces
        SignalObserver::Description desc;
        desc.trace_method = SignalObserver::eTRACE_TRACE;
        desc.spectrometer = SignalObserver::eMassSpectrometer;
        desc.axis_x_decimals = 3;
        desc.axis_y_decimals = 2;
        desc.trace_display_name = CORBA::wstring_dup( L"TIC" );
        desc.trace_id = CORBA::wstring_dup( L"MS.TIC" );
        desc.trace_method = SignalObserver::eTRACE_TRACE;
        
        std::shared_ptr< traceObserver_i > p( new traceObserver_i );
        p->setDescription( desc );
        pTraceObserverVec_.push_back( p );
        do {
            CORBA::Object_var obj = poa->servant_to_reference( p.get() );
            pObserver_->addSibling( SignalObserver::Observer::_narrow( obj ) );
        } while(0);
        
        // add mass chromatograms
        for ( int i = 0; i < 3; ++i ) {
            std::shared_ptr< traceObserver_i > p( new traceObserver_i );
            desc.trace_method = SignalObserver::eTRACE_TRACE;
            desc.spectrometer = SignalObserver::eMassSpectrometer;
            desc.trace_display_name 
                = CORBA::wstring_dup( ( boost::wformat( L"TOF Chromatogram.%1%" ) % (i + 1) ).str().c_str() );
            desc.trace_id 
                = CORBA::wstring_dup( (boost::wformat( L"MS.CHROMATOGRAM.%1%" ) % (i + 1) ).str().c_str() );
            p->setDescription( desc );
            pTraceObserverVec_.push_back( p );
            do {
                CORBA::Object_var obj = poa->servant_to_reference( p.get() );
                pObserver_->addSibling( SignalObserver::Observer::_narrow( obj ) );
            } while(0);
        }
        task_open();
    }

    CORBA::Object_var obj = poa->servant_to_reference( pObserver_.get() );
    return SignalObserver::Observer::_narrow( obj._retn() );
}

bool
toftask::getControlMethod( TOF::ControlMethod& m )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    m = method_;
    return true;
}

bool
toftask::connect( Receiver_ptr receiver, const std::string& token )
{
    if ( CORBA::is_nil( receiver ) )
        return false;

    std::lock_guard< std::mutex > lock( mutex_ );
    
    auto it = std::find_if( receiver_set_.begin(), receiver_set_.end(), [&](const receiver_data& d){
            return d.receiver_->_is_equivalent( receiver );
        });
    
    if ( it != receiver_set_.end() ) {
        Logger( L"Hi %1%, you already have a connection.", EventLog::pri_INFO ) % token;
        return false;
    }
    
    receiver_set_.push_back( receiver_data(receiver, token ) );

    return true;
}

bool
toftask::disconnect( Receiver_ptr receiver )
{
    if ( CORBA::is_nil( receiver ) )
        return false;

    auto it = std::remove_if( receiver_set_.begin(), receiver_set_.end(), [&](const receiver_data& d){
            return d.receiver_->_is_equivalent( receiver );
        });

    if ( it != receiver_set_.end() ) {
        std::string token = it->token_;

        receiver_set_.erase( it, receiver_set_.end() );
        adportable::debug(__FILE__, __LINE__) << "toftask::disconnect: " << token << " has removed, " 
                                              << receiver_set_.size() << " connections remain";
        return true;
    }

    return false;
}

bool
toftask::connect ( SignalObserver::ObserverEvents_ptr cb
                   , SignalObserver::eUpdateFrequency freq
                   , const std::string& token )
{
    if ( CORBA::is_nil( cb ) )
        return false;

    std::lock_guard< std::mutex > lock( mutex_ );
    
    auto it = std::find_if( observer_events_set_.begin()
                            , observer_events_set_.end()
                            , [&](const observer_events_data& d){
                                return d.cb_->_is_equivalent( cb );
        });
    
    if ( it != observer_events_set_.end() ) {
        Logger( L"Hi %1%, you already have a connection.", EventLog::pri_INFO ) % token;
        return false;
    }
    
    observer_events_set_.push_back( observer_events_data( cb, token, freq ) );

    return true;
}

bool
toftask::disconnect( SignalObserver::ObserverEvents_ptr cb )
{
    if ( CORBA::is_nil( cb ) )
        return false;

    auto it = std::remove_if( observer_events_set_.begin(), observer_events_set_.end(), [&](const observer_events_data& d){
            return d.cb_->_is_equivalent( cb );
        });

    if ( it != observer_events_set_.end() ) {
        std::string token = it->token_;

        observer_events_set_.erase( it, observer_events_set_.end() );
        adportable::debug(__FILE__, __LINE__) << "toftask::disconnect: " << token << " has removed, " 
                                              << observer_events_set_.size() << " connections remain";
        if ( observer_events_set_.empty() )
            task_close();

        return true;
    }

    return false;
}

void
toftask::observer_fire_on_update_data( unsigned long objId, long pos )
{
    std::lock_guard< std::mutex > lock( mutex_ );

    for ( auto& d: observer_events_set_ ) {
        if ( !d.failed_ ) {
            try { 
                d.cb_->OnUpdateData( objId, pos );
            } catch ( CORBA::Exception& ex ) {
                d.failed_ = true;
                adportable::debug(__FILE__, __LINE__) << ex._info().c_str();
            }
        }
    }
}

void
toftask::observer_fire_on_method_changed( unsigned long objId, long pos )
{
    std::lock_guard< std::mutex > lock( mutex_ );

    for ( auto& d: observer_events_set_ ) {
        if ( !d.failed_ ) {
            try { 
                d.cb_->OnMethodChanged( objId, pos );
            } catch ( CORBA::Exception& ex ) {
                d.failed_ = true;
                adportable::debug(__FILE__, __LINE__) << ex._info().c_str();
            }
        }
    }
}

void
toftask::observer_fire_on_event( unsigned long objId, unsigned long event, long pos )
{
    std::lock_guard< std::mutex > lock( mutex_ );

    for ( auto& d: observer_events_set_ ) {
        if ( !d.failed_ ) {
            try { 
                d.cb_->OnEvent( objId, event, pos );
            } catch ( CORBA::Exception& ex ) {
                d.failed_ = true;
                adportable::debug(__FILE__, __LINE__) << ex._info().c_str();
            }
        }
    }

}

void
toftask::session_fire_message( Receiver::eINSTEVENT msg, unsigned long value )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    
    for ( auto& d: receiver_set_ ) {
        if ( ! d.failed_ ) {
            try {
                d.receiver_->message( msg, value );
            } catch( CORBA::Exception& ) {
                d.failed_ = true;
            }
        }
    }
}

void
toftask::session_fire_log( long pri, const std::wstring& format, const std::vector< std::wstring >& args
                           , const std::wstring& msgId )
{
}

bool
toftask::task_open()
{
    //--
    // threads_.push_back( std::thread( &worker::run, &__worker ) );
    //--
    timer_.cancel();
    initiate_timer();

    threads_.push_back( std::thread( boost::bind(&boost::asio::io_service::run, &io_service_ ) ) );
    threads_.push_back( std::thread( boost::bind(&boost::asio::io_service::run, &io_service_ ) ) );

    device_facade_->initialize();

    return true;
}

void
toftask::task_close()
{
    // __worker.stop();
    device_facade_->terminate();
    io_service_.stop();
    for ( std::thread& t: threads_ )
        t.join();
}

void
toftask::initiate_timer()
{
    timer_.expires_from_now( boost::posix_time::milliseconds( interval_ ) );
    timer_.async_wait( boost::bind( &toftask::handle_timeout, this, boost::asio::placeholders::error ) );
}

void
toftask::handle_timeout( const boost::system::error_code& ec )
{
    initiate_timer();
    adportable::debug(__FILE__, __LINE__) << "***** toftask::handle_timeout in " << std::this_thread::get_id().hash();
}

void
toftask::handle_post()
{
    adportable::debug(__FILE__, __LINE__) << "***** toftask::handle_post in " << std::this_thread::get_id().hash();
}

void
toftask::handle_eventlog( EventLog::LogMessage msg )
{
    std::lock_guard< std::mutex > lock( mutex_ );

    for ( auto& d: receiver_set_ ) {
        if ( ! d.failed_ ) {
            try {
                d.receiver_->log( msg );
            } catch( CORBA::Exception& ) {
                d.failed_ = true;
            }
        }
    }
}

unsigned long
toftask::get_status()
{
	return 0;
}

bool
toftask::initialize()
{
    device_facade_->initialize();
    return true;
}

bool
toftask::handle_prepare_for_run( ControlMethod::Method m )
{
    adportable::debug(__FILE__, __LINE__) << "***** toftask::handle_prepare_for_run "
                                          << std::this_thread::get_id().hash();
    return true;
}

bool
toftask::handle_event_out( unsigned long )
{
	return true;
}

bool
toftask::handle_start_run()
{
	return true;
}

bool
toftask::handle_suspend_run()
{
	return true;
}

bool
toftask::handle_resume_run()
{
	return true;
}

bool
toftask::handle_stop_run()
{
	return true;
}

bool
toftask::setControlMethod( const TOF::ControlMethod& m, const char * hint )
{
    return device_facade_->setControlMethod( m, hint );
}

void
toftask::handle_profile_data( std::shared_ptr< TOFSignal::tofDATA > data )
{
    ProcessWaveform profile( data );
    profile.push_traces( pTraceObserverVec_ );
    profile.push_profile( pObserver_.get() );
}
