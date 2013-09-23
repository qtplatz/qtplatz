/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#include "task.hpp"
#include "iproxy.hpp"
#include "oproxy.hpp"
#include "logging.hpp"
#include "observer_i.hpp"
#include "manager_i.hpp"
#include "sampleprocessor.hpp"
#include <adinterface/receiverC.h>
#include <adinterface/eventlogC.h>
#include <adinterface/samplebrokerC.h>
#include <iostream>
#include <sstream>
#include <adinterface/eventlog_helper.hpp>
#include <adportable/configuration.hpp>
#include <adportable/configloader.hpp>
#include <adportable/debug.hpp>
#include <adportable/timer.hpp>
#include <acewrapper/orbservant.hpp>
#include <xmlparser/pugixml.hpp>
#include <xmlparser/pugiwrapper.hpp>
#include <boost/bind.hpp>
#include <stdexcept>
#if defined _DEBUG
# include <iostream>
#endif

using namespace adcontroller;

iTask * iTask::instance_ = 0;
std::mutex iTask::mutex_;

namespace adcontroller {
    namespace internal {
	
        struct receiver_data {
            bool operator == ( const receiver_data& ) const;
            bool operator == ( const Receiver_ptr ) const;
            bool operator == ( const ControlServer::Session_ptr ) const;
            ControlServer::Session_var session_;
            Receiver_var receiver_;
            std::string token_;
            size_t failed_;
            receiver_data() : failed_( 0 ) {}
            receiver_data( const receiver_data& t )
                : session_(t.session_), receiver_(t.receiver_), token_( t.token_ ), failed_( t.failed_ ) {
            }
        };
	
    }
}

iTask *
iTask::instance()
{
    if ( instance_ == 0 ) {
        std::lock_guard< std::mutex > lock( mutex_ );
        if ( instance_ == 0 )
            instance_ = new iTask;
    }
    return instance_;
}

iTask::~iTask()
{
}

iTask::iTask() : status_current_( ControlServer::eNothing )
               , status_being_( ControlServer::eNothing )  
               , work_( io_service_ )
               , timer_( io_service_ )
               , interval_( 3000 ) // ms
{
}

void
iTask::reset_clock()
{
    using adcontroller::iTask;

    std::lock_guard< std::mutex > lock( mutex_ );
    for ( auto it: iproxies_ )
        it->reset_clock();
}

bool
iTask::open()
{
    timer_.cancel();
    initiate_timer();
    
    for ( size_t i = 0; i < 8; ++i ) 
        threads_.push_back( std::thread( boost::bind(&boost::asio::io_service::run, &io_service_ ) ) );
    return true;
}

void
iTask::close()
{
    io_service_.stop();
    for ( std::thread& t: threads_ )
        t.join();
}

bool
iTask::setConfiguration( const char * xml )
{
    status_current_ = status_being_ = ControlServer::eNotConfigured;

    pugi::xml_document dom;
    pugi::xml_parse_result result;
	if ( ( result = dom.load( xml ) ) )
        return setConfiguration( dom );
    return static_cast< bool >( result );
}

bool
iTask::setConfiguration( const pugi::xml_document& dom )
{
    status_current_ = status_being_ = ControlServer::eNotConfigured;

    pugi::xpath_node_set set = dom.select_nodes( "//Configuration[@name='InstrumentConfiguration']/Configuration" );
    if ( set.size() > 0 ) {
        for ( const pugi::xpath_node& node : set ) {
            adportable::Configuration child;

			for ( const auto& att: node.node().attributes() )
				child.attribute( att.name(), att.value() );

			child.xml( pugi::helper::to_string( node.node() ) );

            config_.append( child );

        }
        return true;
    }
    return false;
}

bool
iTask::configComplete()
{
    return true;
}

bool
iTask::initialize_configuration()
{
    using namespace adportable;

    if ( status_current_ >= ControlServer::eConfigured )
        return true;

	adportable::timer x;
	Logging(L"iTask::initialize_configuration...", ::EventLog::pri_DEBUG );

    SignalObserver::Observer_var masterObserver = getObserver();
    if ( CORBA::is_nil( masterObserver.in() ) ) {
        assert(0);
        throw std::runtime_error( "iTask::initialize_configuration - can't get master observer servant" );
    }

    int objid = 0;
    for ( Configuration& item: config_ ) {
        ++objid;
        // initialize instrument proxy
        std::shared_ptr<iProxy> pProxy( new iProxy( *this ) );
        if ( pProxy ) {
			adportable::timer timer;
            pProxy->objId( objid );
            if ( ! pProxy->initialConfiguration( item ) ) {
                Logging(L"iTask::initialize_configuration -- instrument initialization failed for \"%1%\""
                        , ::EventLog::pri_WARNING ) % item.name();
				continue; //return false;
            }
            std::lock_guard< std::mutex > lock( mutex_ );
            iproxies_.push_back( pProxy );
            Logging(L"iTask::initialize_configuration -- instrument \"%1%\" successfully initialized as objId %2% took %3% us"
                    , ::EventLog::pri_INFO )  % item.name() % objid % timer.elapsed();
        }

        // initialize observer proxy
        Instrument::Session_var iSession = pProxy->getSession();
        if ( ! CORBA::is_nil( iSession.in() ) ) {
            std::shared_ptr<oProxy> poProxy( new oProxy( *this ) );
            if ( poProxy ) {
				adportable::timer timer;

                poProxy->objId( objid );
                poProxy->setConfiguration( item );
                if ( poProxy->setInstrumentSession( iSession ) ) { // assign objid to source objects
                    size_t n = poProxy->populateObservers( objid );
                    Logging(L"iTask::initialize_configuration -- \"%1%\" has %2% signal observers %3% us"
						, ::EventLog::pri_INFO ) % item.name() % n % timer.elapsed();
                    objid += n;
                }
                std::lock_guard< std::mutex > lock( mutex_ );
                oproxies_.push_back( poProxy );

                // add source into the Cache (1st layer siblings)
                masterObserver->addSibling( poProxy->getObject() );
            }
        }
    }

    // fire connect
    using adcontroller::iProxy;
    using adcontroller::oProxy;
    std::for_each( iproxies_.begin(), iproxies_.end(), boost::bind( &iProxy::connect, _1, "adcontroller.iTask(i)" ) );
    std::for_each( oproxies_.begin(), oproxies_.end(), boost::bind( &oProxy::connect, _1, "adcontroller.iTask(o)" ) );

    status_current_ = status_being_ = ControlServer::eConfigured;  // relevant modules are able to access.
	Logging(L"iTask::initialize_configuration completed. %1% us", ::EventLog::pri_INFO ) % x.elapsed();
    return true;
}

bool
iTask::initialize()
{
    using adcontroller::iProxy;
    using adcontroller::oProxy;

    Logging(L"iTask::initialize...", ::EventLog::pri_INFO );
    if ( initialize_configuration() ) {
        std::lock_guard< std::mutex > lock( mutex_ );
        std::for_each( iproxies_.begin(), iproxies_.end(), boost::bind( &iProxy::initialize, _1 ) );
        std::for_each( oproxies_.begin(), oproxies_.end(), boost::bind( &oProxy::initialize, _1 ) );
        return true;
    }
    return false;
}

bool
iTask::connect( ControlServer::Session_ptr session, Receiver_ptr receiver, const char * token )
{
    internal::receiver_data data;
    data.session_ = ControlServer::Session::_duplicate( session );
    data.receiver_ = Receiver::_duplicate( receiver );
    data.token_ = token;
    
    std::lock_guard< std::mutex > lock( mutex_ );
    
    if ( std::find(receiver_set_.begin(), receiver_set_.end(), data ) != receiver_set_.end() )
        return false;

    receiver_set_.push_back( data );
    
    Logging( L"A pair of session %1%, Receiver %2% from \"%3%\" has success connected"
             , EventLog::pri_INFO ) % static_cast< void * >( session ) % static_cast<void *>( receiver ) % token;

    return true;
}

bool
iTask::disconnect( ControlServer::Session_ptr session, Receiver_ptr receiver )
{
    std::lock_guard< std::mutex > lock( mutex_ );

    do { // disconnecting proxies
        using adcontroller::iProxy;
        using adcontroller::oProxy;
        for ( auto& iproxy: iproxies_ )
            iproxy->disconnect();
        for ( auto& oproxy: oproxies_ )
            oproxy->disconnect();
    } while ( 0 );
    
    // receiver_vector_type::iterator it = std::remove( receiver_set_.begin(), receiver_set_.end(), data );
    auto it = std::remove_if( receiver_set_.begin(), receiver_set_.end(), [&](internal::receiver_data& t ){
            return t.receiver_->_is_equivalent( receiver ) && t.session_->_is_equivalent( session );
        });
    
    if ( it != receiver_set_.end() ) {
        receiver_set_.erase( it, receiver_set_.end() );
        return true;
    }
    return false;
}

::ControlServer::eStatus
iTask::getStatusCurrent()
{
    return status_current_;
}

::ControlServer::eStatus
iTask::getStatusBeing()
{
    return status_being_;
}

bool
iTask::observer_update_data( unsigned long parentId, unsigned long objid, long pos )
{
    // come from oProxy::OnUpdateData --> schedule invoke handle_observer_update_data
    io_service_.post( std::bind(&iTask::handle_observer_update_data, this, parentId, objid, pos ) );
    return true;
}

bool
iTask::observer_update_method( unsigned long parentId, unsigned long objid, long pos )
{
    io_service_.post( std::bind(&iTask::handle_observer_update_method, this, parentId, objid, pos ) );
    return true;
}

bool
iTask::observer_update_event( unsigned long parentId, unsigned long objid, long pos, unsigned long events )
{
    io_service_.post( std::bind(&iTask::handle_observer_update_events, this, parentId, objid, pos, events ) );
    return true;
}


///////////////////////////////////////////////////////////////

bool
internal::receiver_data::operator == ( const receiver_data& t ) const
{
    return receiver_->_is_equivalent( t.receiver_.in() )
        && session_->_is_equivalent( t.session_.in() );
}

bool
internal::receiver_data::operator == ( const Receiver_ptr t ) const
{
    return receiver_->_is_equivalent( t );
}

bool
internal::receiver_data::operator == ( const ControlServer::Session_ptr t ) const
{
    return session_->_is_equivalent( t );
}

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

void
iTask::handle_observer_update_data( unsigned long parentId, unsigned long objId, long pos )
{
    using SignalObserver::DataReadBuffer_var;
    
    if ( DataReadBuffer_var rp = pMasterObserver_->handle_data( parentId, objId, pos ) ) {

        std::lock_guard< std::mutex > lock( mutex_ );

        for ( auto q: queue_ ) {
            // handle_data( objId, pos, rp );
            q->strand().post( std::bind(&SampleProcessor::handle_data, q, objId, pos, rp ) ); // iTask::io_service
        }

    }
    pMasterObserver_->forward_observer_update_data( parentId, objId, pos );
}

void
iTask::handle_observer_update_method( unsigned long parentId, unsigned long objId, long pos )
{
    pMasterObserver_->forward_observer_update_method( parentId, objId, pos );
}

void
iTask::handle_observer_update_events( unsigned long parentId, unsigned long objId, long pos, unsigned long events )
{
    pMasterObserver_->forward_observer_update_events( parentId, objId, pos, events );
}


SignalObserver::Observer_ptr
iTask::getObserver()
{
    PortableServer::POA_var poa = adcontroller::manager_i::instance()->poa();
    if ( ! pMasterObserver_ ) {
        std::lock_guard< std::mutex > lock( mutex_ );
        if ( ! pMasterObserver_ )
            pMasterObserver_.reset( new observer_i() );
    }
    CORBA::Object_ptr obj = poa->servant_to_reference( pMasterObserver_.get() );
    try {
        return SignalObserver::Observer::_narrow( obj );
    } catch ( CORBA::Exception& ) {
    }
    return 0;
}

void
iTask::initiate_timer()
{
    timer_.expires_from_now( boost::posix_time::milliseconds( interval_ ) );
    timer_.async_wait( boost::bind( &iTask::handle_timeout, this, boost::asio::placeholders::error ) );
}

void
iTask::handle_timeout( const boost::system::error_code& )
{
    initiate_timer();
}

void
iTask::handle_echo( std::string s )
{
    for ( auto& d: receiver_set_ ) {
        try {
            d.receiver_->debug_print( 0, 0, s.c_str() );
        } catch ( CORBA::Exception& ) {
            d.failed_++;
            adportable::debug(__FILE__, __LINE__) << "iTask::handle_dispatch_command 'echo' got an exception";
        }
    }
}

void
iTask::handle_prepare_for_run( ControlMethod::Method m )
{
    SampleBroker::SampleSequenceLine s;
    
    std::lock_guard< std::mutex > lock( mutex_ );
    
    for ( auto& proxy: iproxies_ )
        proxy->prepare_for_run( s, m ); 

	status_current_ = status_being_ = ControlServer::eReadyForRun;
    io_service_.post( std::bind( &iTask::notify_message, this, Receiver::STATE_CHANGED, status_current_ ) );
}

void
iTask::handle_start_run()
{
    adportable::debug(__FILE__, __LINE__) << "######################### handle_start_run...";

    std::lock_guard< std::mutex > lock( mutex_ );
	
	if ( queue_.empty() ) {
		queue_.push_back( std::shared_ptr< SampleProcessor >( new SampleProcessor( io_service_ ) ) );
        queue_.back()->prepare_storage( pMasterObserver_->_this() );
    }

	status_current_ = ControlServer::ePreparingForRun;
	status_being_ = ControlServer::eReadyForRun;

    for ( auto& proxy: iproxies_ )
        proxy->startRun();

	status_current_ = status_being_ = ControlServer::eWaitingForContactClosure;
    io_service_.post( std::bind( &iTask::notify_message, this, Receiver::STATE_CHANGED, status_current_ ) );
}

void
iTask::handle_resume_run()
{
}

void
iTask::handle_stop_run()
{
    for ( auto& proxy: iproxies_ )
        proxy->stopRun();

    adportable::debug(__FILE__, __LINE__) << "######################### handle_stop_run...";

    std::lock_guard< std::mutex > lock( mutex_ );

    if ( !queue_.empty() ) {
        adportable::debug(__FILE__, __LINE__) << "handle_stop_run remove one sample-processor";
        queue_.pop_front();
    }

	status_current_ = status_being_ = ControlServer::eReadyForRun;
    io_service_.post( std::bind( &iTask::notify_message, this, Receiver::STATE_CHANGED, status_current_ ) );
}

void
iTask::handle_event_out( unsigned long value )
{
    std::lock_guard< std::mutex > lock( mutex_ );

    for ( auto& proxy: iproxies_ )
        proxy->eventOut( value );

	if ( value == ControlServer::event_InjectOut && status_current_ == ControlServer::eWaitingForContactClosure ) {
        status_current_ = status_being_ = ControlServer::eRunning;
        io_service_.post( std::bind( &iTask::notify_message, this, Receiver::STATE_CHANGED, status_current_ ) );     
    }
}

void
iTask::notify_message( unsigned long msgid, unsigned long value )
{
    std::lock_guard< std::mutex > lock( mutex_ );    

    for ( internal::receiver_data& d: this->receiver_set_ ) {
        try {
            adportable::debug(__FILE__, __LINE__) << "notify_message(" << msgid << ", " << value << ")";
            d.receiver_->message( Receiver::eINSTEVENT( msgid ), value );
        } catch ( CORBA::Exception& ex ) {
            d.failed_++;
            adportable::debug(__FILE__, __LINE__) << "exception: " << ex._name();
        }
    }
}

void
iTask::handle_message( std::wstring name, unsigned long msgid, unsigned long value )
{
    (void)name; // message source 
    std::lock_guard< std::mutex > lock( mutex_ );    

    for ( internal::receiver_data& d: this->receiver_set_ ) {
        try {
            d.receiver_->message( Receiver::eINSTEVENT( msgid ), value );
        } catch ( CORBA::Exception& ex ) {
            d.failed_++;
            adportable::debug(__FILE__, __LINE__) << "exception: " << ex._name();
        }
    }
}

void
iTask::handle_eventlog( EventLog::LogMessage log )
{
    std::lock_guard< std::mutex > lock( mutex_ );    
    
    for ( internal::receiver_data& d: receiver_set_ ) {
        try {
            d.receiver_->log( log );
        } catch ( CORBA::Exception& ) {
            d.failed_++;
        }
    }
}
