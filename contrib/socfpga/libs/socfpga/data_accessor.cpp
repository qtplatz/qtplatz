/**************************************************************************
** Copyright (C) 2010-2019 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2019 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "data_accessor.hpp"
#include "advalue.hpp"
#include <adportable/debug.hpp>
#include <adportable/binary_serializer.hpp>
#include <adportable/date_string.hpp>
#include <adportable/portable_binary_archive.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <adportable/portable_binary_iarchive.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/serialization/array.hpp>
#include <boost/format.hpp>
#include <chrono>

namespace socfpga {
    namespace dgmod {

        template<class Archive>
        void serialize( Archive & ar, advalue& _, const unsigned int version )
        {
            ar & BOOST_SERIALIZATION_NVP( _.elapsed_time );
            ar & BOOST_SERIALIZATION_NVP( _.flags_time );    //!< 'data[desc.indexFirstPoint]' is the first valid point.
            ar & BOOST_SERIALIZATION_NVP( _.posix_time );
            ar & BOOST_SERIALIZATION_NVP( _.adc_counter );
            ar & BOOST_SERIALIZATION_NVP( _.nacc );
            ar & BOOST_SERIALIZATION_NVP( _.flags );   //!< When reading multiple segments in one waveform
            ar & BOOST_SERIALIZATION_NVP( _.ad );
        }
    }
}

using namespace socfpga::dgmod;

data_accessor::data_accessor()
{
}

data_accessor::data_accessor( std::shared_ptr< const std::vector< socfpga::dgmod::advalue > > data ) : data_( data )
{
    assert( data_ );
    it_ = data_->begin();
}

size_t
data_accessor::ndata() const
{
    assert( data_ );
    return data_->size();
}

void
data_accessor::rewind()
{
    assert( data_ );
    it_ = data_->begin();
}

bool
data_accessor::next()
{
    assert( data_ );
    return ++it_ != data_->end();

}

uint64_t
data_accessor::elapsed_time() const
{
    assert( data_ );
    return it_->elapsed_time;
}

uint64_t
data_accessor::epoch_time() const
{
    assert( data_ );
    return it_->posix_time;
}

uint64_t
data_accessor::pos() const
{
    assert( data_ );
    return it_->adc_counter;
}

uint32_t
data_accessor::fcn() const
{
    return 0;
}

uint32_t
data_accessor::events() const
{
    return it_->flags;
}

size_t
data_accessor::xdata( std::string& ar ) const
{
    boost::iostreams::back_insert_device< std::string > inserter( ar );
    boost::iostreams::stream< boost::iostreams::back_insert_device< std::string > > device( inserter );

    portable_binary_oarchive oa( device );

    oa & *it_;

    device.flush();

    return ar.size();
 }

size_t
data_accessor::xmeta( std::string& ar ) const
{
    return 0;
}

//static
bool
data_accessor::deserialize( std::vector< socfpga::dgmod::advalue >& data, const char* xdata, size_t xsize )
{
    socfpga::dgmod::advalue d;
    adportable::binary::deserialize<>()( d, xdata, xsize );
    data.emplace_back( d );

#if !defined NDEBUG && 0
    double t0 = 0;
    auto it = std::find_if( data.begin(), data.end(), [](const auto& a){ return a.flags & 0x10000000; });
    if ( it != data.end() )
        t0 = it->flags_time;
    debug_print( d, t0 );
#endif
    return true;
}

//static
void
data_accessor::debug_print( const socfpga::dgmod::advalue& d, double t0 )
{
    std::ostringstream o;
    for ( auto& v: d.ad )
        o << boost::format( ", %.3lf" ) % v;

#if defined (Q_OS_MACOS)
    auto tp = std::chrono::system_clock::time_point( std::chrono::microseconds( d.posix_time/1000 ) );
#else
    auto tp = std::chrono::system_clock::time_point( std::chrono::microseconds( d.posix_time ) );
#endif

    //std::chrono::system_clock::time_point() + std::chrono::nanoseconds( d.posix_time );
    
    ADDEBUG() << "time: " << boost::format( "%.4fs" ) % (( d.elapsed_time - t0 ) / 1.0e9)
              << ", flag: " << boost::format( "{0x%x, %.3f}" ) % (d.flags >> 24) % (( d.flags_time - t0 ) / 1.0e9 )
              << ", " << adportable::date_string::logformat( tp, true )
              << "\t" << o.str();
}
