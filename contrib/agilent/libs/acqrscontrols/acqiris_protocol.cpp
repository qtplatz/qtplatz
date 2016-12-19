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

#if __APPLE__
///usr/local/include/boost/asio/detail/macos_fenced_block.hpp:51:5: warning: 'OSMemoryBarrier' is deprecated: first deprecated in macOS 10.12 - Use
//      std::atomic_thread_fence() from <atomic> instead [-Wdeprecated-declarations]
//    OSMemoryBarrier();
# include <compiler/disable_deprecated.h>
#endif


#include "acqiris_protocol.hpp"
#include "acqiris_waveform.hpp"
#include <acqrscontrols/acqiris_method.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <fstream>

namespace acqrscontrols {
namespace aqdrv4 {

    static uint32_t __aug__ = 0x20160907;

    boost::uuids::uuid clsid_connection_request =
        boost::uuids::string_generator()( "{8fcb150a-74e7-11e6-8f1d-bfbec26f05f1}" );

    boost::uuids::uuid clsid_acknowledge =
        boost::uuids::string_generator()( "{e6eaa666-74f1-11e6-815d-b31b513d9069}" );

    boost::uuids::uuid clsid_readData =
        boost::uuids::string_generator()( "{6fff8a7c-7a63-11e6-b666-5f9c53af3753}" );

    boost::uuids::uuid clsid_temperature =
        boost::uuids::string_generator()( "{162ceb80-7a71-11e6-aafc-637da0013f14}" );    

    boost::uuids::uuid clsid_event_out =
        boost::uuids::string_generator()( "{109c4276-8046-11e6-9b9a-0f4a8020d4ef}" );    
    
    preamble::preamble( const boost::uuids::uuid& uuid, size_t size ) : clsid( uuid )
                                                                      , aug( __aug__ )
                                                                      , length( size )
                                                                      , request( 0 )
    {
    }

    bool
    preamble::isOk( const preamble * data )
    {
        return data->aug == __aug__;
    }

    std::string
    preamble::debug( const preamble * data )
    {
        std::ostringstream o;
        if ( data->aug == __aug__ )
            o << "AUG OK\t";
        else
            o << "AUG [" << boost::format( "%x" ) % data->aug << "]\t";
                
        if ( data->clsid == clsid_connection_request )
            o << "'clsid_connection_request'";
        else if ( data->clsid == clsid_acknowledge )
            o << "'clsid_acknowledge'";
        else if ( data->clsid == clsid_readData )
            o << "'clsid_readData'";
        else if ( data->clsid == clsid_temperature )
            o << "'clsid_temperature'";
        else if ( data->clsid == waveform::clsid() )       // "{33f5bfd8-793c-11e6-9bd0-1b94f4251234}"
            o << "'clsid_waveform'";
        else if ( data->clsid == acqiris_method::clsid() ) // "{A69B313C-007E-49C4-9E55-1D279A382D2A}"
            o << "'clsid_acqiris_method'";
        else
            o << boost::lexical_cast< std::string >( data->clsid );
        
        o << " payload-length: " << data->length;

        return o.str();
    }

    ////////////////
    
    acqiris_protocol::acqiris_protocol()
    {
    }

    std::vector< boost::asio::const_buffer >
    acqiris_protocol::to_buffers()
    {
        std::vector<boost::asio::const_buffer> buffers;
        
        preamble_.length = payload_.size();
        buffers.push_back( boost::asio::buffer( preamble_.data(), sizeof( struct preamble ) ) );
        buffers.push_back( boost::asio::buffer( payload_.data(), payload_.size() ) );

        return buffers;
    }


    void preamble::dump( std::ostream& of, const unsigned char * p, size_t size )
    {
        for ( size_t i = 0; i < size; ++i ) {
            if ( ( i % 32 ) == 0 )
                of << std::endl;
            of << boost::format( "%02x " ) % uint32_t( p[ i ] );
        }
        of << std::endl;
    }
}
}
