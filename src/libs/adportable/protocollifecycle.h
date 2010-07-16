// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <boost/smart_ptr.hpp>

namespace adportable {

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
		  HELO   = unsigned long ( 'H' << 24 | 'E' << 16 | 'L' << 8 | 'O' )
		  , CONN_SYN     = unsigned long ( 'S' << 24 | 'Y' << 16 | 'N' << 8 | 'R' )
		  , CONN_SYN_ACK = unsigned long ( 'S' << 24 | 'Y' << 16 | 'N' << 8 | 'A' )
		  , CLOSE        = unsigned long ( 'C' << 24 | 'L' << 16 | 'O' << 8 | 'S' )
	  };

	  class LifeCycleImpl;

	  class LifeCycle {
	  public:
		  ~LifeCycle();
		  LifeCycle();
		  LifeCycle( const LifeCycle& );

		  LifeCycleState current_state() const;
		  bool apply_command( LifeCycleCommand, LifeCycleState& );
		  bool reply_received( LifeCycleCommand, unsigned short rseq, LifeCycleState& );
		  bool linkdown_detected( LifeCycleState& ); /* heartbeat timeout */
	  private:
		  LifeCycleState state_;
		  boost::shared_ptr<LifeCycleImpl> pImpl_;
	  };

	  ///////////////////////////////////////////////////////

	  struct LifeCycleFrame {
		unsigned short endian_mark_;
		unsigned short proto_version_;
		unsigned short command_;
		unsigned short ctrl_;
	  };
	
	  struct LifeCycle_Hello {
	    LifeCycleFrame frame_;
		std::string connect_string_;  // "udp:[ipaddr(optional)]:<port#>"
		std::string device_name_;     // "device name, as reference"
		std::string serial_number_;   // "unique number of device"
		std::string revision_;        // "device firmware revision"
		std::string model_name_;      // "device model_name"
		std::string manufacturer_;    // "device or driver manufacturer"
		std::string copyright_;       // optional
	  };
  }
}

