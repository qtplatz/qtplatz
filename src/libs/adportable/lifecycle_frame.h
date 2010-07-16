// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

namespace adportable {

  namespace protocol {

    class lifecycle_frame {
    public:
      lifecycle_frame();
      unsigned short endian_mark_;       // 0,1
      unsigned short protocol_version_;  // 2,3
      unsigned short command_;           // 4,5
      unsigned short nframe_;            // 6,7  number of frames in this packet
      unsigned long frame_size;          // 8,9.a,b, frame data lengts in octets
      unsigned char frame_data[ /* frame_size in octets */ ];
    };

  }
}


