//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "protocollifecycle.hpp"
#include <boost/variant.hpp>
#include <boost/type_traits.hpp>
#include <sstream>

using namespace adportable;
using namespace adportable::protocol;
using namespace adportable::internal;

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

		struct command_hello    {	/**/ };
		struct command_syn      {	/**/ };
		struct command_syn_ack  {	/**/ };
        struct command_data     {	/**/ };
        struct command_data_ack {	/**/ };
		struct command_close    {	/**/ };

		//-----------------------        

		struct LifeCycle_Closed {
			LifeCycle_Closed() {}
			static LifeCycleState state() { return LCS_CLOSED; }
			template<class T> static bool send( LifeCycleType& ) { return false; }
			template<class T> static bool recv( LifeCycleType&, LifeCycleCommand& ) { return false; }
		};

		struct LifeCycle_Listen {
			static LifeCycleState state() { return LCS_LISTEN; }
			template<class T> static bool send( LifeCycleType& ) { return false; }
			template<class T> static bool recv( LifeCycleType&, LifeCycleCommand& ) { return false; }
		};

		struct LifeCycle_SYN_Received {
			static LifeCycleState state() { return LCS_SYN_RCVD; }
			template<class T> static bool send( LifeCycleType& ) { return false; }
			template<class T> static bool recv( LifeCycleType&, LifeCycleCommand& c ) { c = CONN_SYN_ACK; return false; }
		};

		struct LifeCycle_SYN_Sent {
			static LifeCycleState state() { return LCS_SYN_SENT; }
			template<class T> static bool send( LifeCycleType& ) { return false; }
			template<class T> static bool recv( LifeCycleType&, LifeCycleCommand& ) { return false; }
		};

		struct LifeCycle_Established {
			static LifeCycleState state() { return LCS_ESTABLISHED; }
			template<class T> static bool send( LifeCycleType& ) { return false; }
			template<class T> static bool recv( LifeCycleType&, LifeCycleCommand& ) { return false; }
		};

		struct LifeCycle_CloseWait {
			static LifeCycleState state() { return LCS_CLOSE_WAIT; }
			template<class T> static bool send( LifeCycleType& ) { return false; }
			template<class T> static bool recv( LifeCycleType&, LifeCycleCommand& ) { return false; }
		};

		///////////////////////////////////////
        /// following is an example function signature to be used in following two visitors
		/// LifeCycle_Closed::send<command_hello>( LifeCycleType& cycle );
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
            LifeCycleCommand& replyCmd_;
			lifecycle_recv_visitor( LifeCycleType& cycle, LifeCycleCommand& cmd ) : cycle_(cycle), replyCmd_(cmd) {}
			template<typename T> bool operator()( T& ) const {
				return T::recv<Command>( cycle_, replyCmd_ );
			}
		};

        ///////////// Access LifeCycle::state() ///////////////////////////// 
		struct lifecycle_state_visitor : public boost::static_visitor<LifeCycleState> {
			template<typename T> LifeCycleState operator()( T& ) const { return T::state(); }
		};

        ////////////// Access LifeCycleData a.k.a. LifeCycle_Hello::command() ////////////////////////////
		struct lifecycle_command_access_visitor : public boost::static_visitor<LifeCycleCommand> {
			template<typename T> LifeCycleCommand operator()( const T& ) const { return T::command(); }
		};

        //////////////////////////////////////////
		template<bool b> struct sequence_accessor {
			template<class T> static unsigned short sequence( const T& t ) { return t.sequence_; }
			template<class T> static unsigned short remote_sequence( const T& t ) { return t.remote_sequence_; }
			template<class T> static void sequence( T& t, unsigned short value ) { t.sequence_ = value; }
			template<class T> static void remote_sequence( T& t, unsigned short value ) { t.remote_sequence_ = value; }
		};
		template<> struct sequence_accessor<true> {
			template<class T> static unsigned short sequence( const T& ) { return 0; }
			template<class T> static unsigned short remote_sequence( const T& ) { return 0; }
			template<class T> static void sequence( T&, unsigned short ) { /* nothing */ }
			template<class T> static void remote_sequence( T&, unsigned short ) { /* nothing */ }
		};


		struct lifecycle_local_sequence_visitor : public boost::static_visitor<unsigned short> {
			template<typename T> unsigned short operator()( const T& t ) const { 
                typedef sequence_accessor< boost::is_same<T, LifeCycle_Hello>::value > impl;
                return impl::sequence( t );
			}
		};

		struct lifecycle_remote_sequence_visitor : public boost::static_visitor<unsigned short> {
			template<typename T> unsigned short operator()( const T& t ) const { 
                typedef sequence_accessor< boost::is_same<T, LifeCycle_Hello>::value > impl;
				return impl::remote_sequence( t );
			}
            template<> unsigned short operator()( const LifeCycle_Data& ) const { return 0; }
		};

		struct lifecycle_local_sequence_writer : public boost::static_visitor<void> {
			unsigned short number_;
			lifecycle_local_sequence_writer( unsigned short number ) : number_(number) {}
			template<typename T> void operator()( T& t ) const { 
                typedef sequence_accessor< boost::is_same<T, LifeCycle_Hello>::value > impl;
                impl::sequence( t, number_ );
			}
		};

		struct lifecycle_remote_sequence_writer : public boost::static_visitor<void> {
			unsigned short number_;
			lifecycle_remote_sequence_writer( unsigned short number ) : number_(number) {}
			template<typename T> void operator()( T& t ) const { 
                typedef sequence_accessor< boost::is_same<T, LifeCycle_Hello>::value > impl;
				impl::remote_sequence( t, number_ );
			}
            template<> void operator()( LifeCycle_Data& ) const { return; }
		};

		/////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////
		// namespace adportable::internal
		/////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////
		class LifeCycleImpl {
		public:
			LifeCycleImpl() : cycle_( LifeCycle_Closed() ), remote_sequence_(0), local_sequence_(0) {
			}
			bool apply_command( LifeCycleCommand cmd, LifeCycleState& );
			bool dispatch_received_data( const LifeCycleData& data, LifeCycleState&, LifeCycleCommand& );
			void force_close();

			inline unsigned short remote_sequence() const { return remote_sequence_; }
			inline unsigned short local_sequence() const { return local_sequence_; }
			inline unsigned short local_sequence_post_increment() { return local_sequence_++; }
			inline void remote_sequence( unsigned short value ) { remote_sequence_ = value; }  // last good sequence #
			inline void local_sequence( unsigned short value ) { local_sequence_ = value; }
			inline const LifeCycleType& lifeCycle() const { return cycle_; }

		private:
			unsigned short remote_sequence_;
			unsigned short local_sequence_;
			LifeCycleType cycle_;
		};
	}
}

//////////////////////
void
LifeCycleImpl::force_close()
{
	cycle_ = LifeCycle_Closed();
}

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
			local_sequence_ = 0x100;
			result = boost::apply_visitor( lifecycle_send_visitor<command_syn>(next), cycle_ );
			break;
		case CONN_SYN_ACK:
			result = boost::apply_visitor( lifecycle_send_visitor<command_syn_ack>(next), cycle_ );
			break;
        case DATA:
			result = boost::apply_visitor( lifecycle_send_visitor<command_data>(next), cycle_ );
			break;
        case DATA_ACK:
			result = boost::apply_visitor( lifecycle_send_visitor<command_data_ack>(next), cycle_ );
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
LifeCycleImpl::dispatch_received_data( const LifeCycleData& data, LifeCycleState& state, LifeCycleCommand& replyCmd )
{
    bool result = false;
    LifeCycleType next;
    replyCmd = NOTHING;
	LifeCycleCommand cmd = boost::apply_visitor( lifecycle_command_access_visitor(), data );
	switch( cmd ) {
		case HELO:
			result = boost::apply_visitor( lifecycle_recv_visitor<command_hello>(next, replyCmd), cycle_ );
			break;
		case CONN_SYN:
			result = boost::apply_visitor( lifecycle_recv_visitor<command_syn>(next, replyCmd), cycle_ );
			// initialize local/remote sequence number
            // flip local from received data onto remote of my registry
			remote_sequence( boost::apply_visitor( lifecycle_local_sequence_visitor(), data ) );
			local_sequence( remote_sequence() + 0x100 );
			break;
		case CONN_SYN_ACK:
            result = boost::apply_visitor( lifecycle_recv_visitor<command_syn_ack>(next, replyCmd), cycle_ );
			break;
        case DATA:
			result = boost::apply_visitor( lifecycle_recv_visitor<command_data>(next, replyCmd), cycle_ );
			break;
        case DATA_ACK:
			result = boost::apply_visitor( lifecycle_recv_visitor<command_data_ack>(next, replyCmd), cycle_ );
			break;
		case CLOSE:
			result = boost::apply_visitor( lifecycle_recv_visitor<command_close>(next, replyCmd), cycle_ );
			break;
	};
	if ( result ) {
		cycle_ = next;
		state = boost::apply_visitor( lifecycle_state_visitor(), cycle_ );
	}
	return result;
}

////////////////////////////////////////////////////////////////////
/////////////////// STATE CHAGE specializations ////////////////////
////////////////////////////////////////////////////////////////////

template<> bool LifeCycle_Closed::send<command_hello>( LifeCycleType& cycle )
{
	cycle = LifeCycle_Listen();
	return true;  // state has been changed from closed
}

template<> bool LifeCycle_Closed::recv<command_hello>( LifeCycleType& cycle, LifeCycleCommand& cmd )
{
	cycle = LifeCycle_Listen();
	cmd = CONN_SYN;
	return true;  // state has been changed from closed
}

////////////////// Listen state ////////////////////
template<> bool LifeCycle_Listen::send<command_syn>( LifeCycleType& cycle )
{
	cycle = LifeCycle_SYN_Sent();
	return true;
}

template<> bool LifeCycle_Listen::recv<command_syn>( LifeCycleType& cycle, LifeCycleCommand& cmd )
{
    cycle = LifeCycle_SYN_Received();  // SYN <seq#>  reply SYN_ACK <seq#>,<device seq#>
	cmd = CONN_SYN_ACK;
	return true;
}

/////////////////// SYN SENT state ////////////////
template<> bool LifeCycle_SYN_Sent::recv<command_syn_ack>( LifeCycleType& cycle, LifeCycleCommand& )
{
	// controller recieve SYN|ACK from device
	cycle = LifeCycle_Established();  // SYN|ACK <seq#>, <device seq#>
	return true;
}

/////////////////// SYN Received state ////////////////
template<> bool LifeCycle_SYN_Received::send<command_syn_ack>( LifeCycleType& cycle )
{
	cycle = LifeCycle_Established();
	return true;
}

template<> bool LifeCycle_SYN_Received::recv<command_syn_ack>( LifeCycleType& cycle, LifeCycleCommand& )
{
	// device receive SYN|ACK from controller
	cycle = LifeCycle_Established();  // SYN|ACK <seq#>, <device seq#>
	return true;
}

/////////////////// Established state ////////////////

template<> bool LifeCycle_Established::recv<command_data>( LifeCycleType& cycle, LifeCycleCommand& cmd )
{
	(void)cycle;
	cmd = DATA_ACK;
	return false;
}

template<> bool LifeCycle_Established::recv<command_close>( LifeCycleType& cycle, LifeCycleCommand& cmd )
{
	cycle = LifeCycle_Closed();
    cmd = CLOSE_ACK;
	return true;
}

//////// SEND 'CLOSE'
template<> bool LifeCycle_Established::send<command_close>( LifeCycleType& cycle )
{
	cycle = LifeCycle_CloseWait();
	return true;
}

//////// RECV 'CLOSE'
template<> bool LifeCycle_CloseWait::recv<command_close>( LifeCycleType& cycle, LifeCycleCommand& )
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

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

namespace adportable {
    namespace internal {

		struct lifecycle_command_visitor : public boost::static_visitor< LifeCycleCommand > {
			template<class T> LifeCycleCommand operator()( T& ) const { return T::command(); }
		};

        class LifeCycleData_to_string_visitor : public boost::static_visitor< std::string > {
        public:
            template<class T> std::string operator()( const T& t ) const {
                std::ostringstream o;
				o << "<" << T::command_name() << "> remote# " << t.remote_sequence_ << " local# " << t.sequence_;
				return o.str().c_str();
            }
            template<> std::string operator()( const LifeCycle_Data& data ) const {
                std::ostringstream o;
                o << "<DATA seq# " << data.sequence_ << "> flags:" << data.flags_ << " offset: " << data.offset_;
                return o.str().c_str();
            }

			template<> std::string operator()( const LifeCycle_Hello& data ) const {
				std::ostringstream o;
				o << "<HELO>"
					<< ":proto(" << data.proto_ << "," << data.portnumber_ << ")"
					<< ":addr(" << data.ipaddr_ << ")"
					<< ":name(" << data.device_name_ << ")"
					<< ":s/n(" << data.serial_number_ << ")"
					<< ":rev" << data.revision_ << ")"
					<< ":" << data.model_name_
					<< ":" << data.manufacturer_
					<< ":" << data.copyright_;
				return o.str();
			}
		};
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
std::string
LifeCycleHelper::to_string( const LifeCycleData& data )
{
    return boost::apply_visitor( LifeCycleData_to_string_visitor(), data );   
}

unsigned short
LifeCycleHelper::local_sequence( const LifeCycleData& data )
{
	return boost::apply_visitor( lifecycle_local_sequence_visitor(), data );   
}

unsigned short
LifeCycleHelper::remote_sequence( const LifeCycleData& data )
{
	return boost::apply_visitor( lifecycle_remote_sequence_visitor(), data );   
}

LifeCycleCommand
LifeCycleHelper::command( const LifeCycleData& data )
{
	return boost::apply_visitor( lifecycle_command_visitor(), data );   
}

////////////////////////////////////////////////////////////////////////////////
/////////////// LifeCycle implmentation 
///////////////////////////////////////////////////////////////////////////////
LifeCycle::~LifeCycle() 
{
}

LifeCycle::LifeCycle( unsigned short seq ) : state_( LCS_CLOSED )
                                           , pImpl_( new LifeCycleImpl() )
										   , syn_sequence_number_(seq)
{
}

LifeCycleState
LifeCycle::machine_state() const
{
	return boost::apply_visitor( lifecycle_state_visitor(), pImpl_->lifeCycle() );
}

LifeCycleState
LifeCycle::current_state() const
{
	return state_;
}

void
LifeCycle::current_state(LifeCycleState state)
{
    state_ = state;
}

unsigned short
LifeCycle::local_sequence_post_increment()
{
    return pImpl_->local_sequence_post_increment();
}

unsigned short
LifeCycle::local_sequence() const
{
    return pImpl_->local_sequence();
}

unsigned short
LifeCycle::remote_sequence() const
{
    return pImpl_->remote_sequence();
}

void
LifeCycle::remote_sequence( unsigned short value )
{
	pImpl_->remote_sequence(value);
}

void
LifeCycle::force_close()
{
	pImpl_->force_close();
}


bool
LifeCycle::apply_command( LifeCycleCommand cmd, LifeCycleState& state )
{
	return pImpl_->apply_command(cmd, state);
}

bool
LifeCycle::dispatch_received_data( const LifeCycleData& data, LifeCycleState& state, LifeCycleCommand& replyCmd )
{
	return pImpl_->dispatch_received_data( data, state, replyCmd );
}

bool
LifeCycle::linkdown_detected( LifeCycleState& state ) /* heartbeat timeout */
{
	bool res(state_ == LCS_CLOSED);
	state = state_ = LCS_CLOSED;
    return res;
}

bool
LifeCycle::validate_sequence( const LifeCycleData& data )
{
	LifeCycleCommand cmd = boost::apply_visitor( lifecycle_command_access_visitor(), data );
	unsigned short remote_sequence = boost::apply_visitor( lifecycle_remote_sequence_visitor(), data );
    if ( cmd == HELO )
		return true;

	if ( remote_sequence == ( this->remote_sequence() + 1 ) ) {
		return true;
	}
	return true; // todo
}

bool
LifeCycle::prepare_data( LifeCycleData& data, unsigned short flags, unsigned long offset )
{
    data = LifeCycle_Data();

	// set sequence number
	boost::apply_visitor( lifecycle_local_sequence_writer( local_sequence_post_increment() ), data );

    LifeCycle_Data& t = boost::get<LifeCycle_Data&>(data);
    t.flags_ = flags;
    t.offset_ = offset;
    return true;

}

bool
LifeCycle::prepare_reply_data( LifeCycleCommand cmd, LifeCycleData& data, unsigned short remote_sequence )
{
	switch( cmd ) {
		case HELO:
			data = LifeCycle_Hello();
			break;
		case CONN_SYN:
			data = LifeCycle_SYN();
			break;
		case CONN_SYN_ACK:
			data = LifeCycle_SYN_Ack();
			break;
		case CLOSE:
			data = LifeCycle_Close();
			break;
		case DATA:
			data = LifeCycle_Data();
			break;
		case DATA_ACK:
			data = LifeCycle_DataAck();
			break;
		default:
			return false;
   	}
	// set sequences
	boost::apply_visitor( lifecycle_remote_sequence_writer( remote_sequence ), data );
	boost::apply_visitor( lifecycle_local_sequence_writer( local_sequence_post_increment() ), data );
	return true;
}

// static
size_t
LifeCycle::wr_offset()
{
    return sizeof( LifeCycleFrame ) + sizeof( LifeCycle_Data );
}