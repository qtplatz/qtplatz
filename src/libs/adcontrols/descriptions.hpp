// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#include "adcontrols_global.h"
#include "description.hpp"
#include <vector>

namespace boost {
    namespace serialization {
        class access;
	}
	namespace archive {
        class xml_woarchive;
        class xml_wiarchive;
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
	   const Description& operator [](size_t idx) const;

       std::vector< Description >::iterator begin();
       std::vector< Description >::iterator end();
       std::vector< Description >::const_iterator begin() const;
       std::vector< Description >::const_iterator end() const;
       std::wstring make_folder_name( const std::wstring& regex = L".*" ) const;

	   std::string saveXml() const;
	   void loadXml( const std::string& xml );
       std::wstring toString() const;

   private:
	   friend class boost::serialization::access;
	   template<class Archiver> void serialize(Archiver& ar, const unsigned int version);
	   internal::DescriptionsImpl* pImpl_;
   };

    template<> void ADCONTROLSSHARED_EXPORT
    Descriptions::serialize( boost::archive::xml_woarchive& ar, const unsigned int version );
    
    
    template<> void ADCONTROLSSHARED_EXPORT
    Descriptions::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version );
}


