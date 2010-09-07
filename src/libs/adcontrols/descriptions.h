// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "adcontrols_global.h"
#include "description.h"
#include <boost/smart_ptr.hpp>

namespace boost {
  namespace serialization {
	class access;
  }
}

namespace adcontrols {

   namespace internal {
       class DescriptionsImpl;
   }

   class ADCONTROLSSHARED_EXPORT Descriptions {
   public:
	   ~Descriptions();
	   Descriptions();
	   Descriptions( const Descriptions& );
	   Descriptions( const std::wstring& text, const std::wstring& key );

	   void operator = ( const Descriptions& );

	   void append( const Description&, bool uniq = false );
	   size_t size() const;
	   const Description& operator [](int idx) const;

	   std::wstring saveXml() const;
	   void loadXml( const std::wstring& xml );

   private:
	   friend class boost::serialization::access;
	   template<class Archiver> void serialize(Archiver& ar, const unsigned int version);
	   internal::DescriptionsImpl* pImpl_;
   };

}

