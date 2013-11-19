/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "bzip2.hpp"

#if defined _MSC_VER
# pragma warning(disable:4244)
#endif

#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/copy.hpp>

using namespace adportable;

bzip2::bzip2()
{
}

// static
void
bzip2::compress( std::string& compressed, const char * uncompressed, size_t length )
{
    // setup input(source) stream
    boost::iostreams::basic_array_source< char > device( uncompressed, length );
    boost::iostreams::stream< boost::iostreams::basic_array_source< char > > in( device );

    // setup output(result) stream
    boost::iostreams::back_insert_device< std::string > inserter( compressed );
    boost::iostreams::stream< boost::iostreams::back_insert_device< std::string > > out( inserter );
    
    boost::iostreams::filtering_streambuf< boost::iostreams::output > zout;
    zout.push( boost::iostreams::bzip2_compressor() );
    zout.push( out );
                        
    // compress
    boost::iostreams::copy( in, zout );                    
}

// static
void
bzip2::decompress( std::string& uncompressed, const char * compressed, size_t length )
{
    // setup input(source) stream
    boost::iostreams::basic_array_source< char > device( compressed, length );
    boost::iostreams::stream< boost::iostreams::basic_array_source< char > > in( device );

    // setup output(result) stream
    boost::iostreams::back_insert_device< std::string > inserter( uncompressed );
    boost::iostreams::stream< boost::iostreams::back_insert_device< std::string > > out( inserter );
    
    boost::iostreams::filtering_streambuf< boost::iostreams::output > zout;
    zout.push( boost::iostreams::bzip2_decompressor() );
    zout.push( out );
                        
    // compress
    boost::iostreams::copy( in, zout );                    
}

// static
bool
bzip2::is_a( const char * s, size_t length )
{
    if ( length >= 4 )
        return s[0] == 'B' && s[1] == 'Z' && s[2] == 'h' && ( '1' <= s[3] && s[3] <= '9' );
    return false;
}
