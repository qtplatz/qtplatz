/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "threshold_result.hpp"
#include "waveform.hpp"
#include <adcontrols/timedigitalhistogram.hpp>
#include <adportable/debug.hpp>
#include <adportable/portable_binary_iarchive.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <boost/format.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>

using namespace acqrscontrols::ap240;

threshold_result::~threshold_result()
{
}

threshold_result::threshold_result() : foundIndex_(-1)
                                     , findRange_( 0, 0 )
{
}

threshold_result::threshold_result( std::shared_ptr< const acqrscontrols::ap240::waveform > d ) : data_( d )
                                                                                                , foundIndex_( -1 )
                                                                                                , findRange_( 0, 0 )
{
}

threshold_result::threshold_result( const threshold_result& t ) : indecies_( t.indecies_ )
                                                                , data_( t.data_ )
                                                                , processed_( t.processed_ )
                                                                , foundIndex_( t.foundIndex_ )
                                                                , findRange_( t.findRange_ )
{
}

std::shared_ptr< const acqrscontrols::ap240::waveform >&
threshold_result::data()
{
    return data_;
}

std::vector< uint32_t >&
threshold_result::indecies()
{
    return indecies_;
}

std::vector< double >&
threshold_result::processed()
{
    return processed_;
}

std::shared_ptr< const acqrscontrols::ap240::waveform >
threshold_result::data() const
{
    return data_;
}

const std::vector< uint32_t >&
threshold_result::indecies() const
{
    return indecies_;
}

const std::vector< double >&
threshold_result::processed() const
{
    return processed_;
}

const std::pair<uint32_t, uint32_t >&
threshold_result::findRange() const
{
    return findRange_;
}

uint32_t
threshold_result::foundIndex() const
{
    return foundIndex_;
}

void
threshold_result::setFoundAction( uint32_t index, const std::pair< uint32_t, uint32_t >& range )
{
    foundIndex_ = index;
    findRange_ = range;
}

bool
threshold_result::deserialize( const int8_t * xdata, size_t dsize, const int8_t * xmeta, size_t msize )
{
    //auto data = std::make_shared< acqrscontrols::ap240::waveform >();
    auto data = std::make_shared< waveform >();

    data->deserialize_xmeta( reinterpret_cast<const char *>( xmeta ), msize );

    data_ = data;

    // restore indecies
    boost::iostreams::basic_array_source< char > device( reinterpret_cast< const char *>(xdata), dsize );
    boost::iostreams::stream< boost::iostreams::basic_array_source< char > > st( device );

    try {
        portable_binary_iarchive ar( st );
        ar >> indecies_;
    } catch ( std::exception& ) {
        return false;
    }

    return true;
}

namespace acqrscontrols {

    struct index_to_time {
        double xIncrement, initialXOffset;
        index_to_time( const waveform& t ) : xIncrement( t.meta_.xIncrement ), initialXOffset( t.meta_.initialXOffset ) {
        }
        inline double operator ()( uint32_t idx ) {
            return initialXOffset + ( idx * xIncrement );
        }
        inline int32_t bin( double t ) {
            if ( t >= 0 )
                return int32_t ( t / xIncrement + 0.5 );
            else
                return int32_t ( t / xIncrement - 0.5 );
        }
    };
}    
        

bool
threshold_result::operator >> ( adcontrols::TimeDigitalHistogram& x ) const
{
    index_to_time to_time( *data() );

    std::vector< std::pair< double, uint32_t > >& hgrm = x.histogram();
    
    if ( x.trigger_count() == 0 ) {
        
        x.initialXTimeSeconds() = this->data()->meta_.initialXTimeSeconds;
        x.initialXOffset()      = this->data()->meta_.initialXOffset;
        x.xIncrement()          = this->data()->meta_.xIncrement;
        x.actualPoints()        = this->data()->meta_.actualPoints;
        x.serialnumber()        = std::make_pair( this->data()->serialnumber_, this->data()->serialnumber_ );
        x.timeSinceEpoch()      = std::make_pair( this->data()->timeSinceEpoch_, this->data()->timeSinceEpoch_ );
        x.wellKnownEvents()     = this->data()->wellKnownEvents_;
        x.this_protocol()       = this->data()->method_.protocols() [ this->data()->method_.protocolIndex() ];
        x.setProtocolIndex( this->data()->method_.protocolIndex()
                            , uint32_t( this->data()->method_.protocols().size() ) );
    }

    x.trigger_count()++;
    
    if ( indecies_.empty() )
        return true;
    
    if ( hgrm.empty() ) {

        std::for_each( indecies().begin(), indecies().end(), [&] ( uint32_t idx ) {
                x.histogram().emplace_back( to_time( idx ), 1 );
            } );
        return true;        

    } else {

        for ( const auto& index: indecies_ ) {
            
            double t = to_time( index );
            auto it = std::lower_bound( hgrm.begin(), hgrm.end(), t, [&]( std::pair< double, uint32_t >& a, const double& b ) {
                    return to_time.bin( a.first ) < to_time.bin( b );
                });
            if ( to_time.bin( it->first ) == to_time.bin( t ) )
                it->second++;
            else
                hgrm.emplace( it, t, 1 );
        }
    }
    
    return true;
}

namespace acqrscontrols {
    namespace ap240 {

        std::ostream& operator << ( std::ostream& os, const threshold_result& t ) {

            os << boost::format( "\n%d, %.8lf, " ) % t.data()->serialnumber_ % t.data()->meta_.initialXTimeSeconds
                << t.data()->timeSinceEpoch_
                << boost::format( ", %.8e, %.8e" ) % t.data()->meta_.scaleFactor % t.data()->meta_.scaleOffset
                << boost::format( ", %.8e" ) % t.data()->meta_.initialXOffset;

            for ( auto& idx : t.indecies() ) {
                auto v = t.data()->xy( idx );
                os << boost::format( ", %.14le, %d" ) % v.first % v.second;
            }

            return os;
        }
    }
}
