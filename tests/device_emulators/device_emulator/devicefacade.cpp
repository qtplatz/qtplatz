// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
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

#include "devicefacade.h"
#include "constants.h"
#include <boost/format.hpp>
#include <vector>
#include <sstream>
#include <boost/variant.hpp>
#include <acewrapper/lifecycle_frame_serializer.h>
#include <acewrapper/outputcdr.h>
#include <acewrapper/inputcdr.h>
#include <acewrapper/dgramhandler.h>
#include <acewrapper/mcasthandler.h>
#include <acewrapper/ace_string.h>
#include <boost/variant.hpp>
#include <boost/lexical_cast.hpp>

#pragma warning (disable: 4996)
# include <ace/OS_NS_unistd.h>
# include <ace/Singleton.h>
# include <ace/INET_Addr.h>
# include <ace/Time_Value.h>
# include <ace/High_Res_Timer.h>
# include <ace/Stream.h>
#pragma warning (default: 4996)

#include "device_averager.h"
#include "device_hvcontroller.h"

#include "../tofcontroller/tofcontrollerC.h"
#include <adinterface/controlserverC.h>
#include "txtspectrum.h"

using namespace device_emulator;

class DeviceFacadeImpl {
public:
    typedef std::vector< boost::shared_ptr<device_facade_type> > vector_type;
    bool attach_device( device_facade_ptr& );
    bool detach_device( device_facade_type& );

    ACE_INET_Addr& get_remote_addr() { return remote_addr_; }
    const ACE_INET_Addr& get_remote_addr() const { return remote_addr_; }
    void set_remote_addr( const ACE_INET_Addr& addr ) { remote_addr_ = addr; }
	void update_heartbeat();
    bool heartbeat_timeout() const;
	void clear_heartbeat();
	bool handle_data( ACE_InputCDR& cdr );
    adportable::protocol::LifeCycle_Hello& hello() { return hello_; }
    vector_type::iterator begin() { return devices_.begin(); }
    vector_type::iterator end() { return devices_.end(); }
private:
    vector_type devices_;
    ACE_INET_Addr remote_addr_;
	ACE_Time_Value heartbeat_;

    adportable::protocol::LifeCycle_Hello hello_;
};

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

using namespace adportable::protocol;

namespace device_emulator {

    struct deviceType_visitor : public boost::static_visitor<std::string> {
        template<class T> std::string operator()( const T& t ) const { 
            return t.deviceType();
        }
    };

    struct lifecycle_command_visitor : public boost::static_visitor< LifeCycleCommand > {
        template<class T> LifeCycleCommand operator()( T& ) const {
            return T::command();
        }
    };

	struct handle_data_visitor : public boost::static_visitor<bool> {
        ACE_InputCDR& cdr_;
        unsigned long cmdId_;
		handle_data_visitor( ACE_InputCDR& cdr, unsigned long cmdId ) : cdr_( cdr ), cmdId_(cmdId) {
		}
		bool operator()( device_averager& impl ) const {
            return impl.instruct_handle_data( cdr_, cmdId_ );
		}
		bool operator()( device_hvcontroller& impl ) const {
			return impl.instruct_handle_data( cdr_, cmdId_ );
		}
	};

    struct handle_copy_visitor : public boost::static_visitor<bool> {
        ACE_OutputCDR& out_;
        ACE_InputCDR& in_;
        unsigned long clsId_;
		handle_copy_visitor( ACE_OutputCDR& out, ACE_InputCDR& in, unsigned long clsId ) : out_(out), in_( in ), clsId_(clsId) {
		}
        bool operator()( device_averager& impl ) const {
            return impl.instruct_copy_data( out_, in_, clsId_ );
		}
		bool operator()( device_hvcontroller& impl ) const {
            return impl.instruct_copy_data( out_, in_, clsId_ );
		}
    };

    struct activate_visitor : public boost::static_visitor<void> {
        template<class T> void operator()( T& impl ) const {
            return impl.activate();
		}
    };
    struct deactivate_visitor : public boost::static_visitor<void> {
        template<class T> void operator()( T& impl ) const {
            return impl.deactivate();
		}
    };
    bool operator == ( const device_facade_ptr& a, const device_facade_type& b ) {
        return a->which() == b.which();
    }
}


/////////////////////////////////////////////////////////////////

using namespace device_emulator;

DeviceFacade::~DeviceFacade()
{
    delete pImpl_;
}

DeviceFacade::DeviceFacade( size_t n_threads ) : pImpl_(0)
                             , lifeCycle_(0x200) 
                             , n_threads_( n_threads )
                             , barrier_( n_threads )
{
    pImpl_ = new DeviceFacadeImpl;
}

bool
DeviceFacade::attach_device( device_facade_ptr& t )
{
    if ( pImpl_->attach_device( t ) ) {
        emit signal_device_attached( boost::apply_visitor( deviceType_visitor(), *t) );
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
DeviceFacade::heartBeatToController( unsigned long value )
{
    ACE_OutputCDR cdr;
    cdr.write_ulong( static_cast<unsigned long>( GlobalConstants::ClassID_InstEvent ) );
    cdr.write_ulong( ControlServer::event_HeartBeat );
    cdr.write_ulong( value );
    ACE_Message_Block * mb = cdr.begin()->duplicate();
    mb->msg_type( constants::MB_HEARTBEAT_TO_CONTROLLER );
    putq( mb );    
    return true;
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

bool
DeviceFacade::handle_data( ACE_InputCDR& cdr )
{
	return pImpl_->handle_data( cdr );
}

void
DeviceFacade::close()
{
    msg_queue()->deactivate();
    ACE_Task<ACE_SYNCH>::close( 0 );
}
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

bool
DeviceFacadeImpl::attach_device( device_facade_ptr& t )
{
    vector_type::iterator find = std::find( devices_.begin(), devices_.end(), (*t) );
    if ( find != devices_.end() ) // already exist, should be detach_device() before this
        return false;
    devices_.push_back( t );
    boost::apply_visitor( activate_visitor(), *devices_.back() );
    return true;
}

bool
DeviceFacadeImpl::detach_device( device_facade_type& t )
{
    vector_type::iterator find = std::remove(devices_.begin(), devices_.end(), t );
    if ( find == devices_.end() ) // not exist
        return false;

    boost::apply_visitor( deactivate_visitor(), *(*find) );
    devices_.erase( find, devices_.end() );
    return true;
}

void
DeviceFacadeImpl::update_heartbeat()
{
	heartbeat_ = ACE_High_Res_Timer::gettimeofday_hr();
}

void
DeviceFacadeImpl::clear_heartbeat()
{
	heartbeat_ = ACE_Time_Value::zero;
}

bool
DeviceFacadeImpl::heartbeat_timeout() const
{
	if ( heartbeat_ != ACE_Time_Value::zero ) {
		ACE_Time_Value tv = ACE_High_Res_Timer::gettimeofday_hr();
		tv -= heartbeat_;
		if ( tv > ACE_Time_Value( 10 ) )
			return true;
	}
    return false;
}

bool
DeviceFacadeImpl::handle_data( ACE_InputCDR& cdr )
{
	acewrapper::InputCDR in(cdr);
	unsigned long cmdId;
    in >> cmdId;

	for ( vector_type::iterator it = devices_.begin(); it != devices_.end(); ++it ) {

		ACE_InputCDR tmp(cdr);
		if ( boost::apply_visitor( handle_data_visitor(tmp, cmdId), *(*it) ) )
			break;
	}
    return true;
}

//////////////////////////////  ACE_Task implementation ////////////////////////////////
bool
DeviceFacade::cancel_timer()
{
    for ( DeviceFacadeImpl::vector_type::iterator it = pImpl_->begin(); it != pImpl_->end(); ++it ) {
        boost::apply_visitor( deactivate_visitor(), *(*it) );
    }
    return true;
}

bool
DeviceFacade::close_mcast()
{
    if ( mcast_handler_ )
        mcast_handler_->close();
    return true;
}

bool
DeviceFacade::close_dgram()
{
    if ( dgram_handler_ )
        dgram_handler_->close();
    return true;
}

bool
DeviceFacade::initialize_mcast()
{
    mcast_handler_.reset( new acewrapper::McastHandler );
    if ( mcast_handler_ )
        return mcast_handler_->open();
    return false;
}

bool
DeviceFacade::initialize_dgram()
{
    dgram_handler_ = boost::shared_ptr< acewrapper::DgramHandler >( new acewrapper::DgramHandler );

    if ( dgram_handler_ ) {

        int port = 7000;
        while ( ! dgram_handler_->open(port++) && port < 7999 )
            ;
        assert( port < 7999 );
        if ( port < 7999 ) {
            adportable::protocol::LifeCycle_Hello& hello = pImpl_->hello();

            hello.portnumber_ = get_local_addr().get_port_number();
            hello.ipaddr_ = acewrapper::string( get_local_addr() );
            hello.device_name_ = "device_emulator";
            hello.manufacturer_ = "TH";
            hello.proto_ = "udp";
            hello.revision_ = "1.0";
            hello.serial_number_ = boost::lexical_cast< std::string >(ACE_OS::getpid());

            return true;
        }
    }
    return false;
}

const ACE_INET_Addr&
DeviceFacade::get_local_addr() const
{
    return static_cast<const ACE_INET_Addr&>(*dgram_handler_);
}

//////////////////////////////  ACE_Task implementation ////////////////////////////////

int
DeviceFacade::svc()
{
    barrier_.wait();
    for ( ;; ) {
        ACE_Message_Block * mblk;
        if ( getq( mblk ) == (-1) ) {
            if ( errno == ESHUTDOWN )
                break;
        }
        if ( mblk->msg_type() == ACE_Message_Block::MB_HANGUP ) {
            putq( mblk ); // forward the request to any peer threads
            break;
        }
        handleIt( mblk );
        ACE_Message_Block::release( mblk );
    }
    return 0;
}

bool
DeviceFacade::handleIt( ACE_Message_Block * mb )
{
    if ( mb->msg_type() == constants::MB_SENDTO_CONTROLLER ) {
        dump_dgram_send( mb->rd_ptr(), mb->length(), get_remote_addr() );
        return dgram_handler_->send( mb->rd_ptr(), mb->length(), get_remote_addr() );
    }

    if ( lifeCycle_.machine_state() != LCS_ESTABLISHED )
        return true;

    if ( mb->msg_type() == constants::MB_HEARTBEAT_TO_CONTROLLER ) {
        ACE_OutputCDR cdr;
        ACE_InputCDR in(mb);
        LifeCycleData data;
        if ( lifeCycle_.prepare_data( data, 0, 0 ) ) {
            acewrapper::lifecycle_frame_serializer::pack( cdr, data );
            while( in.length() >= sizeof( unsigned long ) )
                cdr.append_ulong( in );
            assert( in.length() == 0 );
            dump_dgram_send( data, cdr.begin()->rd_ptr(), cdr.length(), get_remote_addr() );
            dgram_handler_->send( cdr.begin()->rd_ptr(), cdr.length(), get_remote_addr() );
        }

    } else if ( mb->msg_type() == constants::MB_CLASS_TO_CONTROLLER ) {
        LifeCycleData data;
        if ( lifeCycle_.prepare_data( data, 0, 0 ) ) {
            ACE_OutputCDR cdr;
            if ( acewrapper::lifecycle_frame_serializer::pack( cdr, data ) ) {
                ACE_InputCDR in(mb);
                ACE_CDR::ULong clsid;
                in.read_ulong( clsid );
                for ( DeviceFacadeImpl::vector_type::iterator it = pImpl_->begin(); it != pImpl_->end(); ++it ) {
                    if ( boost::apply_visitor( handle_copy_visitor(cdr, in, clsid), *(*it) ) ) {
                        dump_dgram_send( data, cdr.begin()->rd_ptr(), cdr.length(), get_remote_addr() );
                        if ( ! dgram_handler_->send( cdr.begin()->rd_ptr(), cdr.length(), get_remote_addr() ) )
                            signal_debug("class sent to controller -- error.");
                        break;
                    }
                }
            }
        }
    } else if ( mb->msg_type() == constants::MB_DATA_TO_CONTROLLER ) {
        
        LifeCycleData data;
        if ( lifeCycle_.prepare_data( data, 0, 0 ) ) {

            ACE_OutputCDR cdr( mb );
            if ( acewrapper::lifecycle_frame_serializer::pack( cdr, data ) ) {
                size_t len = mb->length();
                dump_dgram_send( data, mb->rd_ptr(), len, get_remote_addr() );
                if ( ! dgram_handler_->send( mb->rd_ptr(), len, get_remote_addr() ) ) {
                    signal_debug("data send to controller -- error.");
                }
            }
        }
    }

    return true;
}


ACE_HANDLE
DeviceFacade::get_dgram_handle() const
{
    if ( dgram_handler_ )
        return dgram_handler_->get_handle();
    return 0;
}

ACE_HANDLE
DeviceFacade::get_mcast_handle() const
{
    if ( mcast_handler_ )
        return mcast_handler_->get_handle();
    return 0;
}


int
DeviceFacade::handle_input( ACE_HANDLE h )
{
    char rbuf[ 2000 ];
    ACE_INET_Addr from;

    adportable::protocol::LifeCycleData data;
    adportable::protocol::LifeCycleFrame frame;

    memset( rbuf, 0, sizeof(rbuf) );
    int rsize = 0;

    if ( h == dgram_handler_->get_handle() ) {

        if ( ( rsize = dgram_handler_->recv( rbuf, sizeof(rbuf), from ) ) > 0 ) {
            ACE_InputCDR cdr( rbuf, rsize );
            if ( acewrapper::lifecycle_frame_serializer::unpack( cdr, frame, data ) ) {
                dump_dgram_recv( data, rbuf, rsize, from, cdr );
                if ( handle_lifecycle_dgram( from, frame, data ) == adportable::protocol::DATA )
                    handle_data( cdr );
            }
        } else {
            signal_debug( "handle_input dgram.recv error." );
        }

    } else if ( h == mcast_handler_->get_handle() ) {

        if ( ( rsize = mcast_handler_->recv( rbuf, sizeof(rbuf), from ) ) > 0 ) {

            ACE_InputCDR cdr( rbuf, rsize );
            if ( acewrapper::lifecycle_frame_serializer::unpack( cdr, frame, data ) )
                handle_lifecycle_mcast( from, frame, data );

        } else {
            signal_debug("handle_input mcast.recv error");
        }
    }
    return 0;
}

void
DeviceFacade::handle_lifecycle_mcast( const ACE_INET_Addr& from
                                     , const adportable::protocol::LifeCycleFrame& frame
                                     , const adportable::protocol::LifeCycleData& data )
{

    // For device implmentation, nothing to be done acturally for MCAST protocol
    // just for debugging message on screen

    ACE_UNUSED_ARG( frame );

    std::ostringstream o;
    o << "[" << std::string(acewrapper::string( from )).c_str() << "]";
    o << LifeCycleHelper::to_string( data );

    emit signal_debug( QString( o.str().c_str() ) );
} 

adportable::protocol::LifeCycleCommand
DeviceFacade::handle_lifecycle_dgram( const ACE_INET_Addr& from
                                     , const adportable::protocol::LifeCycleFrame& frame
                                     , const adportable::protocol::LifeCycleData& data )
{
    ACE_UNUSED_ARG( frame );
    adportable::protocol::LifeCycleCommand gotCmd = adportable::protocol::LifeCycleHelper::command( data );

    if ( gotCmd == adportable::protocol::CONN_SYN )
        set_remote_addr( from );

    adportable::protocol::LifeCycleCommand replyCmd;
    adportable::protocol::LifeCycleState newState;

    if ( lifeCycle_.dispatch_received_data( data, newState, replyCmd ) )
        lifeCycle_.current_state( newState );    

    if ( replyCmd != adportable::protocol::NOTHING ) {
        adportable::protocol::LifeCycleData reqData;
        if ( lifeCycle_.prepare_reply_data( replyCmd, reqData, 0 ) ) {
            if ( replyCmd == CONN_SYN_ACK )
                newState = adportable::protocol::LCS_ESTABLISHED;
            lifeCycle_.apply_command( replyCmd, newState );
            
            ACE_Message_Block * mb = acewrapper::lifecycle_frame_serializer::pack( reqData );
            dump_dgram_send( mb->rd_ptr(), mb->length(), get_remote_addr() );
            dgram_handler_->send( mb->rd_ptr(), mb->length(), get_remote_addr() );
            mb->release();
        }
    }
    return gotCmd;
}

int
DeviceFacade::handle_timeout( const ACE_Time_Value& tv, const void *)
{
    ACE_UNUSED_ARG(tv);

    using namespace adportable::protocol;
    // const LifeCycle& lifeCycle = singleton::device_facade::instance()->lifeCycle();

    if ( lifeCycle_.machine_state() == LCS_CLOSED )
        singleton::device_facade::instance()->lifeCycleUpdate( HELO );

    if ( lifeCycle_.machine_state() == LCS_LISTEN ) {
        // send 'hello' message
        ACE_Message_Block * mb = acewrapper::lifecycle_frame_serializer::pack( LifeCycleData( pImpl_->hello() ) );
        mcast_handler_->send( mb->rd_ptr(), mb->length() );
        ACE_Message_Block::release( mb );
    }
      
    if ( lifeCycle_.machine_state() == LCS_ESTABLISHED ) {
        static size_t n;
        singleton::device_facade::instance()->heartBeatToController( n++ );
    }

    return 0;
}


void
DeviceFacade::register_test_spectrum( const TXTSpectrum& sp )
{
	spectra_.push_back( sp );
}


const std::vector< TXTSpectrum >& 
DeviceFacade::test_spectra() const
{
	return spectra_;
}

void
DeviceFacade::dump_dgram_send( const char * data, size_t len, const ACE_INET_Addr& addr )
{
    ACE_TCHAR astr[1024];
    addr.addr_to_string( astr, 1024 );
    std::ostringstream o;
    o << ">>dgram sendto: " << astr << "\n";
    for ( size_t i = 0; i < len && i < 128 ; ++i ) {
        o << boost::format( "%02x " ) % int(unsigned char (data[i]));
        if ( i > 0 && ( i % 64 ) == 0 )
            o << "\n";
    }
    o << "-->end.";
    signal_debug( o.str().c_str() );
}

void
DeviceFacade::dump_dgram_send( const adportable::protocol::LifeCycleData& lcd, const char * data, size_t len, const ACE_INET_Addr& addr )
{
    ACE_TCHAR astr[1024];
    addr.addr_to_string( astr, 1024 );

    static int dgram_count;
    std::string msg = (boost::format("--> %1% dgram send to %2% : ") % dgram_count++ % astr).str();
    msg += adportable::protocol::LifeCycleHelper::to_string( lcd );

    if ( adportable::protocol::LifeCycleHelper::command( lcd ) == adportable::protocol::DATA ) {
        ACE_InputCDR cdr( data, len );
        adportable::protocol::LifeCycleData lcdata;
        adportable::protocol::LifeCycleFrame frame;
        if ( acewrapper::lifecycle_frame_serializer::unpack( cdr, frame, lcdata ) ) {
            ACE_CDR::ULong cmdId, clsId;
            cdr.read_ulong( cmdId );
            cdr.read_ulong( clsId );
            msg += (boost::format(" appdata[%04x %04x...]") % cmdId % clsId ).str();
        }
    }
    signal_debug( msg.c_str() );

    std::ostringstream o;
    const unsigned short * udata = reinterpret_cast< const unsigned short * >(data);
    for ( size_t i = 0; i < len/2 && i < 64; ++i ) {
        o << boost::format( "%04x " ) % udata[i];
        if ( i > 0 && ( i % 32 ) == 0 )
            o << "\n";
    }
    o << "-->end.";
    signal_debug( o.str().c_str() );
}

void
DeviceFacade::dump_dgram_recv( const adportable::protocol::LifeCycleData& lcd
                              , const char * data, size_t len
                              , const ACE_INET_Addr& addr
                              , const ACE_InputCDR& cdr )
{
    ACE_TCHAR astr[1024];
    addr.addr_to_string( astr, 1024 );

    static int dgram_count;
    std::string msg = (boost::format("--> %1% dgram recv from %2% : ") % dgram_count++ % astr).str();
    msg += adportable::protocol::LifeCycleHelper::to_string( lcd );

    if ( adportable::protocol::LifeCycleHelper::command( lcd ) == adportable::protocol::DATA ) {
        const adportable::protocol::LifeCycle_Data& data = boost::get< adportable::protocol::LifeCycle_Data >( lcd );
        (void)data;
        ACE_InputCDR temp = cdr;
        ACE_CDR::ULong cmdId, clsId;
        temp.read_ulong( cmdId );
        temp.read_ulong( clsId );
        msg += (boost::format(" cmdId(%04x), clsId(%8x)") % cmdId % clsId).str();
    }
    signal_debug( msg.c_str() );

    const unsigned short * udata = reinterpret_cast< const unsigned short * >(data);
    std::ostringstream o;
    for ( size_t i = 0; i < len/2; ++i ) {
        o << boost::format( "%04x " ) % udata[i];
        if ( i > 0 && ( i % 32 ) == 0 )
            o << "\n";
    }
    o << "<--end.";
    signal_debug( o.str().c_str() );
}


