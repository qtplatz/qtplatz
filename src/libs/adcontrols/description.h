// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#ifndef DESCRIPTION_H
#define DESCRIPTION_H

#include "adcontrols_global.h"
#include <string>
#include <time.h>

#include <boost/serialization/string.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>

namespace adcontrols {

   class ADCONTROLSSHARED_EXPORT Description{
   public:
       ~Description();
       Description();
       Description( const Description& );
       Description( const std::wstring& text, const std::wstring& key );
       inline bool operator == ( const Description& t ) const;
       inline const std::wstring& text() const { return text_; }
       inline const std::wstring& key() const { return key_; }
	 
   private:
       time_t tv_sec_;
       long tv_usec_;
       std::wstring key_;
       std::wstring text_;
	 
       friend class boost::serialization::access;
       template<class Archive>
       void serialize(Archive& ar, const unsigned int version) {
           using namespace boost::serialization;
           if ( version > 0 ) {
               ar & BOOST_SERIALIZATION_NVP(tv_sec_);
               ar & BOOST_SERIALIZATION_NVP(tv_usec_);
               ar & BOOST_SERIALIZATION_NVP(key_);
               ar & BOOST_SERIALIZATION_NVP(text_);
           }
       }

   };
}

BOOST_CLASS_VERSION(adcontrols::Description, 1);

#endif // DESCRIPTION_H
