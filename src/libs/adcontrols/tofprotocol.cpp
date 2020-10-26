/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com or info@ms-cheminfo.com
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

#include "tofprotocol.hpp"
#include <adcontrols/metric/prefix.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/nvp.hpp>
#include <adportable_serializer/portable_binary_oarchive.hpp>
#include <adportable_serializer/portable_binary_iarchive.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/property_tree/ptree.hpp>

namespace adcontrols {

    //////////////////////////////////////////
    template<typename T = TofProtocol >
    class TofProtocol_archive {
    public:
        template<class Archive>
        void serialize( Archive& ar, T& _, const unsigned int version ) {
            using namespace boost::serialization;
                ar & BOOST_SERIALIZATION_NVP( _.lower_mass_ );
                ar & BOOST_SERIALIZATION_NVP( _.upper_mass_ );
                ar & BOOST_SERIALIZATION_NVP( _.number_of_triggers_ );
                if ( version >= 1 )
                    ar & BOOST_SERIALIZATION_NVP( _.mode_ );
                if ( version >= 2 )
                    ar & BOOST_SERIALIZATION_NVP( _.digitizer_delay_width_ );
                ar & BOOST_SERIALIZATION_NVP( _.delay_pulses_ );
                ar & BOOST_SERIALIZATION_NVP( _.additionals_ );
                ar & BOOST_SERIALIZATION_NVP( _.reference_ );
                ar & BOOST_SERIALIZATION_NVP( _.formulae_ );
                ar & BOOST_SERIALIZATION_NVP( _.devicedata_ );
        }
    };

    template<> ADCONTROLSSHARED_EXPORT void TofProtocol::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
    {
        TofProtocol_archive<>().serialize( ar, *this, version );
    }
    template<> ADCONTROLSSHARED_EXPORT void TofProtocol::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        TofProtocol_archive<>().serialize( ar, *this, version );
    }

    template<> ADCONTROLSSHARED_EXPORT void TofProtocol::serialize( portable_binary_oarchive& ar, const unsigned int version )
    {
        TofProtocol_archive<>().serialize( ar, *this, version );
    }

    template<> ADCONTROLSSHARED_EXPORT void TofProtocol::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        TofProtocol_archive<>().serialize( ar, *this, version );
    }

}

using namespace adcontrols;

TofProtocol::TofProtocol() : lower_mass_( 0 )
                           , upper_mass_( 0 )
                           , mode_( 0 )
                           , number_of_triggers_( 0 )
                           , digitizer_delay_width_( { 0, 0 })
                           , delay_pulses_( { { 0,0 }   // push
                                   , { 0,0 }            // inject
                                   , { 0,0 }            // exit
                                   , { 0,0 }, { 0,0 }   // gate0, gate1
                                   , { 0,0 } }  )       // ext. trig delay (helio -> digitizer trig.)
{
}

TofProtocol::TofProtocol( const TofProtocol& t ) : lower_mass_( t.lower_mass_ )
                                                 , upper_mass_( t.upper_mass_ )
                                                 , mode_( t.mode_ )
                                                 , number_of_triggers_( t.number_of_triggers_ )
                                                 , digitizer_delay_width_( t.digitizer_delay_width_ )
                                                 , delay_pulses_( t.delay_pulses_ )
                                                 , additionals_( t.additionals_ )
                                                 , reference_( t.reference_ )
                                                 , formulae_( t.formulae_ )
                                                 , devicedata_( t.devicedata_ )
{
}

std::vector< TofProtocol::delay_pulse_type >&
TofProtocol::delay_pulses()
{
    return delay_pulses_;
}

const std::vector< TofProtocol::delay_pulse_type >&
TofProtocol::delay_pulses() const
{
    return delay_pulses_;
}

void
TofProtocol::set_delay_pulses( std::vector< delay_pulse_type >&& t )
{
    delay_pulses_ = t;
}

void
TofProtocol::setDevicedata( const std::string& value )
{
    devicedata_ = value;
}

const std::string&
TofProtocol::devicedata() const
{
    return devicedata_;
}

std::vector< std::string> &
TofProtocol::formulae()
{
    return formulae_;
}

const std::vector< std::string> &
TofProtocol::formulae() const
{
    return formulae_;
}

std::vector< std::pair< int32_t, TofProtocol::additional_value_type > >&
TofProtocol::additionals()
{
    return additionals_;
}

const std::vector< std::pair< int32_t, TofProtocol::additional_value_type > >&
TofProtocol::additionals() const
{
    return additionals_;
}

void
TofProtocol::setReference( uint32_t reference )
{
    reference_ = reference;
}

uint32_t
TofProtocol::reference() const
{
    return reference_;
}

uint32_t
TofProtocol::number_of_triggers() const
{
    return number_of_triggers_;
}

void
TofProtocol::setNumber_of_triggers( uint32_t value )
{
    number_of_triggers_ = value;
}

uint32_t
TofProtocol::mode() const
{
    return mode_;
}

void
TofProtocol::setMode( uint32_t value )
{
    mode_ = value;
}

void
TofProtocol::setDigitizerDelayWidth( std::pair< double, double >&& pair )
{
    digitizer_delay_width_ = pair;
}

const std::pair<double, double>&
TofProtocol::digitizerDelayWidth() const
{
    return digitizer_delay_width_;
}

///////////////////
// static
boost::optional< TofProtocol >
TofProtocol::fromJson( const boost::property_tree::ptree& pt )
{
    TofProtocol atof;

    auto avgr_delay = pt.get_optional< double >( "avgr_delay" );
    auto avgr_duration = pt.get_optional< double >( "avgr_duration" );
    if ( avgr_delay && avgr_duration )
        atof.setDigitizerDelayWidth( { *avgr_delay, *avgr_duration } );

    auto formulae = pt.get_optional< std::string >( "formulae" );
    auto mode = pt.get_optional< int >( "mode" );
    auto navg = pt.get_optional< int >( "number_of_triggers" );
    if ( !( formulae && navg && mode ) )
        return boost::none;

    atof.setNumber_of_triggers( *navg );
    atof.setMode( *mode );
    atof.formulae().emplace_back( *formulae );
    if ( auto ref = pt.get_optional< int >( "reference" ) )
        atof.setReference( *ref );

    if ( auto pulses = pt.get_child_optional( "pulses" ) ) {
        std::vector< TofProtocol::delay_pulse_type > delay_pulses;
        for ( auto pulse: *pulses ) {
            if ( auto enable = pulse.second.get_optional< bool >( "enable" ) )
                delay_pulses.emplace_back(
                    pulse.second.get< double >( "delay" )
                    , *enable ? pulse.second.get< double >( "width" ) : 0.0 );
            else
                delay_pulses.emplace_back(
                    pulse.second.get< double >( "delay" )
                    , pulse.second.get< double >( "width" ) );
        }
        atof.set_delay_pulses( std::move( delay_pulses ) );
    }
    return atof;
}

boost::property_tree::ptree
TofProtocol::toJson( int index ) const
{
    boost::property_tree::ptree pt, pulses;

    int id(0);
    for ( auto& p: delay_pulses_ ) {
        boost::property_tree::ptree pulse;
        pulse.put( "delay", p.first );
        pulse.put( "width", p.second );
        pulse.put( "id", id++ );
        pulses.push_back( { "", pulse } );
    }

    pt.put( "index", index );

    pt.put( "lower_mass", lower_mass_ );
    pt.put( "upper_mass", upper_mass_ );
    pt.put( "mode", mode_ );
    pt.put( "number_of_triggers", number_of_triggers_ );
    pt.put( "avgr_delay",  digitizer_delay_width_.first );
    pt.put( "avgr_duration",  digitizer_delay_width_.second );
    pt.add_child( "pulses", pulses );
    pt.put( "reference", reference_ );
    //
    std::string formulae;
    std::for_each( formulae_.begin(), formulae_.end(),  [&]( const auto& formula ) {
        formulae += formulae.empty() ? formula : (std::string(";") + formula);
    });

    if ( ! formulae.empty() )
        pt.put( "formulae", formulae );
    // todo: device_data
    return pt;
}
