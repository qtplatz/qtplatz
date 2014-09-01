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

#ifndef DESCRIPTION_H
#define DESCRIPTION_H

#include "adcontrols_global.h"
#include <string>
#include <time.h>
#include <boost/any.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <adportable/utf.hpp>

namespace adcontrols {

   class ADCONTROLSSHARED_EXPORT description {
   public:
       ~description();
       description();
       description( const description& );
       description( const std::wstring& key, const std::wstring& text );
       inline bool operator == ( const description& t ) const;
       inline const std::wstring& text() const { return text_; }
       inline const std::wstring& key() const { return key_; }
	   inline const std::string& xml() const { return xml_; }
	   inline void xml( const std::string& t ) { xml_ = t; }
	 
   private:
       time_t tv_sec_;
       long tv_usec_;
#if defined _MSC_VER
# pragma warning( disable: 4251 )
#endif
       std::wstring key_;
       std::wstring text_;
       std::string xml_;
	 
       friend class boost::serialization::access;
       template<class Archive>
           void serialize(Archive& ar, const unsigned int version ) {
           using namespace boost::serialization;
           ar & BOOST_SERIALIZATION_NVP(tv_sec_);
           ar & BOOST_SERIALIZATION_NVP(tv_usec_);
           ar & BOOST_SERIALIZATION_NVP(key_);
           ar & BOOST_SERIALIZATION_NVP(text_);
           if ( version == 1 ) {
               std::wstring wxml;
               ar & BOOST_SERIALIZATION_NVP(wxml);
               xml_ = adportable::utf::to_utf8( wxml );
           } else {
               ar & BOOST_SERIALIZATION_NVP(xml_);
           }
       }

   };

}

BOOST_CLASS_VERSION(adcontrols::description, 2);

#endif // DESCRIPTION_H
