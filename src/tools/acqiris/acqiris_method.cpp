/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#include "acqiris_method.hpp"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_serialize.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <adportable/portable_binary_iarchive.hpp>

namespace aqdrv4 {

    template< typename T = trigger_method >
    struct trigger_method_archive {
        template<class Archive>
        void serialize( Archive& ar, T& _, const unsigned int version ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( _.trigClass );
            ar & BOOST_SERIALIZATION_NVP( _.trigPattern );
            ar & BOOST_SERIALIZATION_NVP( _.trigSlope );
            ar & BOOST_SERIALIZATION_NVP( _.trigLevel1 );
            ar & BOOST_SERIALIZATION_NVP( _.trigLevel2 );
        }
    };

    template<> void trigger_method::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
    {
        trigger_method_archive<>().serialize( ar, *this, version );
    }
    template<> void trigger_method::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        trigger_method_archive<>().serialize( ar, *this, version );
    }    
    template<> void trigger_method::serialize( portable_binary_oarchive& ar, const unsigned int version )
    {
        trigger_method_archive<>().serialize( ar, *this, version );
    }
    template<> void trigger_method::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        trigger_method_archive<>().serialize( ar, *this, version );
    }    
    
    template< typename T = acqiris_method >
    struct acqiris_method_archive {
        template<class Archive>
        void serialize( Archive& ar, T& _, const unsigned int version ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( _.clsid_ );
            ar & BOOST_SERIALIZATION_NVP( _.trig_ );
            ar & BOOST_SERIALIZATION_NVP( _.hor_ );
            ar & BOOST_SERIALIZATION_NVP( _.ext_ );            
            ar & BOOST_SERIALIZATION_NVP( _.ch1_ );
            ar & BOOST_SERIALIZATION_NVP( _.ch2_ );
        }
    };

    template<> void acqiris_method::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
    {
        acqiris_method_archive<>().serialize( ar, *this, version );
    }
    template<> void acqiris_method::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        acqiris_method_archive<>().serialize( ar, *this, version );
    }    
    template<> void acqiris_method::serialize( portable_binary_oarchive& ar, const unsigned int version )
    {
        acqiris_method_archive<>().serialize( ar, *this, version );
    }
    template<> void acqiris_method::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        acqiris_method_archive<>().serialize( ar, *this, version );
    }

    template< typename T = horizontal_method >
    struct horizontal_method_archive {
        template<class Archive>
        void serialize( Archive& ar, T& _, const unsigned int version ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( _.sampInterval );
            ar & BOOST_SERIALIZATION_NVP( _.delayTime );
            ar & BOOST_SERIALIZATION_NVP( _.nbrSamples );
            ar & BOOST_SERIALIZATION_NVP( _.mode );
            ar & BOOST_SERIALIZATION_NVP( _.flags );
            ar & BOOST_SERIALIZATION_NVP( _.nbrAvgWaveforms );
        }
    };
    template<> void horizontal_method::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
    {
        horizontal_method_archive<>().serialize( ar, *this, version );
    }
    template<> void horizontal_method::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        horizontal_method_archive<>().serialize( ar, *this, version );
    }    
    template<> void horizontal_method::serialize( portable_binary_oarchive& ar, const unsigned int version )
    {
        horizontal_method_archive<>().serialize( ar, *this, version );
    }
    template<> void horizontal_method::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        horizontal_method_archive<>().serialize( ar, *this, version );
    }

    template< typename T = vertical_method >
    struct vertical_method_archive {
        template<class Archive>
        void serialize( Archive& ar, T& _, const unsigned int version ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( _.fullScale );
            ar & BOOST_SERIALIZATION_NVP( _.offset );
            ar & BOOST_SERIALIZATION_NVP( _.coupling );
            ar & BOOST_SERIALIZATION_NVP( _.invertData );
            ar & BOOST_SERIALIZATION_NVP( _.autoScale );
        }
    };
    template<> void vertical_method::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
    {
        vertical_method_archive<>().serialize( ar, *this, version );
    }
    template<> void vertical_method::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        vertical_method_archive<>().serialize( ar, *this, version );
    }    
    template<> void vertical_method::serialize( portable_binary_oarchive& ar, const unsigned int version )
    {
        vertical_method_archive<>().serialize( ar, *this, version );
    }
    template<> void vertical_method::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        vertical_method_archive<>().serialize( ar, *this, version );
    }
    
    
}

using namespace aqdrv4;

acqiris_method::acqiris_method()
{
}

acqiris_method::acqiris_method( const acqiris_method& t ) : clsid_( clsid() )
                                                          , trig_( t.trig_ )
                                                          , hor_( t.hor_ )
                                                          , ch1_( t.ch1_ )
                                                          , ch2_( t.ch2_ )
{
}

const boost::uuids::uuid&
acqiris_method::clsid()
{
    static auto __clsid = boost::uuids::string_generator()( "{A69B313C-007E-49C4-9E55-1D279A382D2A}" );
    return __clsid;
}

std::shared_ptr< trigger_method >
acqiris_method::mutable_trig()
{
    if ( ! trig_ )
        trig_ = std::make_shared< trigger_method >();
    return trig_;
}

std::shared_ptr< horizontal_method >
acqiris_method::mutable_hor()
{
    if ( ! hor_ )
        hor_ = std::make_shared< horizontal_method >();
    return hor_;
}

std::shared_ptr< vertical_method >
acqiris_method::mutable_ext()
{
    if ( ! ext_ )
        ext_ = std::make_shared< vertical_method >();
    return ext_;
}

std::shared_ptr< vertical_method >
acqiris_method::mutable_ch1()
{
    if ( ! ch1_ )
        ch1_ = std::make_shared< vertical_method >();
    return ch1_;
}

std::shared_ptr< vertical_method >
acqiris_method::mutable_ch2()
{
    if ( ! ch2_ )
        ch2_ = std::make_shared< vertical_method >();
    return ch2_;    
}

std::shared_ptr< const trigger_method >
acqiris_method::trig() const
{
    return trig_;
}

std::shared_ptr< const horizontal_method >
acqiris_method::hor() const
{
    return hor_;
}

std::shared_ptr< const vertical_method >
acqiris_method::ext() const
{
    return ext_;
}

std::shared_ptr< const vertical_method >
acqiris_method::ch1() const
{
    return ch1_;
}

std::shared_ptr< const vertical_method >
acqiris_method::ch2() const
{
    return ch2_;    
}


void
vertical_method::set_fullScale( double d )
{
    fullScale = d;
}

void
vertical_method::set_offset( double d )
{
    offset = d;
}

void
vertical_method::set_coupling( uint32_t d )
{
    coupling = d;
}

void
vertical_method::set_bandwidth( uint32_t d )
{
    bandwidth = d;
}

void
vertical_method::set_invertData( bool d )
{
    invertData = d;
}

#if defined USING_PROTOBUF
void
acqiris_method::set_defaults( acqiris::method * method )
{
    method->set_channels( 0x01 );
    set_defaults( method->mutable_trig() );
    set_defaults( method->mutable_hor() );
    set_defaults( method->add_ver() );
}

void
acqiris_method::set_defaults( acqiris::horizontal_method * hor )
{
    hor->set_sampinterval( 0.5e-9 );
    hor->set_delay( 0.0 );
    hor->set_width( 10.0e-6 );
    hor->set_mode( acqiris::Digitizer );
    hor->set_flags( 0 );
    hor->set_nstartdelay( 0 );
    hor->set_nbravgwaveforms( 1 );
    hor->set_nbrsamples( uint32_t( 10.0e-6 / 0.5e-9 + 0.5 ) );
}

void
acqiris_method::set_defaults( acqiris::vertical_method * ver )
{
    ver->set_fullscale( 5.0 );
    ver->set_offset( 0.0 );
    ver->set_coupling( 3 );
    ver->set_bandwidth( 2 );
    ver->set_invertdata( false );
    ver->set_autoscale( true );
}

void
acqiris_method::set_defaults( acqiris::trigger_method * trig )
{
    trig->set_trigclass( 1 );            // edge trigger
    trig->set_trigpattern( 0x80000000 ); // external
    trig->set_trigcoupling( 0 );          // DC
    trig->set_trigslope( 0 ); // positive
    trig->set_triglevel1( 1000.0 ); // mV for external, %fs for CHn
    trig->set_triglevel2( 0.0 );
}
#endif
