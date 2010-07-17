// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

namespace adportable {

  namespace protocol {

     struct lifecycle_header {
	   unsigned short endian_mark_;       // 0,1 [0xfffe]
	   unsigned short protocol_version_;  // 2,3 [0x0001]
     };

     class lifecycle_frame {
	public:
	   lifecycle_frame();
	   unsigned long frame_size;          // 8,9.a,b, frame data lengts in octets (including frame_size)
	   unsigned long message_;
	   unsigned char frame_data[ 1 /* frame_size in octets */ ];
	   // additional lifecycle_frame can be here...
     };

     
  }
}


