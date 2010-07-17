//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "protocollifecycle.h"
#include <boost/variant.hpp>
#include <sstream>

using namespace adportable;
using namespace adportable::protocol;

namespace adportable {

	namespace internal {

        struct LifeCycle_Closed;
		struct LifeCycle_Listen;
		struct LifeCycle_SYN_Received;
		struct LifeCycle_SYN_Sent;
		struct LifeCycle_Established;
		struct LifeCycle_CloseWait;

		typedef boost::variant< LifeCycle_Closed
			                  , LifeCycle_Listen
							  , LifeCycle_SYN_Received
							  , LifeCycle_SYN_Sent
							  , LifeCycle_Established
							  , LifeCycle_CloseWait > LifeCycleType;  

		struct command_hello   {	/**/ };
		struct command_syn     {	/**/ };
		struct command_syn_ack {	/**/ };
		struct command_close   {	/**/ };

		//-----------------------        

		struct LifeCycle_Closed {
			LifeCycle_Closed() {}
			static LifeCycleState state() { return LCS_CLOSED; }
			template<class T> static bool send( LifeCycleType& ) { return false; }
			template<class T> static bool recv( LifeCycleType& ) { return false; }
		};

		struct LifeCycle_Listen {
			static LifeCycleState state() { return LCS_LISTEN; }
			template<class T> static bool send( LifeCycleType& ) { return false; }
			template<class T> static bool recv( LifeCycleType& ) { return false; }
		};

		struct LifeCycle_SYN_Received {
			static LifeCycleState state() { return LCS_SYN_RCVD; }
			template<class T> static bool send( LifeCycleType& ) { return false; }
			template<class T> static bool recv( LifeCycleType& ) { return false; }
		};

		struct LifeCycle_SYN_Sent {
			static LifeCycleState state() { return LCS_SYN_SENT; }
			template<class T> static bool send( LifeCycleType& ) { return false; }
			template<class T> static bool recv( LifeCycleType& ) { return false; }
		};

		struct LifeCycle_Established {
			static LifeCycleState state() { return LCS_ESTABLISHED; }
			template<class T> static bool send( LifeCycleType& ) { return false; }
			template<class T> static bool recv( LifeCycleType& ) { return false; }
		};

		struct LifeCycle_CloseWait {
			static LifeCycleState state() { return LCS_CLOSE_WAIT; }
			template<class T> static bool send( LifeCycleType& ) { return false; }
			template<class T> static bool recv( LifeCycleType& ) { return false; }
		};

		///////////////////////////////////////
		///////////////////////////////////////
		template<class Command> struct lifecycle_send_visitor : public boost::static_visitor<bool> {
			LifeCycleType& cycle_;
			lifecycle_send_visitor( LifeCycleType& cycle ) : cycle_(cycle) {}
			template<typename T> bool operator()( T& ) const {
				return T::send<Command>( cycle_ );
			}
		};

        //////////////////////////////////////////
		template<class Command> struct lifecycle_recv_visitor : public boost::static_visitor<bool> {
			LifeCycleType& cycle_;
			lifecycle_recv_visitor( LifeCycleType& cycle ) : cycle_(cycle) {}
			template<typename T> bool operator()( T& ) const {
				return T::recv<Command>( cycle_ );
			}
		};

        //////////////////////////////////////////
		struct lifecycle_state_visitor : public boost::static_visitor<LifeCycleState> {
			template<typename T> LifeCycleState operator()( T& ) const { return T::state(); }
		};
	}

	namespace protocol {
		using namespace adportable::internal;

		class LifeCycleImpl {
		public:
			LifeCycleImpl() : cycle_( LifeCycle_Closed() ), remote_seq_(0), myseq_(0) {
			}
			bool apply_command( LifeCycleCommand cmd, LifeCycleState& );
			bool reply_received( LifeCycleCommand cmd, unsigned short seq, LifeCycleState& );

		private:
			unsigned short remote_seq_;
			unsigned short myseq_;
			LifeCycleType cycle_;
		};
	}
}

LifeCycle::~LifeCycle() 
{
}

LifeCycle::LifeCycle() : state_( LCS_CLOSED ), pImpl_( new LifeCycleImpl() )
{
}

LifeCycleState
LifeCycle::current_state() const
{
	return state_;
}

bool
LifeCycle::apply_command( LifeCycleCommand cmd, LifeCycleState& state )
{
	return pImpl_->apply_command(cmd, state);
}

bool
LifeCycle::reply_received( LifeCycleCommand cmd, unsigned short rseq, LifeCycleState& state )
{
	return pImpl_->reply_received(cmd, rseq, state);
}

bool
LifeCycle::linkdown_detected( LifeCycleState& state ) /* heartbeat timeout */
{
	bool res(state_ == LCS_CLOSED);
	state = state_ = LCS_CLOSED;
    return res;
}

//////////////////////

bool
LifeCycleImpl::apply_command( LifeCycleCommand cmd, LifeCycleState& state )
{
    LifeCycleType next(cycle_);
    bool result = false;
	switch( cmd ) {
		case HELO:
			result = boost::apply_visitor( lifecycle_send_visitor<command_hello>(next), cycle_ );
			break;
		case CONN_SYN:
			myseq_ = 0x100;
			result = boost::apply_visitor( lifecycle_send_visitor<command_syn>(next), cycle_ );
			break;
		case CONN_SYN_ACK:
			result = boost::apply_visitor( lifecycle_send_visitor<command_syn_ack>(next), cycle_ );
			break;
		case CLOSE:
			result = boost::apply_visitor( lifecycle_send_visitor<command_close>(next), cycle_ );
			break;
	};

	if ( result ) {
		cycle_ = next;
		state = boost::apply_visitor( lifecycle_state_visitor(), cycle_ );
	}

	return result;
}

bool
LifeCycleImpl::reply_received( LifeCycleCommand cmd, unsigned short rseq, LifeCycleState& state )
{
    static_cast<void>(rseq); // unsed
    
    bool result = false;
    LifeCycleType next;
	switch( cmd ) {
		case HELO:
			result = boost::apply_visitor( lifecycle_recv_visitor<command_hello>(next), cycle_ );
			break;
		case CONN_SYN:
			result = boost::apply_visitor( lifecycle_recv_visitor<command_syn>(next), cycle_ );
			break;
		case CONN_SYN_ACK:
			result = boost::apply_visitor( lifecycle_recv_visitor<command_syn_ack>(next), cycle_ );
			break;
		case CLOSE:
			result = boost::apply_visitor( lifecycle_recv_visitor<command_close>(next), cycle_ );
			break;
	};
	if ( result ) {
		cycle_ = next;
		state = boost::apply_visitor( lifecycle_state_visitor(), cycle_ );
	}
	return result;
}


template<> bool LifeCycle_Closed::send<command_hello>( LifeCycleType& cycle )
{
	// in closed state, controller will send 'HELO' command, 
    // then change to "LISTEN" state until connection received
	cycle = LifeCycle_Listen();
	return true;  // state has been changed from closed
}

////////////////// Listen state ////////////////////
template<> bool LifeCycle_Listen::send<command_syn>( LifeCycleType& cycle )
{
	// controller send SYN to device
	cycle = LifeCycle_SYN_Sent(); // SYN <seq#>
	return true;
}

template<> bool LifeCycle_Listen::recv<command_syn>( LifeCycleType& cycle )
{
	// device receive SYN from device
	cycle = LifeCycle_SYN_Received();  // SYN <seq#>  reply SYN_ACK <seq#>,<device seq#>
	return true;
}

/////////////////// SYN SENT state ////////////////
template<> bool LifeCycle_SYN_Sent::recv<command_syn_ack>( LifeCycleType& cycle )
{
	// controller recieve SYN|ACK from device
	cycle = LifeCycle_Established();  // SYN|ACK <seq#>, <device seq#>
	return true;
}

/////////////////// SYN Received state ////////////////
template<> bool LifeCycle_SYN_Received::recv<command_syn_ack>( LifeCycleType& cycle )
{
	// device receive SYN|AKC from controller
	cycle = LifeCycle_Established();  // SYN|ACK <seq#>, <device seq#>
	return true;
}

/////////////////// Established state ////////////////
template<> bool LifeCycle_Established::send<command_close>( LifeCycleType& cycle )
{
	cycle = LifeCycle_CloseWait();
	return true;
}

template<> bool LifeCycle_Established::recv<command_close>( LifeCycleType& cycle )
{
	cycle = LifeCycle_Closed();
	return true;
}


template<> bool LifeCycle_CloseWait::recv<command_close>( LifeCycleType& cycle )
{
	cycle = LifeCycle_Closed();
	return true;
}

////////////////////////////////////////

LifeCycleFrame::LifeCycleFrame( LifeCycleCommand cmd ) : endian_mark_(0xfffe)
						       , proto_version_(0x0001)
						       , ctrl_(0)
						       , hoffset_(8)
						       , command_(cmd)
{
}

std::string
LifeCycle_Hello::to_string( const LifeCycle_Hello& data )
{
    std::ostringstream o;

    o << "<HELO>"
        << ":proto(" << data.proto_ << "," << data.portnumber_ << ")"
        << ":addr(" << data.ipaddr_ << ")"
        << ":name(" << data.device_name_ << ")"
        << ":s/n(" << data.serial_number_ << ")"
        << ":rev" << data.revision_ << ")"
        << ":" << data.model_name_
        << ":" << data.manufacturer_
        << ":" << data.copyright_ << "</HELO>" ;
   return o.str();
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

namespace adportable {
    namespace internal {

        class LifeCycleData_to_string_visitor : public boost::static_visitor< std::string > {
        public:
            template<class T> std::string operator()( const T& ) const { return std::string("error"); }
        };

        template<> std::string LifeCycleData_to_string_visitor::operator () (const LifeCycle_Hello& t) const
        {
            return LifeCycle_Hello::to_string(t);
        }

    }
}

std::string
LifeCycleHelper::to_string( const LifeCycleData& data )
{
    using namespace adportable::internal;
    return boost::apply_visitor( LifeCycleData_to_string_visitor(), data );   
}
