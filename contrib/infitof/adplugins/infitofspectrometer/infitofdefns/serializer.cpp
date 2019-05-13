/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
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
#include "avgrdata.hpp"
#include "tracedata.hpp"
#include "method.hpp"
#include <adportable/debug.hpp>
#include <adportable/binary_serializer.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/format.hpp>

using namespace infitof;

serializer::serializer()
{
}

// static
bool
serializer::serialize( const AveragerData& data, std::string& ar )
{
    return adportable::binary::serialize<>()(data, ar);
}

bool
serializer::deserialize( AveragerData& data, const char * s, std::size_t size )
{
    return adportable::binary::deserialize<>()( data, s, size );
}

// static
bool
serializer::serialize( const std::vector< SpectrumProcessedData >& data, std::string& ar )
{
    return adportable::binary::serialize<>()(data, ar);
}

// static
bool
serializer::deserialize( std::vector< SpectrumProcessedData >& data, const char * pdevice, std::size_t size )
{
    return adportable::binary::deserialize<>()(data, pdevice, size);
}

// static
bool
serializer::serialize( const ControlMethod& data, std::string& ar )
{
    return adportable::binary::serialize<>()( data, ar );
}

bool
serializer::deserialize( ControlMethod& data, const char * s, std::size_t size )
{
    return adportable::binary::deserialize<>()(data, s, size);
}


namespace infitof {
    namespace detail {

        struct debug_dump {
            static void dump_aqrs( const infitof::AveragerData&, const char * msg, int nData );
        };

    }
}


using namespace infitof::detail;

void
debug_dump::dump_aqrs( const infitof::AveragerData& avgr, const char * msg, int nData )
{
    try {

        const infitof::acqiris::AqDescriptors& aqrs = 
            boost::get< infitof::acqiris::AqDescriptors >( avgr.desc );
        
        unsigned long flags = aqrs.segDesc.flags;  // 0:P1, 1:P2, 2:A, 3:B
        std::string name = "[";
        if ( flags & 1 )
            name += "P1,";
        if ( flags & 2 )
            name += "P2,";
        if ( flags & 4 )
            name += "A,";
        if ( flags & 8 )
            name += "B,";
        name += "]";
        
        static unsigned long long last;
        unsigned long long timestamp = uint64_t( aqrs.segDesc.timeStampHi ) << 32 | aqrs.segDesc.timeStampLo;

        ADDEBUG()
            << ( boost::format( "@ %x\t" ) % ( static_cast<const void *>( &avgr ) ) ).str()
            << msg
            << "\tnpos="            << avgr.npos
            << " "                  << ( avgr.avgrType == Averager_AP240 ? "AP240" : "ARP" )
            << " uptime="           << avgr.uptime
            << " nbrSamples="       << avgr.nbrSamples
            << " nbrAverage="       << avgr.nbrAverage
            << " nDelay="           << avgr.nSamplingDelay
            << " wkEvents="         << avgr.wellKnownEvents  
            << " nTurns="           << avgr.nTurns_deprecated     
            << " protocol="         << avgr.protocolId << "/"<< avgr.nProtocols
            << " mark:"             << ( boost::format( "%x" ) % avgr.mark ).str()
            << " waveform:"         << avgr.waveform.size()
            << " interval:" << adcontrols::metric::scale_to_milli<double>( double(timestamp - last), adcontrols::metric::pico ) / nData << "ms" // ps --> us
            << " " << ( boost::format( "0x%x" ) % flags ).str() << name
            << " sampInterval="     << avgr.sampInterval
            << " " << ( boost::format( "%.le" ) % aqrs.dataDesc.sampTime ).str()
            ;

        assert( avgr.nbrSamples );
        ADDEBUG()  << "avgr.nbrAverage(" << avgr.nbrAverage << ") == (" << aqrs.dataDesc.nbrAvgWforms << ")";
        assert( avgr.nbrAverage == unsigned( aqrs.dataDesc.nbrAvgWforms ) );
        assert( avgr.nbrSamples == unsigned( aqrs.dataDesc.returnedSamplesPerSeg ) );

        last = timestamp;
        (void)last;

    } catch ( std::exception& ex ) {
        adportable::debug(__FILE__, __LINE__) << ex.what();
    }

}

