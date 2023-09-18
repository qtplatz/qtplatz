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
#include <boost/json/fwd.hpp>
#include <boost/json/value_to.hpp>
#include <vector>
#include <optional>

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

	   void append( const description&, bool uniq = true );
	   size_t size() const;
	   const description& operator [] ( size_t idx ) const;

       std::vector< description >::iterator begin();
       std::vector< description >::iterator end();
       std::vector< description >::const_iterator begin() const;
       std::vector< description >::const_iterator end() const;
       std::wstring make_folder_name( const std::wstring& regex = L".*", bool negative_lookaround = false ) const;
       std::string make_folder_name( const std::string& regex = ".*", bool negative_lookaround = false ) const;

       std::wstring toString() const;
       std::string toJson() const;

       std::optional< std::string > hasKey( const std::string& regex ) const;

       static bool xml_archive( std::wostream&, const descriptions& );
       static bool xml_restore( std::wistream&, descriptions& );

   private:
       class impl;
       std::unique_ptr< impl > impl_;

	   friend class boost::serialization::access;
	   template<class Archiver> void serialize(Archiver& ar, const unsigned int version);

       friend ADCONTROLSSHARED_EXPORT void tag_invoke( boost::json::value_from_tag, boost::json::value&, const descriptions& );
       friend ADCONTROLSSHARED_EXPORT descriptions tag_invoke( boost::json::value_to_tag< descriptions >&, const boost::json::value& );
   };

    template<> void ADCONTROLSSHARED_EXPORT
    descriptions::serialize( boost::archive::xml_woarchive& ar, const unsigned int version );


    template<> void ADCONTROLSSHARED_EXPORT
    descriptions::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version );
}
