// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

// #include <adportable/lifecycleprotocol.h>

namespace adportable {
    namespace protocol {
        struct LifeCycleFrame;
        struct LifeCycle_Hello;
        struct LifeCycle_SYN;
        struct LifeCycle_Ack;
        struct LifeCycle_Close;
    }
}


class ACE_Message_Block;

namespace acewrapper {

  class lifecycle_frame_serializer {
  public:
    lifecycle_frame_serializer();

    template<class T> static ACE_Message_Block * pack( const T& );
    template<class T> static bool unpack( ACE_Message_Block *, adportable::protocol::LifeCycleFrame&, T& );
  };

  

}


