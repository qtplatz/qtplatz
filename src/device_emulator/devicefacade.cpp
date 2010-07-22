//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#include "devicefacade.h"
#include <vector>
#include <boost/variant.hpp>
#include <acewrapper/lifecycle_frame_serializer.h>
#include <ace/Singleton.h>
#include <ace/INET_Addr.h>

#include "roleanalyzer.h"
#include "roleaverager.h"
#include "roleesi.h"

class DeviceFacadeImpl {
public:
    typedef std::vector< device_facade_type > vector_type;
    bool attach_device( device_facade_type& );
    bool detach_device( device_facade_type& );

    ACE_INET_Addr& get_remote_addr() { return remote_addr_; }
    const ACE_INET_Addr& get_remote_addr() const { return remote_addr_; }
    void set_remote_addr( const ACE_INET_Addr& addr ) { remote_addr_ = addr; }
private:
    vector_type devices_;
    ACE_INET_Addr remote_addr_;
};

/////////////////////////////////////////////////////////////////
struct deviceType_visitor : public boost::static_visitor<std::string> {
    template<class T> std::string operator()( const T& t ) const { return t.deviceType(); }
};

/////////////////////////////////////////////////////////////////

using namespace adportable::protocol;

struct lifecycle_command_visitor : public boost::static_visitor< LifeCycleCommand > {
    template<class T> LifeCycleCommand operator()( T& ) const { return T::command(); }
};

/////////////////////////////////////////////////////////////////
DeviceFacade::~DeviceFacade()
{
    delete pImpl_;
}

DeviceFacade::DeviceFacade() : pImpl_(0)
                             , lifeCycle_(0x200) 
{
    pImpl_ = new DeviceFacadeImpl;
}

bool
DeviceFacade::attach_device( device_facade_type& t )
{
    if ( pImpl_->attach_device( t ) ) {
        emit signal_device_attached( boost::apply_visitor( deviceType_visitor(), t) );
        return true;
    }
    return false;
}

bool
DeviceFacade::detach_device( device_facade_type& t )
{
    if ( pImpl_->detach_device( t ) ) {
        emit signal_device_detached( boost::apply_visitor( deviceType_visitor(), t) );
        return true;
    }
    return false;
}

bool
DeviceFacade::lifeCycleUpdate( adportable::protocol::LifeCycleCommand cmd )
{
    using namespace adportable::protocol;
    LifeCycleState state;
    if ( lifeCycle_.apply_command( cmd, state ) ) {
        lifeCycle_.current_state( state );
        return true;
    }
    return false;
}

bool
DeviceFacade::handle_dgram( const LifeCycleFrame& frame, const LifeCycleData& data, LifeCycleData& replyData )
{
    LifeCycleCommand cmd = boost::apply_visitor( lifecycle_command_visitor(), data );
    assert( frame.command_ == cmd );
    std::string msg = LifeCycleHelper::to_string( data );
	emit signal_debug( QString( "handle_dgram got " ) + msg.c_str() );

    LifeCycleState nextState;
    LifeCycleCommand replyCmd;

	lifeCycle_.dispatch_received_data( data, nextState, replyCmd );
	if ( lifeCycle_.validate_sequence( data ) ) {
		unsigned short remote_sequence = LifeCycleHelper::local_sequence( data );  // flip local number on recived data to remote on mine
		if ( lifeCycle_.prepare_reply_data( replyCmd, replyData, remote_sequence ) ) {
			std::string rmsg = LifeCycleHelper::to_string( replyData );
			emit signal_debug( QString( "handle_dgram reply " ) + rmsg.c_str() );
			return true;
		}
	}
	return false;
}

const ACE_INET_Addr&
DeviceFacade::get_remote_addr() const
{
    return pImpl_->get_remote_addr();
}

void
DeviceFacade::set_remote_addr( const ACE_INET_Addr& addr )
{
    pImpl_->set_remote_addr( addr );
}


///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

bool
DeviceFacadeImpl::attach_device( device_facade_type& t )
{
    vector_type::iterator find = std::find( devices_.begin(), devices_.end(), t );
    if ( find != devices_.end() ) // already exist, should be detach_device() before this
        return false;
    devices_.push_back( t );
    return true;
}

bool
DeviceFacadeImpl::detach_device( device_facade_type& t )
{
    vector_type::iterator find = std::remove(devices_.begin(), devices_.end(), t );
    if ( find == devices_.end() ) // not exist
        return false;
    devices_.erase( find, devices_.end() );
    return true;
}
