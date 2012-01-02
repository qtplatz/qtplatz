// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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
            , NOTHING      = 0 
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
            static const char * command_name() { return "HELO"; }
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
            static const char * command_name() { return "CONN_SYN"; }
            unsigned short sequence_;
            unsigned short remote_sequence_;
        };
     
        struct LifeCycle_Close {
            static LifeCycleCommand command() { return CLOSE; }
            static const char * command_name() { return "CLOSE"; }
            unsigned short sequence_;
            unsigned short remote_sequence_;
        };

        struct LifeCycle_SYN_Ack {
            static LifeCycleCommand command() { return CONN_SYN_ACK; }
            static const char * command_name() { return "CONN_SYN_ACK"; }
            unsigned short sequence_;
            unsigned short remote_sequence_;
        };

        struct LifeCycle_Data {
            static LifeCycleCommand command() { return DATA; }
            static const char * command_name() { return "DATA"; }
            boost::uint16_t sequence_;
            boost::uint16_t flags_;  // bit 0:=not in use, bit 1:=fragmented data, bit 2:=last of fragmented data
            boost::uint32_t offset_;
        };

        struct LifeCycle_DataAck {
            static LifeCycleCommand command() { return DATA_ACK; }
            static const char * command_name() { return "DATA_ACK"; }
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
            LifeCycle( unsigned short sequence_initial = 0x100 );
            LifeCycle( const LifeCycle& );

            LifeCycleState machine_state()	const;
            LifeCycleState current_state() const;
            void current_state( LifeCycleState );

            bool validate_sequence( const LifeCycleData& );
            bool prepare_reply_data( LifeCycleCommand, LifeCycleData&, unsigned short remote_sequence );
            bool prepare_data( LifeCycleData&, unsigned short flags = 0, unsigned long offset = 0 );
            static size_t wr_offset();

            void force_close();
            bool apply_command( LifeCycleCommand, LifeCycleState& );
            bool dispatch_received_data( const LifeCycleData&, adportable::protocol::LifeCycleState& nextState, adportable::protocol::LifeCycleCommand& replyCmd );
            bool linkdown_detected( LifeCycleState& ); /* heartbeat timeout */
            unsigned short local_sequence_post_increment();
            unsigned short local_sequence() const;
            unsigned short remote_sequence() const;
            void remote_sequence( unsigned short );
        private:
            unsigned short syn_sequence_number_;
            LifeCycleState state_;
            boost::shared_ptr<internal::LifeCycleImpl> pImpl_;
        };

        struct LifeCycleHelper {
            static std::string to_string( const LifeCycleData& );
            static std::string command_by_name( const LifeCycleData& );
            static LifeCycleCommand command( const LifeCycleData& );
            static unsigned short local_sequence( const LifeCycleData& );
            static unsigned short remote_sequence( const LifeCycleData& );
        };

    }
}

