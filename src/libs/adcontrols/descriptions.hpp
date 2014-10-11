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
       class descriptionsImpl;
   }

   class ADCONTROLSSHARED_EXPORT descriptions {
   public:
	   ~descriptions();
	   descriptions();
	   descriptions( const descriptions& );
	   descriptions( const std::wstring& text, const std::wstring& key );

	   void operator = ( const descriptions& );

	   void append( const description&, bool uniq = false );
	   size_t size() const;
	   const description& operator [] ( size_t idx ) const;
       descriptions& operator << ( const description& );

       std::vector< description >::iterator begin();
       std::vector< description >::iterator end();
       std::vector< description >::const_iterator begin() const;
       std::vector< description >::const_iterator end() const;
       std::wstring make_folder_name( const std::wstring& regex = L".*" ) const;

       std::wstring toString() const;

       static bool xml_archive( std::wostream&, const descriptions& );
       static bool xml_restore( std::wistream&, descriptions& );

   private:
	   friend class boost::serialization::access;
	   template<class Archiver> void serialize(Archiver& ar, const unsigned int version);
	   internal::descriptionsImpl* pImpl_;
   };

    template<> void ADCONTROLSSHARED_EXPORT
    descriptions::serialize( boost::archive::xml_woarchive& ar, const unsigned int version );
    
    
    template<> void ADCONTROLSSHARED_EXPORT
    descriptions::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version );
}


