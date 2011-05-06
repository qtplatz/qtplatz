//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "lifecycle_frame_serializer.h"
#include <adportable/protocollifecycle.hpp>
#include <acewrapper/OutputCDR.h>
#include <acewrapper/InputCDR.h>
#include <boost/variant/static_visitor.hpp>

using namespace acewrapper;
using namespace adportable::protocol;

lifecycle_frame_serializer::lifecycle_frame_serializer()
{
}

//////////////////////////////////////////////////////////////////////////////////////

namespace acewrapper {
   namespace internal {
      
      using namespace adportable::protocol;
      using namespace acewrapper;

      class lifecycle_serializer {
      public:
	    static void serialize( OutputCDR& cdr, const LifeCycleFrame& frame ) {
	       cdr << frame.endian_mark_;
	       cdr << frame.proto_version_;
	       cdr << frame.ctrl_;
	       cdr << frame.hoffset_;
	       cdr << frame.command_;
	    }
	    static void serialize( ACE_OutputCDR& cdr, const LifeCycleFrame& frame ) {
	       cdr.write_ushort( frame.endian_mark_ );
		   cdr.write_ushort( frame.proto_version_ );
		   cdr.write_ushort( frame.ctrl_);
		   cdr.write_ushort( frame.hoffset_ );
		   cdr.write_ulong( frame.command_ );
	    }

      };

      //-----------
      class lifecycle_command_visitor : public boost::static_visitor< adportable::protocol::LifeCycleCommand > {
      public:
          template<class T> LifeCycleCommand operator()( T& ) const { return T::command(); }
      };

      //-----------------
      class lifecycle_serializer_visitor : public boost::static_visitor< unsigned int > {
	    ACE_OutputCDR& cdr_;
	 public:
	    lifecycle_serializer_visitor( ACE_OutputCDR& cdr ) : cdr_(cdr) {}
        unsigned int operator()( const LifeCycle_Hello& data ) const;
        unsigned int operator()( const LifeCycle_SYN& data ) const;
        unsigned int operator()( const LifeCycle_SYN_Ack& data ) const;
        unsigned int operator()( const LifeCycle_Data& data ) const;
        unsigned int operator()( const LifeCycle_DataAck& data ) const;
        unsigned int operator()( const LifeCycle_Close& data ) const;
      };
      //-----------------
      class lifecycle_deserializer_visitor : public boost::static_visitor< void > {
		  ACE_InputCDR& cdr_;
	 public:
	    lifecycle_deserializer_visitor( ACE_InputCDR& in ) : cdr_(in) {}
        void operator()( LifeCycleFrame& frame ) const;
	    void operator()( LifeCycle_Hello& data ) const;
	    void operator()( LifeCycle_SYN& data ) const;
	    void operator()( LifeCycle_SYN_Ack& data ) const;
	    void operator()( LifeCycle_Data& data ) const;
	    void operator()( LifeCycle_DataAck& data ) const;
	    void operator()( LifeCycle_Close& data ) const;
      };
   }
}


//////////////////////////////////////////////////////////////////////////////////////
template<> bool
lifecycle_frame_serializer::pack( ACE_OutputCDR& cdr, const LifeCycleData& v )
{
    LifeCycleFrame frame( boost::apply_visitor( internal::lifecycle_command_visitor(), v ) );

    internal::lifecycle_serializer::serialize(cdr, frame);
    unsigned int size = boost::apply_visitor( internal::lifecycle_serializer_visitor(cdr), v );
	if ( size )
		return true;
	return false;
}

template<> ACE_Message_Block *
lifecycle_frame_serializer::pack( const LifeCycleData& v )
{
    ACE_OutputCDR ace_cdr;
    OutputCDR cdr(ace_cdr);

    LifeCycleFrame frame( boost::apply_visitor( internal::lifecycle_command_visitor(), v ) );

    internal::lifecycle_serializer::serialize(cdr, frame);
    unsigned int size = boost::apply_visitor( internal::lifecycle_serializer_visitor(cdr), v );
    ACE_UNUSED_ARG(size);

    ACE_Message_Block * mb = cdr.begin()->clone();
    mb->length( cdr.length() );
    return mb;
}

template<> bool
lifecycle_frame_serializer::unpack( ACE_InputCDR& cdr, LifeCycleFrame& frame, LifeCycleData& v )
{
	internal::lifecycle_deserializer_visitor unpacker(cdr);
	unpacker( frame );

	switch( frame.command_ ) {
    case HELO:
		v = LifeCycle_Hello();
		break;
	case CONN_SYN:
		v = LifeCycle_SYN();
		break;
	case CONN_SYN_ACK:
		v = LifeCycle_SYN_Ack();
		break;
	case DATA:
		v = LifeCycle_Data();
		break;
	case DATA_ACK:
		v = LifeCycle_DataAck();
		break;
	case CLOSE:
		v = LifeCycle_Close();
		break;
	default:
		return false;
	}
	boost::apply_visitor( unpacker, v );
	return true;
}

template<> bool
lifecycle_frame_serializer::unpack( ACE_Message_Block * mb, LifeCycleFrame& frame, LifeCycleData& v )
{
    ACE_InputCDR input( mb );
    InputCDR cdr( input );
	return unpack( cdr, frame, v );
}

/////////////////////////////////////////
namespace acewrapper {
    namespace internal {

        unsigned int lifecycle_serializer_visitor::operator()( const LifeCycle_Hello& data ) const
        {
            unsigned int orign = cdr_.length();
            OutputCDR cdr( cdr_ );
            cdr << data.portnumber_;
            cdr << data.proto_;
            cdr << data.ipaddr_;
            cdr << data.device_name_;
            cdr << data.serial_number_;
            cdr << data.revision_;
            cdr << data.model_name_;
            cdr << data.manufacturer_;
            cdr << data.copyright_;
            return cdr.length() - orign;
        }

        unsigned int lifecycle_serializer_visitor::operator()( const LifeCycle_SYN& data ) const
        {
            unsigned int orign = cdr_.length();
			OutputCDR cdr( cdr_ );
            cdr << data.sequence_;
            cdr << data.remote_sequence_;
            return cdr.length() - orign;
        }

        unsigned int lifecycle_serializer_visitor::operator()( const LifeCycle_SYN_Ack& data ) const
        {
            unsigned int orign = cdr_.length();
			OutputCDR cdr( cdr_ );
            cdr << data.sequence_;
            cdr << data.remote_sequence_;
            return cdr.length() - orign;
        }

        unsigned int lifecycle_serializer_visitor::operator()( const LifeCycle_Data& data ) const
        {
            unsigned int orign = cdr_.length();
			OutputCDR cdr( cdr_ );
            cdr << data.sequence_;
            cdr << data.flags_;
            cdr << data.offset_;
            return cdr.length() - orign;
        }

        unsigned int lifecycle_serializer_visitor::operator()( const LifeCycle_DataAck& data ) const
        {
            unsigned int orign = cdr_.length();
			OutputCDR cdr( cdr_ );
            cdr << data.sequence_;
            cdr << data.remote_sequence_;
            return cdr.length() - orign;
        }

        unsigned int lifecycle_serializer_visitor::operator()( const LifeCycle_Close& data ) const
        {
            unsigned int orign = cdr_.length();
			OutputCDR cdr( cdr_ );
            cdr << data.sequence_;
            cdr << data.remote_sequence_;
            return cdr.length() - orign;
        }

    }
}

namespace acewrapper {
    namespace internal {

        void lifecycle_deserializer_visitor::operator()( LifeCycleFrame& frame ) const
        {
            InputCDR cdr( cdr_ );
            cdr >> frame.endian_mark_;
            if ( frame.endian_mark_ == 0xfeff )
                static_cast<ACE_InputCDR&>(cdr).reset_byte_order( ACE_CDR_BYTE_ORDER == 1 ? 0 : 1 );
            cdr >> frame.proto_version_;
            cdr >> frame.ctrl_;
            cdr >> frame.hoffset_;
            cdr >> frame.command_;
        }

        void lifecycle_deserializer_visitor::operator()( LifeCycle_Hello& data ) const
        {
            InputCDR cdr( cdr_ );
            cdr >> data.portnumber_;
            cdr >> data.proto_;
            cdr >> data.ipaddr_;
            cdr >> data.device_name_;
            cdr >> data.serial_number_;
            cdr >> data.revision_;
            cdr >> data.model_name_;
            cdr >> data.manufacturer_;
            cdr >> data.copyright_;
        }

        void lifecycle_deserializer_visitor::operator()( LifeCycle_SYN& data ) const
        {
            InputCDR cdr( cdr_ );
            cdr >> data.sequence_;
            cdr >> data.remote_sequence_;
        }

        void lifecycle_deserializer_visitor::operator()( LifeCycle_SYN_Ack& data ) const
        {
            InputCDR cdr( cdr_ );
            cdr >> data.sequence_;
            cdr >> data.remote_sequence_;
        }

        void lifecycle_deserializer_visitor::operator()( LifeCycle_Data& data ) const
        {
            InputCDR cdr( cdr_ );
            cdr >> data.sequence_;
            cdr >> data.flags_;
            cdr >> data.offset_;
        }

        void lifecycle_deserializer_visitor::operator()( LifeCycle_DataAck& data ) const
        {
            InputCDR cdr( cdr_ );
            cdr >> data.sequence_;
            cdr >> data.remote_sequence_;
        }

        void lifecycle_deserializer_visitor::operator()( LifeCycle_Close& data ) const
        {
            InputCDR cdr( cdr_ );
            cdr >> data.sequence_;
            cdr >> data.remote_sequence_;
        }

    }
}
