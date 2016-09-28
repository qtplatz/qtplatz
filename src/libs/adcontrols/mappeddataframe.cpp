/**************************************************************************
** Copyright (C) 2014-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2014-2016 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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

#include "mappeddataframe.hpp"
#include <adportable/graycode.hpp>
#include <adcontrols/metric/prefix.hpp>

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <adcontrols/mappedspectrum.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>
#include <adportable/portable_binary_iarchive.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <mutex>
#include <fstream>

namespace adcontrols {

    ////////// PORTABLE BINARY ARCHIVE //////////
    template<> ADCONTROLSSHARED_EXPORT void 
    MappedDataFrame::serialize( portable_binary_oarchive& ar, const unsigned int )
    {
        ar & data_;
    }

    template<> ADCONTROLSSHARED_EXPORT void
    MappedDataFrame::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        ar & data_;
    }

    ///////// XML archive ////////
    template<> void
    MappedDataFrame::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
    {
        ar & BOOST_SERIALIZATION_NVP(data_);
    }

    template<> void
    MappedDataFrame::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        ar & BOOST_SERIALIZATION_NVP(data_);
    }
}


using namespace adcontrols;

MappedDataFrame::~MappedDataFrame()
{
}

MappedDataFrame::MappedDataFrame() : dataReaderUuid_( { 0 } )
                                   , data_( 0, 0 )
                                   , rowId_( 0 )
                                   , timeSinceEpoch_( 0 )
                                   , trig_timepoint_( 0 )
                                   , trig_number_( 0 )
                                   , trig_delay_( 0 )
                                   , sampInterval_( 0 )
                                   , wellKnownEvents_( 0 )
                                   , clock_count_( 50000000 )
                                   , ident_( nullptr )
{
}

MappedDataFrame::MappedDataFrame( const MappedDataFrame& t ) : dataReaderUuid_( t.dataReaderUuid_ )
                                                             , data_( t.data_ )
                                                             , rowId_( t.rowId_ )
                                                             , timeSinceEpoch_( t.timeSinceEpoch_ )
                                                             , trig_timepoint_( t.trig_timepoint_ )
                                                             , trig_number_( t.trig_number_ )
                                                             , trig_delay_( t.trig_delay_ )
                                                             , sampInterval_( t.sampInterval_ )
                                                             , wellKnownEvents_( t.wellKnownEvents_ )
                                                             , clock_count_( t.clock_count_ )
                                                             , ident_( t.ident_ )
{
}

size_t
MappedDataFrame::size1() const
{
    return data_.size1();
}

size_t
MappedDataFrame::size2() const
{
    return data_.size2();
}

uint16_t&
MappedDataFrame::operator ()( size_t i, size_t j )
{
    return data_( i, j );
}

const uint16_t&
MappedDataFrame::operator ()( size_t i, size_t j ) const
{
    return data_( i, j );
}

double
MappedDataFrame::time( uint16_t binary, uint32_t delay, double clock )
{
    return double( binary + delay ) / clock;
}


uint64_t&
MappedDataFrame::timeSinceEpoch()
{
    return timeSinceEpoch_;
}

uint64_t
MappedDataFrame::timeSinceEpoch() const
{
    return timeSinceEpoch_;
}

double
MappedDataFrame::trig_delay() const
{
    return trig_delay_;
}

double&
MappedDataFrame::trig_delay()
{
    return trig_delay_;
}

uint32_t
MappedDataFrame::trig_number() const
{
    return trig_number_;
}

uint32_t&
MappedDataFrame::trig_number()
{
    return trig_number_;
}

uint64_t
MappedDataFrame::trig_timepoint() const
{
    return trig_timepoint_;
}

uint64_t&
MappedDataFrame::trig_timepoint()
{
    return trig_timepoint_;
}

MappedDataFrame::operator const boost::numeric::ublas::matrix< uint16_t >& () const
{
    return data_;
}

const boost::numeric::ublas::matrix< uint16_t >&
MappedDataFrame::matrix() const
{
    return data_;
}

boost::numeric::ublas::matrix< uint16_t >&
MappedDataFrame::matrix()
{
    return data_;
}

bool
MappedDataFrame::empty() const
{
    for ( size_t i = 0; i < data_.size1(); ++i ) {
		for ( size_t j = 0; j < data_.size2(); ++j ) {
            if ( data_( i, j ) )
                return false;
        }
    }
    return true;
}

void
MappedDataFrame::setDataReaderUuid( const boost::uuids::uuid& uuid )
{
    dataReaderUuid_ = uuid;
}

int64_t
MappedDataFrame::rowid() const
{
    return rowId_;
}

int64_t&
MappedDataFrame::rowid()
{
    return rowId_;
}

uint32_t
MappedDataFrame::numSamples() const
{
    return numSamples_;
}

uint32_t&
MappedDataFrame::numSamples()
{
    return numSamples_;
}

double
MappedDataFrame::samplingInterval() const
{
    return sampInterval_;
}

double&
MappedDataFrame::samplingInterval()
{
    return sampInterval_;    
}

uint32_t
MappedDataFrame::coaddSpectrum( adcontrols::MappedSpectrum& sp, size_t x0, size_t y0, size_t w, size_t h ) const
{
    uint32_t tic( 0 );
    
    if ( sp.timeSinceEpoch().first == 0 ) {
        double clockTime = 1.0 / double( clock_count_ ); // 20.0e-9; // if 50MHz
        double sampInterval = clockTime / 16; // 4bit TMC
        
        // Need change if not 50MHz
        sp.setSamplingInfo( sampInterval, trig_delay_, 4096 );
        sp.timeSinceEpoch().second = timeSinceEpoch_;
        sp.setTrigNumber( trig_number(), trig_number() );
    }
    sp.timeSinceEpoch().first = timeSinceEpoch_;
    sp.setTrigNumber( trig_number() );

    for ( size_t i = x0; i < data_.size1() && i < x0 + w; ++i ) {

		for ( size_t j = y0; j < data_.size2() && j < y0 + h; ++j ) {

            if ( auto binary = data_( i, j ) ) {
                double tof = time( binary, trig_delay_, clock_count_ );
                sp << std::make_pair( tof, 1 );
                tic++;
            }
        }
    }
    return tic;
}

bool
MappedDataFrame::transform( adcontrols::MappedSpectrum& sp, size_t x0, size_t y0, size_t w, size_t h ) const
{
    double clockTime = 1.0 / double( clock_count_ ); // 20.0e-9; // if 50MHz
    double sampInterval = clockTime / 16; // 4bit TMC
    
    // Need change if not 50MHz
    sp.setSamplingInfo( sampInterval, trig_delay_, 4096 );
    sp.timeSinceEpoch() = std::make_pair( timeSinceEpoch(), timeSinceEpoch() );
    sp.setTrigNumber( trig_number(), trig_number() );

    coaddSpectrum( sp, x0, y0, w, h );

    return true;
}

