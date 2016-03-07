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
#include <adportable/portable_binary_iarchive.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>

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

