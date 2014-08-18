// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>

namespace adportable {

    template<class T> class xml_serializer {
    public:
        static bool serialize( const T& data, std::wstring& ar ) {
            boost::iostreams::back_insert_device< std::wstring > inserter( ar );
            boost::iostreams::stream< boost::iostreams::back_insert_device< std::wstring > > device( inserter );
            boost::archive::xml_woarchive oa( device );
            oa << boost::serialization::make_nvp("xml_serializer", data);
            device.flush();
            return true;
        }

        static bool deserialize( T& data, const wchar_t * s, std::size_t size ) {
            boost::iostreams::basic_array_source< wchar_t > device( s, size );
            boost::iostreams::stream< boost::iostreams::basic_array_source< wchar_t > > st( device );
            boost::archive::xml_wiarchive ia( st );
            ia >> boost::serialization::make_nvp( "xml_serializer", data );
            return true;
        }

    };

    struct make_xmlstring {
    public:
        template<class T> bool operator()( const T& data, std::wstring& ar ) const {
            ar.clear();
            return xml_serializer<T>::serialize( data, ar );
        }
    };

}

