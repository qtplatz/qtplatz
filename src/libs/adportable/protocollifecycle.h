// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <boost/smart_ptr.hpp>
#include <boost/variant.hpp>

namespace adportable {

	namespace internal {
		class LifeCycleImpl;   
	}
     
  namespace protocol {

     enum LifeCycleState {
         LCS_CLOSED
         , LCS_LISTEN
         , LCS_SYN_RCVD
         , LCS_SYN_SENT
         , LCS_ESTABLISHED
         , LCS_CLOSE_WAIT
     };
     
     enum LifeCycleCommand {
         HELO           = 0xffff0720
         , CONN_SYN     = 0x20100720
         , CONN_SYN_ACK = 0x20100721
         , CLOSE        = 0x20100722
         , CLOSE_ACK    = 0x20100723
         , DATA         = 0x20100724
         , DATA_ACK     = 0x20100725
     };
     

     ///////////////////////////////////////////////////////
     
     struct LifeCycleFrame {
	   LifeCycleFrame( LifeCycleCommand cmd = LifeCycleCommand(0) );
	   unsigned short endian_mark_;
	   unsigned short proto_version_;
	   unsigned short ctrl_;         // LSB := Ack frame
	   unsigned short hoffset_;      // fix 8
	   unsigned long command_;
     };

     // MCAST data
     struct LifeCycle_Hello {
	   static LifeCycleCommand command() { return HELO; }
	   static char * command_name() { return "HELO"; }
	   unsigned short portnumber_;
	   std::string proto_;           // "udp"
	   std::string ipaddr_;          // "0.0.0.0"
	   std::string device_name_;     // "device name, as reference"
	   std::string serial_number_;   // "unique number of device"
	   std::string revision_;        // "device firmware revision"
	   std::string model_name_;      // "device model_name"
	   std::string manufacturer_;    // "device or driver manufacturer"
	   std::string copyright_;       // optional
     };
     
     // DGRAM data
     struct LifeCycle_SYN {
	   static LifeCycleCommand command() { return CONN_SYN; }
	   static char * command_name() { return "CONN_SYN"; }
	   unsigned short sequence_;
	   unsigned short remote_sequence_;
     };
     
     struct LifeCycle_Close {
	   static LifeCycleCommand command() { return CLOSE; }
	   static char * command_name() { return "CLOSE"; }
	   unsigned short sequence_;
	   unsigned short remote_sequence_;
     };

     struct LifeCycle_SYN_Ack {
	   static LifeCycleCommand command() { return CONN_SYN_ACK; }
	   static char * command_name() { return "CONN_SYN_ACK"; }
	   unsigned short sequence_;
	   unsigned short remote_sequence_;
     };

     struct LifeCycle_Data {
	   static LifeCycleCommand command() { return DATA; }
	   static char * command_name() { return "DATA"; }
	   unsigned short sequence_;
	   unsigned short remote_sequence_;
     };

     struct LifeCycle_DataAck {
       static LifeCycleCommand command() { return DATA_ACK; }
	   static char * command_name() { return "DATA_ACK"; }
	   unsigned short sequence_;
	   unsigned short remote_sequence_;
     };

     typedef boost::variant< LifeCycle_Hello
			     , LifeCycle_SYN
			     , LifeCycle_SYN_Ack
			     , LifeCycle_Data
			     , LifeCycle_DataAck
			     , LifeCycle_Close > LifeCycleData;

	 class LifeCycle {
	 public:
		 ~LifeCycle();
		 LifeCycle();
		 LifeCycle( const LifeCycle& );
	   
		 LifeCycleState current_state() const;
		 void current_state( LifeCycleState );

		 bool apply_command( LifeCycleCommand, LifeCycleState& );
		 bool reply_received( const LifeCycleData&, adportable::protocol::LifeCycleState& nextState, adportable::protocol::LifeCycleCommand& replyCmd );
		 bool linkdown_detected( LifeCycleState& ); /* heartbeat timeout */
		 unsigned short local_sequence_post_increment();
		 unsigned short local_sequence() const;
		 unsigned short remote_sequence() const;
	 private:
		 LifeCycleState state_;
		 boost::shared_ptr<internal::LifeCycleImpl> pImpl_;
	 };

     struct LifeCycleHelper {
         static std::string to_string( const LifeCycleData& );
		 static std::string command_by_name( const LifeCycleData& );
         static unsigned short local_sequence( const LifeCycleData& );
		 static unsigned short remote_sequence( const LifeCycleData& );
     };

  }
}

