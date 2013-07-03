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
#include "protocolids.hpp"
#include "signalC.h"
#if defined _MSC_VER
# pragma warning(disable:4244)
#endif
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/stream.hpp>
#include <iomanip>

using namespace tofinterface;

serializer::serializer()
{
}

// static
std::size_t
serializer::serialize( const tofDATA& data, char * p, std::size_t nOctets )
{
    typedef boost::iostreams::basic_array_sink<char> Device;

    boost::iostreams::stream_buffer< Device > buffer( p, nOctets );
    boost::archive::binary_oarchive oa( buffer, boost::archive::no_header );

    // oa << data;
    oa << data.protocolId_;        // packet id
    oa << data.sequenceNumber_;    // packet sequence number
    oa << data.rtcTimeStamp_;
    oa << data.clockTimeStamp_;
    oa << data.methodId_;
    oa << data.numberOfProfiles_;
    uint16_t padding = 0xffff;
    oa << padding;

    for ( size_t n = 0; n < data.data_.size(); ++n ) {
        const tofDATA::datum& spc = data.data()[ n ];
        const std::vector< tofDATA::datum::value_type >& values = spc.values();
        size_t ndata = values.size();
        oa << ndata;
        for ( size_t i = 0; i < ndata; ++i )
            oa << values[ i ];
    }
    size_t octets = boost::iostreams::seek( buffer, 0, std::ios_base::cur ); // octets
    return octets;
}

// static
bool
serializer::serialize( TOFSignal::tofDATA& data, char * p, std::size_t nOctets )
{
    typedef boost::iostreams::basic_array_source<char> Device;

    boost::iostreams::stream_buffer< Device > buffer( p, nOctets );
    boost::archive::binary_iarchive ia( buffer, boost::archive::no_header );

    unsigned long protoId;
    ia >> protoId;
    if ( protoId != Constants::DATA )
        return false;
    
    ia >> data.sequenceNumber;
    ia >> data.rtcTimeStamp;
    ia >> data.clockTimeStamp;
    ia >> data.methodId;
    ia >> data.numberOfProfiles;
    data.data.length( data.numberOfProfiles );

    for ( size_t n = 0; n < data.numberOfProfiles; ++n ) {
        TOFSignal::datum& datum = data.data[ n ];

        size_t numdata;
        ia >> numdata;

        datum.values.length( numdata );
        for ( size_t i = 0; i < numdata; ++i )
            ia >> datum.values[ i ];

    }
    return true;
}
