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
#include <adportable/counting/threshold_finder.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <ratio>

using namespace acqrscontrols::u5303a;

threshold_result::threshold_result() : foundIndex_( npos )
                                     , findRange_( 0, 0 )
                                     , findUp_( false )
{
}

threshold_result::threshold_result( std::shared_ptr< const waveform > d ) : data_( d )
                                                                          , foundIndex_( npos )
                                                                          , findRange_( 0, 0 )
                                                                          , findUp_( false )
{
}

threshold_result::threshold_result( const threshold_result& t ) : adportable::counting::counting_result( t )
                                                                , data_( t.data_ )
                                                                , indecies_( t.indecies_ )
                                                                , processed_( t.processed_ )
                                                                , foundIndex_( t.foundIndex_ )
                                                                , findRange_( t.findRange_ )
                                                                , findUp_( t.findUp_ )
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

const std::vector< uint32_t >&
threshold_result::indecies() const
{
    return indecies_;
}

// std::vector< adportable::counting::threshold_index >&
// threshold_result::indecies2()
// {
//     return indecies2_;
// }

// const std::vector< adportable::counting::threshold_index >&
// threshold_result::indecies2() const
// {
//     return indecies2_;
// }

std::vector< double >&
threshold_result::processed()
{
    return processed_;
}

const std::vector< double >&
threshold_result::processed() const
{
    return processed_;
}

std::shared_ptr< const waveform >
threshold_result::data() const
{
    return data_;
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
    //
    // see threshold_result_accessor.cpp in infitof/plugins/infitof2
    //
    auto data = std::make_shared< acqrscontrols::u5303a::waveform >();

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

void
threshold_result::setFindUp( bool f )
{
    findUp_ = f;
}

bool
threshold_result::findUp() const
{
    return findUp_;
}

// call from infitof2 - counting
void
threshold_result::write3( std::ostream& os, const threshold_result& t )
{
    if ( os.tellp() == std::streamoff(0) )
        os << "## trig#, prot#, timestamp(s), epoch_time(ns), events, threshold(mV), algo(0=absolute,1=average,2=deferential)"
            "\t[time(s), peak-front(s), peak-front(mV), peak-end(s), peak-end(mV)]";
    
    if ( auto data = t.data() ) {
        
        os << boost::format( "\n%d, %d, %.8lf, %.8lf, 0x%08x, %.8lf, %d" )
            % data->serialnumber_
            % data->method_.protocolIndex()
            % data->meta_.initialXTimeSeconds
            % t.data()->timeSinceEpoch_
            % t.data()->wellKnownEvents_
            % ( t.threshold_level() * std::milli::den )
            % t.algo();
        
        if ( ! t.indecies2().empty() ) {
            for ( auto& idx : t.indecies2() ) {
                
                auto apex  = data->xy( idx.apex );

                os << boost::format( ",\t%.14le, %.6f, %d, %.6f, %d, %.6f" )
                    % apex.first    // apex (time)
                    % ( t.data()->toVolts( apex.second ) * std::milli::den )
                    % ( int( idx.first ) - int( idx.apex ) )   // front-apex distance
                    % ( t.data()->toVolts( (*data)[ idx.first ] ) * std::milli::den )     // front intensity
                    % ( int( idx.second ) - int( idx.apex ) )
                    % ( t.data()->toVolts( (*data)[ idx.second ] ) * std::milli::den );
            }
        }
    }
}

namespace acqrscontrols {
    namespace u5303a {

        // call from u5303aplugin
        std::ostream& operator << ( std::ostream& os, const threshold_result& t ) {

            if ( os.tellp() == std::streamoff(0) )
                os << "## trig#, time-stamp(s), time(s), epoch time(ns), events, scale factor, scale offset, delay time(s),"
                    "[time(s), idx0, idx1, idx2, value]";
            
            if ( auto data = t.data() ) {

                os << boost::format( "\n%d, %.8lf, %.8lf, " )
                    % data->serialnumber_ % data->meta_.initialXTimeSeconds % t.data()->timeSinceInject_
                   << t.data()->timeSinceEpoch_
                   << boost::format( ", 0x%08x" ) % t.data()->wellKnownEvents_
                   << boost::format( ", %.8e, %.8e" ) % data->meta_.scaleFactor % data->meta_.scaleOffset
                   << boost::format( ", %.8e" ) % data->meta_.initialXOffset;
                
                if ( ! t.indecies2().empty() ) {
                    for ( auto& idx : t.indecies2() ) {
                        auto v = data->xy( idx.first );
                        os << boost::format( ",\t%.14le, %d, %d, %d, %.6f" )
                            % v.first % idx.first % idx.second % idx.apex
                            % t.data()->toVolts( idx.value );
                    }
                    
                } else {
                    for ( auto& idx : t.indecies() ) {
                        auto v = data->xy( idx );
                        os << boost::format( ", %.14le, %d" ) % v.first % v.second;
                    }
                }
            }
            return os;
        }
    }
}

