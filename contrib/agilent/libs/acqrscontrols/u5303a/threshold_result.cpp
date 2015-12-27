/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include <boost/format.hpp>
#include <adportable/portable_binary_iarchive.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>

using namespace acqrscontrols::u5303a;

threshold_result::threshold_result() : foundIndex_( npos )
                                     , findRange_( 0, 0 )
{
}

threshold_result::threshold_result( std::shared_ptr< const waveform > d ) : data_( d )
                                                                          , foundIndex_( npos )
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

std::shared_ptr< const waveform >&
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

std::shared_ptr< const waveform >
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

size_t
threshold_result::serialize_xmeta( std::string& os ) const
{
    boost::iostreams::back_insert_device< std::string > inserter( os );
    boost::iostreams::stream< boost::iostreams::back_insert_device< std::string > > device( inserter );

    portable_binary_oarchive ar( device );
    ar & data_->meta_;

    return os.size();
}

size_t
threshold_result::serialize_xdata( std::string& device ) const
{
    device.resize( ( indecies_.size() * sizeof(uint32_t) + ( 2 * sizeof(uint32_t) ) ) );
    
    uint32_t * dest_p = reinterpret_cast<uint32_t *>( const_cast< char * >( device.data() ) );
    *dest_p++ = 0x7ffe0001; // separater & endian marker
    *dest_p++ = uint32_t( indecies_.size() );
    
    std::copy( indecies_.begin(), indecies_.end(), dest_p );
    return device.size();
}

namespace acqrscontrols {
    namespace u5303a {

        std::ostream& operator << ( std::ostream& os, const threshold_result& t ) {

            if ( auto data = t.data() ) {
                os << boost::format( "\n%d, %.8lf, " ) % data->serialnumber_ % data->meta_.initialXTimeSeconds
                    << t.data()->timeSinceEpoch_
                    << boost::format( ", %.8e, %.8e" ) % data->meta_.scaleFactor % data->meta_.scaleOffset
                    << boost::format( ", %.8e" ) % data->meta_.initialXOffset;

                for ( auto& idx : t.indecies() ) {
                    auto v = data->xy( idx );
                    os << boost::format( ", %.14le, %d" ) % v.first % v.second;
                }
            }
            return os;
        }
    }
}
