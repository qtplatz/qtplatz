// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <ace/INET_Addr.h>
#include <boost/serialization.hpp>

namespace boost {
  namespace serialization {

    template<class Archive>
	inline void save_construct_data(Archive& ar
									, const ACE_INET_Addr& addr
									, const unsigned int version ) {
	  // todo, not tested
	  ar << addr;
    }
    template<class Archive>
      void save(Archive& ar, ACE_INET_Addr& addr, const unsigned int version ) {
	  //ar << make_nvp("ipaddr",  addr.);
    }
    template<class Archive>
	inline void load_construct_data(Archive& ar
									, ACE_INET_Addr& addr
									, const unsigned int version ) {
	  
    }

  }
}
