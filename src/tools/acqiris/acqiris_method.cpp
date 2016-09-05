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
            ar & BOOST_SERIALIZATION_NVP( _.mode );
            ar & BOOST_SERIALIZATION_NVP( _.trig );
            ar & BOOST_SERIALIZATION_NVP( _.hor );
            ar & BOOST_SERIALIZATION_NVP( _.ch1 );
            ar & BOOST_SERIALIZATION_NVP( _.ch2 );
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
            ar & BOOST_SERIALIZATION_NVP( _.mode );
            ar & BOOST_SERIALIZATION_NVP( _.flags );
            ar & BOOST_SERIALIZATION_NVP( _.nbrAvgWaveforms );
            ar & BOOST_SERIALIZATION_NVP( _.nStartDelay );
            ar & BOOST_SERIALIZATION_NVP( _.nbrSamples );
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
                                                          , mode( t.mode )
                                                          , trig( t.trig )
                                                          , hor( t.hor )
                                                          , ch1( t.ch1 )
                                                          , ch2( t.ch2 )
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
    if ( ! trig )
        trig = std::make_shared< trigger_method >();
    return trig;
}

std::shared_ptr< horizontal_method >
acqiris_method::mutable_hor()
{
    if ( ! hor )
        hor = std::make_shared< horizontal_method >();
    return hor;
}

std::shared_ptr< vertical_method >
acqiris_method::mutable_ch1()
{
    if ( ! ch1 )
        ch1 = std::make_shared< vertical_method >();
    return ch1;
}

std::shared_ptr< vertical_method >
acqiris_method::mutable_ch2()
{
    if ( ! ch2 )
        ch2 = std::make_shared< vertical_method >();
    return ch2;    
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
