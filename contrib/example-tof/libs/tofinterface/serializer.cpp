/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#include "serializer.hpp"
#include "tofdata.hpp"
#include "tofprocessed.hpp"
#include "protocolids.hpp"
#include "signalC.h"
#if defined _MSC_VER
# pragma warning(disable:4244)
#endif
#include <adportable/portable_binary_oarchive.hpp>
#include <adportable/portable_binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <iomanip>

using namespace tofinterface;

serializer::serializer()
{
}

// static
bool
serializer::serialize( const tofDATA& data, std::string& ar )
{
    boost::iostreams::back_insert_device< std::string > inserter( ar );
    boost::iostreams::stream< boost::iostreams::back_insert_device< std::string > > device( inserter );
    // boost::archive::binary_oarchive oa( device );
    portable_binary_oarchive oa( device );
    oa << data;
    device.flush();
    return true;
}

bool
serializer::deserialize( tofDATA& data, const char * s, std::size_t size )
{
    boost::iostreams::basic_array_source< char > device( s, size );
	boost::iostreams::stream< boost::iostreams::basic_array_source< char > > st( device );
    // boost::archive::binary_iarchive ia( st );
    portable_binary_iarchive ia( st );
    ia >> data;
    return true;
}

// static
bool
serializer::serialize( const std::vector< tofProcessedData >& data, std::string& ar )
{
    boost::iostreams::back_insert_device< std::string > inserter( ar );
    boost::iostreams::stream< boost::iostreams::back_insert_device< std::string > > device( inserter );
    // boost::archive::binary_oarchive oa( device );
    portable_binary_oarchive oa( device );
    oa << data;
    device.flush();
    return true;
}

// static
bool
serializer::deserialize( std::vector< tofProcessedData >& data, const char * pdevice, std::size_t size )
{
    boost::iostreams::basic_array_source< char > device( pdevice, size );
	boost::iostreams::stream< boost::iostreams::basic_array_source< char > > st( device );
    // boost::archive::binary_iarchive ia( st );
    portable_binary_iarchive ia( st );
    ia >> data;
    return true;
}
