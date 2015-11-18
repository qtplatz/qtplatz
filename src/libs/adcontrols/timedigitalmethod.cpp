// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
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

#include "timedigitalmethod.hpp"

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>
#include <adportable/portable_binary_iarchive.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/archive/archive_exception.hpp>

namespace adcontrols {

    template<typename T = TimeDigitalMethod>
    class TimeDigitalMethod_archive {
        public:
        template<class Archive>
        void serialize( Archive& ar, T& _, const unsigned int version ) {
            
            using namespace boost::serialization;
            
            ar & BOOST_SERIALIZATION_NVP( _.action_ );
            ar & BOOST_SERIALIZATION_NVP( _.thresholds_ );

        }
    };

    template<> ADCONTROLSSHARED_EXPORT void TimeDigitalMethod::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
    {
        TimeDigitalMethod_archive<>().serialize( ar, *this, version );
    }
    
    template<> ADCONTROLSSHARED_EXPORT void TimeDigitalMethod::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        TimeDigitalMethod_archive<>().serialize( ar, *this, version );
    }
    
    template<> ADCONTROLSSHARED_EXPORT void TimeDigitalMethod::serialize( portable_binary_oarchive& ar, const unsigned int version )
    {
        TimeDigitalMethod_archive<>().serialize( ar, *this, version );
    }
    
    template<> ADCONTROLSSHARED_EXPORT void TimeDigitalMethod::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        TimeDigitalMethod_archive<>().serialize( ar, *this, version );
    }
    
    bool TimeDigitalMethod::archive( std::ostream& os, const TimeDigitalMethod& t )
    {
        try {
            portable_binary_oarchive ar( os );
            ar & boost::serialization::make_nvp( "m", t );
            return true;
        } catch ( std::exception& ex ) {
            BOOST_THROW_EXCEPTION( ex );
        }
        return false;
    }
    
    bool TimeDigitalMethod::restore( std::istream& is, TimeDigitalMethod& t )
    {
        try {
            portable_binary_iarchive ar( is );
            ar & boost::serialization::make_nvp( "m", t );
            return true;
        } catch ( std::exception& ex ) {
            BOOST_THROW_EXCEPTION( ex );
        }
        return false;
    }
    
    bool TimeDigitalMethod::xml_archive( std::wostream& os, const TimeDigitalMethod& t )
    {
        try {
            boost::archive::xml_woarchive ar( os );
            ar & boost::serialization::make_nvp( "m", t );
            return true;
        } catch ( std::exception& ex ) {
            BOOST_THROW_EXCEPTION( ex );
        }
        return false;
    }
    
    bool TimeDigitalMethod::xml_restore( std::wistream& is, TimeDigitalMethod& t )
    {
        try {
            boost::archive::xml_wiarchive ar( is );
            ar & boost::serialization::make_nvp( "m", t );
            return true;
        } catch ( std::exception& ex ) {
            BOOST_THROW_EXCEPTION( ex );
        }
        return false;
        
    }
}

using namespace adcontrols;

TimeDigitalMethod::~TimeDigitalMethod()
{
}

TimeDigitalMethod::TimeDigitalMethod()
{
    thresholds_.push_back( threshold_method() ); // at least one
}

TimeDigitalMethod::TimeDigitalMethod( const TimeDigitalMethod& t ) : action_( t.action_ )
                                                                   , thresholds_( t.thresholds_ )
{
}

std::vector< threshold_method >&
TimeDigitalMethod::thresholds()
{
    return thresholds_;
}

const std::vector< threshold_method >&
TimeDigitalMethod::thresholds() const
{
    return thresholds_;
}

const threshold_method&
TimeDigitalMethod::threshold( size_t ch ) const
{
    if ( ch < thresholds_.size() )
        return thresholds_[ ch ];

    static const threshold_method __empty__;
    return __empty__;
}

void
TimeDigitalMethod::setThreshold( size_t ch, const threshold_method& t )
{
    if ( ch < 2 && thresholds_.size() < ch )
        thresholds_.resize( ch + 1 );
    thresholds_[ ch ] = t;
}

threshold_action&
TimeDigitalMethod::action()
{
    return action_;
}

const threshold_action&
TimeDigitalMethod::action() const
{
    return action_;    
}

