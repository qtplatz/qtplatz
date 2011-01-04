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

#include "TOFTask.h"
#include "tofsession_i.h"
#include "marshal.hpp"
#include "constants.h"
#include "deviceproxy.h"

#include <ace/Reactor.h>
#include <acewrapper/constants.h>
#include <acewrapper/mutex.hpp>
#include <acewrapper/timeval.h>
#include <acewrapper/orbservant.h>
#include <acewrapper/reactorthread.h>
#include <acewrapper/ace_string.h>
#include <acewrapper/lifecycle_frame_serializer.h>
#include <adportable/string.h>
#include <adportable/protocollifecycle.h>
#include "analyzerdevicedata.h"
#include "tofobserver_i.h"
#include "traceobserver_i.h"
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <adportable/spectrum_processor.h>

#pragma warning (disable : 4996 )
# include "tofcontrollerC.h"
# include <ace/OS.h>
# include <adinterface/global_constantsC.h>
# include <adinterface/receiverC.h>
#pragma warning (default : 4996 )

using namespace tofcontroller;

namespace tofcontroller {
	namespace internal {

		struct observer_events_data {
			bool operator == ( const observer_events_data& ) const;
			bool operator == ( const SignalObserver::ObserverEvents_ptr ) const;
			observer_events_data();
			observer_events_data( const observer_events_data& );
			SignalObserver::ObserverEvents_var cb_;
			SignalObserver::eUpdateFrequency freq_;
			std::wstring token_;
		};

		struct receiver_data {
			bool operator == ( const receiver_data& ) const;
			bool operator == ( const Receiver_ptr ) const;
			receiver_data() {};
			receiver_data( const receiver_data& t ) : receiver_(t.receiver_) {}
			Receiver_var receiver_;
		};

	}
}


TOFTask::TOFTask( size_t n ) : n_threads_(n)
                             , barrier_( n )
{
}

TOFTask::~TOFTask(void)
{
    ACE_Reactor * reactor = this->reactor();
    this->reactor( 0 );
	delete reactor;
}

bool
TOFTask::setConfiguration( const wchar_t * xml )
{
	if ( ! configXML_.empty() )
		return false;
	configXML_ = xml;
	return true;
}

bool
TOFTask::open()
{
    internal_initialize();
	if ( activate( THR_NEW_LWP, n_threads_ ) != - 1 ) {
		return true;
	}
	return false;
}

void
TOFTask::close()
{
    if ( reactor() )
		reactor()->end_reactor_event_loop();
	msg_queue()->deactivate();
	ACE_Task<ACE_MT_SYNCH>::close( 0 );
}

bool
TOFTask::connect( Receiver_ptr receiver )
{
	internal::receiver_data data;
    data.receiver_ = Receiver::_duplicate( receiver );

	acewrapper::scoped_mutex_t<> lock( mutex_ );

	if ( std::find( ibegin(), iend(), data ) != iend() )
		return false;
  
	receiver_set_.push_back( data );

	return true;
}

bool
TOFTask::disconnect( Receiver_ptr receiver )
{
	internal::receiver_data data;
    data.receiver_ = Receiver::_duplicate( receiver );

	acewrapper::scoped_mutex_t<> lock( mutex_ );

	receiver_vector_type::iterator it = std::remove( ibegin(), iend(), data );

	if ( it != iend() ) {
		receiver_set_.erase( it, iend() );
		return true;
	}
	return false;

}

SignalObserver::Observer_ptr 
TOFTask::getObserver()
{
	PortableServer::POA_var poa = singleton::tofSession_i::instance()->poa();
  
	if ( ! pObserver_ ) {
		acewrapper::scoped_mutex_t<> lock( mutex_ );
		if ( ! pObserver_ )
			pObserver_.reset( new tofObserver_i( *this ) );

        // add Traces
		SignalObserver::Description desc;
        desc.axis_x_decimals = 3;
        desc.axis_y_decimals = 2;
		desc.trace_display_name = CORBA::wstring_dup( L"TIC" );
		desc.trace_id = CORBA::wstring_dup( L"MS.TIC" );
		desc.trace_method = SignalObserver::eTRACE_TRACE;

		boost::shared_ptr< traceObserver_i > p( new traceObserver_i( *this ) );
		p->setDescription( desc );
        pTraceObserverVec_.push_back( p );
		do {
			CORBA::Object_ptr obj = poa->servant_to_reference( p.get() );
			pObserver_->addSibling( SignalObserver::Observer::_narrow( obj ) );
		} while(0);

		// add mass chromatograms
		for ( int i = 0; i < 3; ++i ) {
			boost::shared_ptr< traceObserver_i > p( new traceObserver_i( *this ) );
            desc.trace_display_name = CORBA::wstring_dup( ( boost::wformat( L"Example Chromatogram.%1%" ) % (i + 1) ).str().c_str() );
            desc.trace_id = CORBA::wstring_dup( (boost::wformat( L"MS.CHROMATOGRAM.%1%" ) % (i + 1) ).str().c_str() );
			p->setDescription( desc );
			pTraceObserverVec_.push_back( p );
			do {
				CORBA::Object_ptr obj = poa->servant_to_reference( p.get() );
				pObserver_->addSibling( SignalObserver::Observer::_narrow( obj ) );
			} while(0);
		}
	}
	CORBA::Object_ptr obj = poa->servant_to_reference( pObserver_.get() );
	return SignalObserver::Observer::_narrow( obj );
}

////////////////////////////////////////////////////////////
bool
TOFTask::connect( SignalObserver::ObserverEvents_ptr cb
				 , SignalObserver::eUpdateFrequency freq, const std::wstring& token )
{
	acewrapper::scoped_mutex_t<> lock( mutex_ );
	if ( std::find( obegin(), oend(), cb ) != oend() )
		return false;

	internal::observer_events_data data;
	data.cb_ = SignalObserver::ObserverEvents::_duplicate( cb );
    data.freq_ = freq;
    data.token_ = token;
  
	observer_events_set_.push_back( data );
	return true;
}

bool
TOFTask::disconnect( SignalObserver::ObserverEvents_ptr cb )
{
	acewrapper::scoped_mutex_t<> lock( mutex_ );

	observer_events_vector_type::iterator it = std::remove( obegin(), oend(), cb );

	if ( it != observer_events_set_.end() ) {
		observer_events_set_.erase( it, oend() );
		return true;
	}
	return false;

}


////////////////////////////////////////////////////////////////

int
TOFTask::handle_timeout( const ACE_Time_Value& tv, const void * )
{
	do {
		ACE_Message_Block * mb = new ACE_Message_Block( sizeof( tv ) );
		*reinterpret_cast< ACE_Time_Value *>(mb->wr_ptr()) = tv;
		mb->wr_ptr( sizeof(tv) );
		putq( mb );
	} while(0);
	do {
        TAO_OutputCDR cdr;
		cdr << constants::SESSION_QUERY_DEVICE;
		cdr << TOFConstants::ClassID_AnalyzerDeviceData;
		cdr << TOFConstants::ClassID_MSMethod;
        cdr << GlobalConstants::EOR;
        ACE_Message_Block * mb = cdr.begin()->duplicate();
        mb->msg_type( constants::MB_QUERY_DEVICE );
		this->putq( mb );
	} while(0);
	return 0;
}

int
TOFTask::handle_input( ACE_HANDLE h )
{
    const size_t size = 2000;
    ACE_Message_Block * mb = new ACE_Message_Block( size );
    ACE_Message_Block * pfrom = new ACE_Message_Block( sizeof( ACE_INET_Addr ) );
	ACE_INET_Addr * addr = new ( pfrom->wr_ptr() ) ACE_INET_Addr();

    int res = 0;
	if ( mcast_handler_ && ( h == mcast_handler_->get_handle() ) ) {

		mb->msg_type( constants::MB_MCAST );
		res = mcast_handler_->recv( mb->wr_ptr(), size, *addr );

	}
	if ( res == (-1) ) {
		DWORD err = GetLastError();
		(void)err;
		ACE_Message_Block::release( mb );
		ACE_Message_Block::release( pfrom );
		return 0;
	}
	mb->length( res );
	mb->cont( pfrom );

	putq( mb );

	return 0;
}

void
TOFTask::internal_initialize()
{
	acewrapper::scoped_mutex_t<> lock( mutex_ );

	acewrapper::ORBServant< tofcontroller::tofSession_i >
		* pServant = tofcontroller::singleton::tofSession_i::instance();
	CORBA::ORB_var orb = pServant->orb();
    
	CORBA::Object_var obj = orb->string_to_object( singleton::tofSession_i::instance()->broker_manager_ior() );
	Broker::Manager_var manager = Broker::Manager::_narrow( obj.in() );

	if ( ! CORBA::is_nil( manager.in() ) ) {
		logger_ = manager->getLogger();

		if ( ! CORBA::is_nil( logger_.in() ) ) {
			std::wostringstream o;
			o << L"tofcontroller task id(" << ACE_OS::getpid() << L") initialized";
			Broker::LogMessage msg;
            msg.tv_sec = msg.tv_usec = 0;
			msg.text = o.str().c_str();
			logger_->log( msg );
		}
	}
	internal_initialize_timer();
	internal_initialize_mcast();
}

bool
TOFTask::internal_initialize_reactor()
{
	ACE_Reactor * reactor = this->reactor();
	if ( reactor == 0 && !reactor_thread_ ) {
		reactor_thread_.reset( new acewrapper::ReactorThread() );
		acewrapper::ReactorThread::spawn( reactor_thread_.get() );
        reactor = reactor_thread_->get_reactor();
		this->reactor( reactor );
	}
	return true;
}

bool
TOFTask::internal_initialize_timer()
{
	if ( ! reactor() )
		internal_initialize_reactor();
	reactor()->schedule_timer( this, 0, ACE_Time_Value(3), ACE_Time_Value(3) );
	return true;
}

bool
TOFTask::internal_initialize_mcast()
{
	if ( ! reactor() )
		internal_initialize_reactor();
	if ( ! mcast_handler_ ) {
		mcast_handler_.reset( new acewrapper::McastHandler() );
		if ( mcast_handler_->open() )
			reactor()->register_handler( mcast_handler_->get_handle(), this, ACE_Event_Handler::READ_MASK );
	}
	return true;
}

int
TOFTask::svc()
{
	barrier_.wait();

	for ( ;; ) {

		ACE_Message_Block * mblk = 0;

		if ( this->getq( mblk ) == (-1) ) {
			if ( errno == ESHUTDOWN )
				ACE_ERROR_RETURN((LM_ERROR, "(%t) queue is deactivated\n"), 0);
			else
				ACE_ERROR_RETURN((LM_ERROR, "(%t) %p\n", "putq"), -1);
		}
		if ( mblk->msg_type() == ACE_Message_Block::MB_HANGUP ) {
			this->putq( mblk ); // forward the request to any peer threads
			break;
		}
		doit( mblk );
		ACE_Message_Block::release( mblk );
	}
	return 0;
}

void
TOFTask::doit( ACE_Message_Block * mblk )
{
	if ( mblk->msg_type() == ACE_Message_Block::MB_DATA ) {

        long x = 0;
        (void)x;

	} else if ( mblk->msg_type() >= ACE_Message_Block::MB_USER ) {
		switch( mblk->msg_type() ) {
		case constants::MB_COMMAND:
			dispatch_command( mblk );
			break;
		case constants::MB_MCAST:
            dispatch_mcast( mblk );
			break;
		case constants::MB_DGRAM:
            dispatch_dgram( mblk );
			break;
        case constants::MB_DEBUG:
			dispatch_debug( mblk );
            break;
		case constants::MB_SENDTO_DEVICE:
            dispatch_sendto_device( mblk );
            break;
		case constants::MB_QUERY_DEVICE:
            dispatch_query_device( mblk );
			break;
		};
	}
}

void
TOFTask::dispatch_debug( const std::wstring& text, const std::wstring& key )
{
	::EventLog::LogMessage log;
	log.tv.sec = 0;
	log.tv.usec = 0;
	log.srcId = key.c_str();
	log.format = text.c_str();
	for ( receiver_vector_type::iterator it = ibegin(); it != iend(); ++it ) {
		it->receiver_->log( log );
	}
}

void
TOFTask::dispatch_debug( ACE_Message_Block * mblk )
{
	ACE_InputCDR cdr( mblk );
	CORBA::WChar * text, *key;
	cdr.read_wstring( text );
	cdr.read_wstring( key );

	::EventLog::LogMessage log;
	log.tv.sec = 0;
	log.tv.usec = 0;
	log.srcId = key;
	log.format = text;
	for ( receiver_vector_type::iterator it = ibegin(); it != iend(); ++it ) {
		it->receiver_->log( log );
	}
}

void
TOFTask::dispatch_mcast( ACE_Message_Block * mb )
{
	ACE_Message_Block * pfrom = mb->cont();
	ACE_INET_Addr& from_addr( *reinterpret_cast<ACE_INET_Addr *>( pfrom->rd_ptr() ) );

	std::wstring key = adportable::string::convert( acewrapper::string( from_addr ) );

	DeviceProxy * proxy = DeviceProxy::check_hello_and_create( mb, from_addr, this );
	if ( proxy && ! proxy->name().empty() ) {
		device_proxies_[ proxy->name() ].reset( proxy );
		if ( proxy->initialize_dgram() ) {
			reactor()->register_handler( proxy->get_handle(), proxy, ACE_Event_Handler::READ_MASK );
		}
	}
}

void
TOFTask::dispatch_dgram( ACE_Message_Block * mblk )
{
	ACE_Message_Block * pfrom = mblk->cont();
	if ( pfrom ) {
		const ACE_INET_Addr& addr = *(reinterpret_cast< const ACE_INET_Addr *>(pfrom->rd_ptr()));
		std::string addr_str = acewrapper::string( addr );
		std::wstring key( addr_str.size(), L'\0' );
		std::copy( addr_str.begin(), addr_str.end(), key.begin() );
	}
}

void
TOFTask::dispatch_command( ACE_Message_Block * mblk )
{
    CORBA::ULong cmd = marshal<CORBA::ULong>::get( mblk ); //SESSION_INITIALIZE, MB_COMMAND );

	acewrapper::scoped_mutex_t<> lock( mutex_ );
	for ( map_type::iterator it = device_proxies_.begin(); it != device_proxies_.end(); ++it ) {
        TAO_OutputCDR cdr;
        it->second->prepare_data( cdr ) << cmd;
		it->second->sendto( cdr.begin() );
	}
}

// this entry should be serialized, marshaling with endian management

template<class T> struct sendto_device {
    T d_;
    sendto_device( TAO_InputCDR& in ) {
        in >> d_;
    }
    bool operator ()( DeviceProxy& device, CORBA::ULong cmd, CORBA::ULong cls ) const {
        TAO_OutputCDR cdr;
        device.prepare_data( cdr ) << cmd;
        cdr << cls;
        cdr << d_;
        return device.sendto( cdr.begin() );
    }
};

void
TOFTask::dispatch_sendto_device( const ACE_Message_Block * mb )
{
    assert( mb->msg_type() == constants::MB_SENDTO_DEVICE );

    TAO_InputCDR in(mb);
    CORBA::ULong scmd, clsid;
    in >> scmd;
    in >> clsid;

    assert( constants::SESSION_SENDTO_DEVICE == scmd );
    switch( clsid ) {
    case TOFConstants::ClassID_AnalyzerDeviceData:
        do {
            sendto_device<TOFInstrument::AnalyzerDeviceData> sender(in);
            acewrapper::scoped_mutex_t<> lock( mutex_ );
            // endian & marshaling has to be manipulated one by one, because it depend on device by specification
            for ( map_type::iterator it = device_proxies_.begin(); it != device_proxies_.end(); ++it )
                sender( *it->second, scmd, clsid );
        } while(0);
        break;
    default:
        assert(0);
        throw std::bad_cast( "bad data type for TOFTask::dispatch_sendto_device" );
        break;
    }
}

void
TOFTask::dispatch_query_device( const ACE_Message_Block * mb )
{
    assert( mb->msg_type() == constants::MB_QUERY_DEVICE );

    TAO_InputCDR in(mb);
    CORBA::ULong scmd, clsid;
    in >> scmd;
    assert( constants::SESSION_QUERY_DEVICE == scmd );

    std::vector< CORBA::ULong > vec;
    do {
        in >> clsid;
        vec.push_back( clsid );
    } while ( clsid != GlobalConstants::EOR );

    acewrapper::scoped_mutex_t<> lock( mutex_ );
    // endian & marshaling has to be manipulated one by one, because it depend on device by specification
    for ( map_type::iterator it = device_proxies_.begin(); it != device_proxies_.end(); ++it ) {
        TAO_OutputCDR cdr;
        it->second->prepare_data( cdr ) << scmd;
        for ( unsigned int i = 0; i < vec.size(); ++i )
            cdr << vec[i];
        it->second->sendto( cdr.begin() );
    }
}


void
TOFTask::command_initialize()
{
}

///////////////////////////////

void
TOFTask::device_update_notification( unsigned long clsId )
{
	acewrapper::scoped_mutex_t<> lock( mutex_ );

	for ( receiver_vector_type::iterator it = ibegin(); it != iend(); ++it ) {
        // todo: check client token in order to avoid broadcast clsid, which most of object can't understnad
        it->receiver_->message( Receiver::SETPTS_UPDATED, clsId );
	}
}

void
TOFTask::controller_update_notification( unsigned long clsId )
{
	acewrapper::scoped_mutex_t<> lock( mutex_ );

    struct command {
        unsigned long cmdid_;
        unsigned long clsid_;
        command( unsigned long cmdid, unsigned long clsid ) : cmdid_(cmdid), clsid_(clsid) {}
    };
    ACE_Message_Block * mb = new ACE_Message_Block( sizeof( command ) );
    *reinterpret_cast<command *>( mb->wr_ptr() ) = command( constants::SESSION_SENDTO_DEVICE, clsId);
    mb->msg_type( constants::MB_SENDTO_DEVICE );
    putq( mb );
}

void
TOFTask::adConfiguration( const TOFInstrument::ADConfigurations& v )
{
	acewrapper::scoped_mutex_t<> lock( mutex_ );
	pADConfigurations_.reset( new TOFInstrument::ADConfigurations( v ) );
}

void
TOFTask::setAnalyzerDeviceData( const TOFInstrument::AnalyzerDeviceData& d )
{
	acewrapper::scoped_mutex_t<> lock( mutex_ );
    if ( pAnalyzerDeviceData_ )
        tofcontroller::copy_helper< TOFInstrument::AnalyzerDeviceData >::copy( *pAnalyzerDeviceData_, d );
	else
		pAnalyzerDeviceData_.reset( new TOFInstrument::AnalyzerDeviceData( d ) );
}

bool
TOFTask::getAnalyzerDeviceData( TOFInstrument::AnalyzerDeviceData& d ) const
{
	acewrapper::scoped_mutex_t<> lock( const_cast< TOFTask *>(this)->mutex() );

    if ( ! pAnalyzerDeviceData_ ) {
        d.model = CORBA::string_dup("n/a");
        d.hardware_rev = CORBA::string_dup("n/a");
        d.firmware_rev = CORBA::string_dup("n/a");
        d.serailnumber = CORBA::string_dup("n/a");
        d.positive_polarity = true;
        d.ionguide_bias_voltage = 11;
        d.ionguide_rf_voltage = 12;
        d.orifice1_voltage = 13;
        d.orifice2_voltage = 14;
        d.orifice4_voltage = 15;
        d.focus_lens_voltage = 16;
        d.left_right_voltage = 17;
        d.quad_lens_voltage = 18;
        d.pusher_voltage = 19;
        d.pulling_voltage = 20;
        d.supress_voltage = 21;
        d.pushbias_voltage = 22;
        d.mcp_voltage = 23;
        d.accel_voltage = 24;  // digital value
		return false;
    }
    tofcontroller::copy_helper< TOFInstrument::AnalyzerDeviceData >::copy( d, *pAnalyzerDeviceData_ );
	return true;
}

void
TOFTask::push_profile_data( ACE_Message_Block * mb )
{
    TAO_InputCDR cdr( mb->rd_ptr(), sizeof(unsigned long) * 32 );
    
    TOFInstrument::AveragerData data;
    CORBA::ULong clsid;
    cdr >> clsid;
    assert( clsid == TOFConstants::ClassID_ProfileData );
    cdr >> data;
    unsigned char * pchar = reinterpret_cast<unsigned char *>( mb->rd_ptr() );
    pchar += 32 * sizeof(long);

    boost::scoped_array< long > pLong( new long [ data.nbrSamples ] );
    for ( size_t i = 0; i < data.nbrSamples; ++i ) {
        pLong[i] = pchar[0] << 16 | pchar[1] << 8 | pchar[2];
        if ( pchar[0] & 0x80 )
            pLong[i] |= 0xff000000;
        pchar += 3;
    }
    double dbase(0), rms(0);
    double tic = adportable::spectrum_processor::tic( data.nbrSamples, pLong.get(), dbase, rms );

    if ( pObserver_ )
        pObserver_->push_profile_data( mb );

    if ( pTraceObserverVec_.size() >= 1 )
        pTraceObserverVec_[0]->push_trace_data( data.npos, tic, data.wellKnownEvents );
}

void
TOFTask::observer_fire_on_update_data( unsigned long objId, long pos )
{
    acewrapper::scoped_mutex_t<> lock( mutex_ );
    for ( observer_events_vector_type::iterator it = obegin(); it != oend(); ++it )
        it->cb_->OnUpdateData( objId, pos );
}

void
TOFTask::observer_fire_on_method_changed( unsigned long objId, long pos )
{
    acewrapper::scoped_mutex_t<> lock( mutex_ );
    for ( observer_events_vector_type::iterator it = obegin(); it != oend(); ++it )
        it->cb_->OnMethodChanged( objId, pos );
}

void
TOFTask::observer_fire_on_event( unsigned long objId, unsigned long event, long pos )
{
    acewrapper::scoped_mutex_t<> lock( mutex_ );
    for ( observer_events_vector_type::iterator it = obegin(); it != oend(); ++it )
        it->cb_->OnEvent( objId, event, pos );
}

///////////////////////////////////////////////////////////////
bool
internal::receiver_data::operator == ( const receiver_data& t ) const
{
	return receiver_->_is_equivalent( t.receiver_.in() );
}

bool
internal::receiver_data::operator == ( const Receiver_ptr t ) const
{
	return receiver_->_is_equivalent( t );
}

///////////////////////////////////////////////////////////////
internal::observer_events_data::observer_events_data()
{
}

internal::observer_events_data::observer_events_data( const observer_events_data& t ) 
: cb_(t.cb_)
, freq_(t.freq_)
, token_(t.token_)
{
}

bool
internal::observer_events_data::operator == ( const observer_events_data& t ) const
{
	return cb_->_is_equivalent( t.cb_.in() );
}

bool
internal::observer_events_data::operator == ( const SignalObserver::ObserverEvents_ptr t ) const
{
	return cb_->_is_equivalent( t );
}

