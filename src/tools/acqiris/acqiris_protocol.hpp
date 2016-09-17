/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#include <boost/uuid/uuid.hpp>
#include <memory>
#include <string>
#include <vector>
#include <adportable/debug.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <adportable/portable_binary_iarchive.hpp>
#include <boost/asio.hpp>
#include <boost/exception/all.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>

class waveform;

namespace aqdrv4 {

    class acqiris_method;

    extern boost::uuids::uuid clsid_connection_request;
    extern boost::uuids::uuid clsid_acknowledge;
    extern boost::uuids::uuid clsid_readData;
    extern boost::uuids::uuid clsid_temperature;

    struct preamble {
        uint32_t aug; // methionine
        uint32_t length;
        uint32_t request;
        boost::uuids::uuid clsid;

        preamble( const boost::uuids::uuid& uuid = boost::uuids::uuid(), size_t length = 0 );
        const char * data() const { return reinterpret_cast< const char * >( this ); }
        static bool isOk( const preamble * );
        static std::string debug( const preamble * );
    };

    class acqiris_protocol : public std::enable_shared_from_this< acqiris_protocol > {
    public:
        acqiris_protocol();

        inline class preamble& preamble()   { return preamble_; }
        inline std::string& payload() { return payload_; }

        std::vector< boost::asio::const_buffer > to_buffers();

    private:
        class preamble preamble_;
        std::string payload_;
    };

    struct protocol_serializer {
        template< typename T >
        static std::shared_ptr< acqiris_protocol > serialize( const T& d ) {

            auto data = std::make_shared< acqiris_protocol >();
            {
                boost::iostreams::back_insert_device< std::string > inserter( data->payload() );
                boost::iostreams::stream< boost::iostreams::back_insert_device< std::string > > device( inserter );
            
                portable_binary_oarchive ar( device );
                try {
                    ar & d;
                } catch ( ... ) {
                    ADDEBUG() << boost::current_exception_diagnostic_information();
                    return nullptr;
                }
            }
            data->preamble().clsid = d.clsid();
            data->preamble().length = data->payload().size();

            return data;
        }

        template< typename T >
        static std::shared_ptr< T > deserialize( const aqdrv4::preamble& pre, const char * data ) {
            
            auto s = std::string( data, pre.length );

            auto p = std::make_shared< T >();
            boost::iostreams::basic_array_source< char > device( data, pre.length );
            boost::iostreams::stream< boost::iostreams::basic_array_source< char > > st( device );
            portable_binary_iarchive ar( st );
            try {            
                ar & *p;
            } catch ( ... ) {
                ADDEBUG() << boost::current_exception_diagnostic_information();
                return nullptr;
            }

            return p;
        }
    };

}

