/**************************************************************************
** Copyright (C) 2015-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2015-2016 MS-Cheminformatics LLC
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

#include "timedigitalhistogram.hpp"
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <adportable/portable_binary_iarchive.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>

namespace adcontrols {

    template< typename T = TimeDigitalHistogram >
    class TimeDigitalHistogram_archive {
    public:
        template<class Archive>
        void serialize( Archive& ar, T& _, const unsigned int version ) {
            ar & BOOST_SERIALIZATION_NVP( _.initialXTimeSeconds_ );
            ar & BOOST_SERIALIZATION_NVP( _.initialXOffset_ );
            ar & BOOST_SERIALIZATION_NVP( _.xIncrement_ );
            ar & BOOST_SERIALIZATION_NVP( _.actualPoints_ );
            ar & BOOST_SERIALIZATION_NVP( _.trigger_count_ );
            ar & BOOST_SERIALIZATION_NVP( _.serialnumber_ );
            ar & BOOST_SERIALIZATION_NVP( _.timeSinceEpoch_ );
            ar & BOOST_SERIALIZATION_NVP( _.histogram_ );
        }
    };
    
    ///////// Portable binary archive ////////    
    template<> void
    TimeDigitalHistogram::serialize( portable_binary_oarchive& ar, const unsigned int version ) {
        TimeDigitalHistogram_archive<>().serialize( ar, *this, version );
    }

    template<> void
    TimeDigitalHistogram::serialize( portable_binary_iarchive& ar, const unsigned int version ) {
        TimeDigitalHistogram_archive<>().serialize( ar, *this, version );
    }
    
    ///////// XML archive ////////
    template<> void
    TimeDigitalHistogram::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
    {
        TimeDigitalHistogram_archive<>().serialize( ar, *this, version );
    }

    template<> void
    TimeDigitalHistogram::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        TimeDigitalHistogram_archive<>().serialize( ar, *this, version );        
    }
}

using namespace adcontrols;

TimeDigitalHistogram::TimeDigitalHistogram() : initialXTimeSeconds_( 0 )
                                             , initialXOffset_( 0 )
                                             , xIncrement_( 0 )
                                             , trigger_count_( 0 )
                                             , actualPoints_( 0 )
                                             , serialnumber_( { 0, 0 } )
                                             , timeSinceEpoch_( { 0, 0 } )
{
}

TimeDigitalHistogram::TimeDigitalHistogram( const TimeDigitalHistogram& t ) : initialXTimeSeconds_( t.initialXTimeSeconds_ )
                                                                            , initialXOffset_( t.initialXOffset_ )
                                                                            , xIncrement_( t.xIncrement_ )
                                                                            , actualPoints_( t.actualPoints_ )
                                                                            , serialnumber_( t.serialnumber_ )
                                                                            , timeSinceEpoch_( t.timeSinceEpoch_ )
                                                                            , trigger_count_( t.trigger_count_ )
                                                                            , histogram_( t.histogram_ )
{
}

double&
TimeDigitalHistogram::initialXTimeSeconds()
{
    return initialXTimeSeconds_;
}

double&
TimeDigitalHistogram::initialXOffset()
{
    return initialXOffset_;
}

double&
TimeDigitalHistogram::xIncrement()
{
    return xIncrement_;
}

uint64_t&
TimeDigitalHistogram::actualPoints()
{
    return actualPoints_;
}

uint64_t&
TimeDigitalHistogram::trigger_count()
{
    return trigger_count_;
}

double
TimeDigitalHistogram::initialXTimeSeconds() const
{
    return initialXTimeSeconds_;
}

double
TimeDigitalHistogram::initialXOffset() const
{
    return initialXOffset_;
}

double
TimeDigitalHistogram::xIncrement() const
{
    return xIncrement_;
}

uint64_t
TimeDigitalHistogram::actualPoints() const
{
    return actualPoints_;
}

uint64_t
TimeDigitalHistogram::trigger_count() const
{
    return trigger_count_;
}
