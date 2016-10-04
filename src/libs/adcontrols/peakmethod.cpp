// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#include "peakmethod.hpp"
#include "serializer.hpp"
#include <adportable/float.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/version.hpp>
#include <adportable/portable_binary_iarchive.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>

namespace adcontrols {

    template< typename T = PeakMethod::TimedEvent >
    class TimedEvent_archive {
    public:
        template<class Archive>
        void serialize(Archive& ar, T& _, const unsigned int version) {
            ar & BOOST_SERIALIZATION_NVP( _.time_ );
            ar & BOOST_SERIALIZATION_NVP( _.event_ );
            if ( version < 2 ) {
                double tmp(0);
                ar & BOOST_SERIALIZATION_NVP( tmp );
                _.value_ = tmp;
            } else {
                ar & BOOST_SERIALIZATION_NVP( _.value_ );
            }
        }
            
    };

    template<> void PeakMethod::TimedEvent::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
    {
        TimedEvent_archive<>().serialize( ar, *this, version );
    }
    template<> void PeakMethod::TimedEvent::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        TimedEvent_archive<>().serialize( ar, *this, version );
    }
    
    template<> void PeakMethod::TimedEvent::serialize( portable_binary_oarchive& ar, const unsigned int version )
    {
        TimedEvent_archive<>().serialize( ar, *this, version );
    }
    
    template<> void PeakMethod::TimedEvent::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        TimedEvent_archive<>().serialize( ar, *this, version );
    }

    /////////////////////
    template< typename T = PeakMethod >
    class PeakMethod_archive {
    public:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, T& _, const unsigned int version) {
            ar & BOOST_SERIALIZATION_NVP( _.minimumHeight_ );
            ar & BOOST_SERIALIZATION_NVP( _.minimumArea_ );
            ar & BOOST_SERIALIZATION_NVP( _.minimumWidth_ );
            ar & BOOST_SERIALIZATION_NVP( _.doubleWidthTime_ );
            ar & BOOST_SERIALIZATION_NVP( _.slope_ );
            ar & BOOST_SERIALIZATION_NVP( _.drift_ );
            ar & BOOST_SERIALIZATION_NVP( _.t0_ );
            ar & BOOST_SERIALIZATION_NVP( _.pharmacopoeia_ );
            ar & BOOST_SERIALIZATION_NVP( _.peakWidthMethod_ );
            ar & BOOST_SERIALIZATION_NVP( _.theoreticalPlateMethod_ );
                
            if ( version >= 2 ) {
                ar & BOOST_SERIALIZATION_NVP( _.noiseFilterMethod_);
                ar & BOOST_SERIALIZATION_NVP( _.cutoffFreqHz_ );
            }
            if ( version >= 3 ) {
                ar & BOOST_SERIALIZATION_NVP( _.timedEvents_ );
            }
        }
    };

    template<> void PeakMethod::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
    {
        try {
            PeakMethod_archive<>().serialize( ar, *this, version );
        } catch ( std::exception& ex ) {
            BOOST_THROW_EXCEPTION( serializer_error() << info( "serialize 'adcontrols::PeakMethod' to xml_woarchive" ) );
        }
    }
    template<> void PeakMethod::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        try {
            PeakMethod_archive<>().serialize( ar, *this, version );
        } catch ( std::exception& ex ) {
            BOOST_THROW_EXCEPTION( serializer_error() << info( "serialize 'adcontrols::PeakMethod' to xml_wiarchive" ) );
        }
    }
    
    template<> void PeakMethod::serialize( portable_binary_oarchive& ar, const unsigned int version )
    {
        try {
            PeakMethod_archive<>().serialize( ar, *this, version );
        } catch ( std::exception& ex ) {
            BOOST_THROW_EXCEPTION( serializer_error() << info( "serialize 'adcontrols::PeakMethod' to portable_binary_oarchive" ) );
        }
    }
    
    template<> void PeakMethod::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        try {
            PeakMethod_archive<>().serialize( ar, *this, version );
        } catch ( std::exception& ex ) {
            BOOST_THROW_EXCEPTION( serializer_error() << info( "serialize 'adcontrols::PeakMethod' to portable_binary_iarchive" ) );
        }
    }
}

using namespace adcontrols;
using namespace adcontrols::chromatography;

PeakMethod::~PeakMethod(void)
{
}

PeakMethod::PeakMethod(void) : minimumHeight_(10)
                             , minimumArea_(0)
							 , minimumWidth_( 0.01 ) // min := 0.6sec
                             , doubleWidthTime_( 0.0 ) // min
                             , slope_( 0.005 ) // uV/min
                             , drift_( 0.0 ) // uV/min
                             , t0_( 0 )
                             , pharmacopoeia_( ePHARMACOPOEIA_NotSpcified )
                             , peakWidthMethod_( ePeakWidth_HalfHeight )
                             , theoreticalPlateMethod_( ePeakWidth_HalfHeight )
{
}

PeakMethod::PeakMethod(const PeakMethod & t ) 
{
    operator=(t);
}

PeakMethod&
PeakMethod::operator = ( const PeakMethod & rhs )
{
    minimumHeight_          = rhs.minimumHeight_;
    minimumArea_            = rhs.minimumArea_;
    minimumWidth_           = rhs.minimumWidth_;
    doubleWidthTime_        = rhs.doubleWidthTime_;
    slope_                  = rhs.slope_;
    drift_                  = rhs.drift_;
    t0_                     = rhs.t0_;
    pharmacopoeia_          = rhs.pharmacopoeia_;
    peakWidthMethod_        = rhs.peakWidthMethod_;
    theoreticalPlateMethod_ = rhs.theoreticalPlateMethod_;
    timedEvents_            = rhs.timedEvents_;
    return * this;
}

bool
PeakMethod::operator == ( const PeakMethod & rhs ) const
{
    using adportable::compare;

    if ( compare<double>::approximatelyEqual( minimumHeight_, rhs.minimumHeight_ ) &&
         compare<double>::approximatelyEqual( minimumArea_, rhs.minimumArea_ ) &&
         compare<double>::approximatelyEqual( minimumWidth_, rhs.minimumWidth_ ) &&
         compare<double>::approximatelyEqual( doubleWidthTime_, rhs.doubleWidthTime_ ) &&
         compare<double>::approximatelyEqual( slope_, rhs.slope_ ) &&
         compare<double>::approximatelyEqual( drift_, rhs.drift_ ) &&
         compare<double>::approximatelyEqual( t0_, rhs.t0_ ) &&
         ( pharmacopoeia_ == rhs.pharmacopoeia_ ) &&
         ( peakWidthMethod_ == rhs.peakWidthMethod_ ) &&
         ( theoreticalPlateMethod_ == rhs.theoreticalPlateMethod_ ) ) {
        return true;
    }
    return false;
}

bool
PeakMethod::operator != ( const PeakMethod & rhs ) const
{
    return !(*this == rhs);
}


double
PeakMethod::minimumHeight() const
{
    return minimumHeight_;
}

void
PeakMethod::minimumHeight( double t )
{
    minimumHeight_ = t;
}

double
PeakMethod::minimumArea() const
{
    return minimumArea_;
}

void
PeakMethod::minimumArea( double t )
{
    minimumArea_ = t;
}

double
PeakMethod::minimumWidth() const
{
    return minimumWidth_;
}

void
PeakMethod::minimumWidth( double t )
{
    minimumWidth_ = t;
}

double
PeakMethod::doubleWidthTime() const
{
    return doubleWidthTime_;
}

void
PeakMethod::doubleWidthTime( double t )
{
    doubleWidthTime_ = t;
}

double
PeakMethod::slope() const
{
    return slope_;
}

void
PeakMethod::slope( double t )
{
    slope_ = t;
}

double
PeakMethod::drift() const
{
    return drift_;
}

void
PeakMethod::drift( double t )
{
    drift_ = t;
}

double
PeakMethod::t0() const
{
    return t0_;
}

void
PeakMethod::t0( double t )
{
    t0_ = t;
}

chromatography::ePharmacopoeia
PeakMethod::pharmacopoeia() const
{
    return pharmacopoeia_;
}

void
PeakMethod::pharmacopoeia( chromatography::ePharmacopoeia t )
{
    pharmacopoeia_ = t;
}

chromatography::ePeakWidthMethod
PeakMethod::peakWidthMethod() const
{
    return peakWidthMethod_;
}

void
PeakMethod::peakWidthMethod( chromatography::ePeakWidthMethod t )
{
    peakWidthMethod_ = t;
}

chromatography::ePeakWidthMethod
PeakMethod::theoreticalPlateMethod() const
{
    return theoreticalPlateMethod_;
}

void
PeakMethod::theoreticalPlateMethod( chromatography::ePeakWidthMethod t )
{
    theoreticalPlateMethod_ = t;
}

PeakMethod::size_type
PeakMethod::size() const
{
    return timedEvents_.size();
}

PeakMethod::iterator_type
PeakMethod::begin()
{
    return timedEvents_.begin();
}

PeakMethod::iterator_type
PeakMethod::end()
{
    return timedEvents_.end();
}

PeakMethod::const_iterator_type
PeakMethod::begin() const
{
    return timedEvents_.begin();
}

PeakMethod::const_iterator_type
PeakMethod::end() const
{
    return timedEvents_.end();
}

PeakMethod&
PeakMethod::operator << ( const PeakMethod::TimedEvent& t )
{
    timedEvents_.push_back( t );
    return *this;
}

PeakMethod::iterator_type
PeakMethod::erase( iterator_type pos )
{
    return timedEvents_.erase( pos );
}

PeakMethod::iterator_type
PeakMethod::erase( iterator_type first, iterator_type last )
{
    return timedEvents_.erase( first, last );
}

//----------------
PeakMethod::TimedEvent::~TimedEvent()
{
}

PeakMethod::TimedEvent::TimedEvent() : time_(0)
                                     , event_( chromatography::ePeakEvent_Nothing )
                                     , value_( false )
{
} 

PeakMethod::TimedEvent::TimedEvent( seconds_t t
                                    , chromatography::ePeakEvent e ) : time_( t )
                                                                     , event_( e )
{
    if ( isBool( e ) )
        value_ = bool( false );
    else
        value_ = double(0.0);
}

PeakMethod::TimedEvent::TimedEvent( const TimedEvent& t ) : time_( t.time_ )
                                                          , event_( t.event_ )
														  , value_( t.value_ )
{
}

//static
bool
PeakMethod::TimedEvent::isBool( adcontrols::chromatography::ePeakEvent t )
{
    switch( t ) {
    case ePeakEvent_Nothing:
    case ePeakEvent_Off:
    case ePeakEvent_ForcedBase:
    case ePeakEvent_ShiftBase:
    case ePeakEvent_VtoV:
    case ePeakEvent_Tailing:
    case ePeakEvent_Leading:
    case ePeakEvent_Shoulder:
    case ePeakEvent_NegativePeak:
    case ePeakEvent_NegativeLock:
    case ePeakEvent_HorizontalBase:
    case ePeakEvent_PostHorizontalBase:
    case ePeakEvent_ForcedPeak:
    case ePeakEvent_Elimination:
    case ePeakEvent_Manual:
        return true;
    case ePeakEvent_Slope:
    case ePeakEvent_MinWidth:
    case ePeakEvent_MinHeight:
    case ePeakEvent_MinArea:
    case ePeakEvent_Drift:
       return false;
    }
    return false;
}

//static
bool
PeakMethod::TimedEvent::isDouble( adcontrols::chromatography::ePeakEvent t )
{
    return !isBool( t );
}
  
double
PeakMethod::TimedEvent::time( bool asMinutes ) const
{
    if ( asMinutes )
        return timeutil::toMinutes( time_ );
    else
        return time_;
}

void
PeakMethod::TimedEvent::setTime( double time, bool isMinutes )
{
    if ( isMinutes )
        time_ = timeutil::toSeconds( time );
    else
        time_ = time;
}

chromatography::ePeakEvent
PeakMethod::TimedEvent::peakEvent() const
{
    return event_;
}

void
PeakMethod::TimedEvent::setPeakEvent( chromatography::ePeakEvent t )
{
    event_ = t;
}

bool
PeakMethod::TimedEvent::isBool() const
{
	return value_.type() == typeid( bool );
}

bool
PeakMethod::TimedEvent::isDouble() const
{
	return value_.type() == typeid( double );
}

double
PeakMethod::TimedEvent::doubleValue() const
{
	return boost::get<double>( value_ ); // may raise bad_cast exception
}

bool
PeakMethod::TimedEvent::boolValue() const
{
	return boost::get<bool>( value_ ); // may raise bad_cast exception
}

void
PeakMethod::TimedEvent::setValue( bool value )
{
    value_ = value;
}

void
PeakMethod::TimedEvent::setValue( double value )
{
    value_ = value;
}

void
PeakMethod::sort()
{
    std::sort( timedEvents_.begin(), timedEvents_.end(), [] ( const TimedEvent& a, const TimedEvent& b ) {
            return a.time() < b.time();
        });
}
