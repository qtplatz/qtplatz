/**************************************************************************
** Copyright (C) 2010-2018 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2018 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <adportable/portable_binary_iarchive.hpp>

namespace acqrscontrols {
namespace aqdrv4 {

    template< typename T = trigger_method >
    struct trigger_method_archive {
        template<class Archive>
        void serialize( Archive& ar, T& _, const unsigned int version ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( _.trigClass );
            ar & BOOST_SERIALIZATION_NVP( _.trigPattern );
            ar & BOOST_SERIALIZATION_NVP( _.trigCoupling );
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
            ar & BOOST_SERIALIZATION_NVP( _.methodNumber_ );
        }
    };

    template<> DECL_EXPORT void acqiris_method::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
    {
        acqiris_method_archive<>().serialize( ar, *this, version );
    }
    template<> DECL_EXPORT void acqiris_method::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        acqiris_method_archive<>().serialize( ar, *this, version );
    }    
    template<> DECL_EXPORT void acqiris_method::serialize( portable_binary_oarchive& ar, const unsigned int version )
    {
        acqiris_method_archive<>().serialize( ar, *this, version );
    }
    template<> DECL_EXPORT void acqiris_method::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        acqiris_method_archive<>().serialize( ar, *this, version );
    }

    template< typename T = horizontal_method >
    struct horizontal_method_archive {
        template<class Archive>
        void serialize( Archive& ar, T& _, const unsigned int version ) {
            using namespace boost::serialization;
            if ( version == 0 ) {
                double width;
                int32_t nStartDelay;
                uint32_t nbrSamples;
                ar & BOOST_SERIALIZATION_NVP( _.sampInterval );
                ar & BOOST_SERIALIZATION_NVP( _.delayTime );
                ar & BOOST_SERIALIZATION_NVP( width );
                ar & BOOST_SERIALIZATION_NVP( _.mode );
                ar & BOOST_SERIALIZATION_NVP( _.flags );
                ar & BOOST_SERIALIZATION_NVP( _.nbrAvgWaveforms );
                ar & BOOST_SERIALIZATION_NVP( nStartDelay );
                ar & BOOST_SERIALIZATION_NVP( nbrSamples );
            } else {
                ar & BOOST_SERIALIZATION_NVP( _.sampInterval );
                ar & BOOST_SERIALIZATION_NVP( _.delayTime );
                ar & BOOST_SERIALIZATION_NVP( _.nbrSamples );
                ar & BOOST_SERIALIZATION_NVP( _.mode );
                ar & BOOST_SERIALIZATION_NVP( _.flags );
                ar & BOOST_SERIALIZATION_NVP( _.nbrAvgWaveforms );
            }
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
            ar & BOOST_SERIALIZATION_NVP( _.bandwidth );
            ar & BOOST_SERIALIZATION_NVP( _.invertData );
            ar & BOOST_SERIALIZATION_NVP( _.autoScale );
            if ( version >= 2 )
                ar & BOOST_SERIALIZATION_NVP( _.enable );
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
}

using namespace acqrscontrols::aqdrv4;

static uint32_t __counter__ = 0;

acqiris_method::acqiris_method() : methodNumber_( __counter__++ )
{
}

acqiris_method::acqiris_method( const acqiris_method& t ) : clsid_( clsid() )
                                                          , trig_( t.trig_ )
                                                          , hor_( t.hor_ )
                                                          , ch1_( t.ch1_ )
                                                          , ch2_( t.ch2_ )
                                                          , methodNumber_( t.methodNumber_ )
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
    if ( ! ch2_ ) {
        ch2_ = std::make_shared< vertical_method >();
        ch2_->enable = false;
    }
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

void
vertical_method::set_enable( bool d )
{
    enable = d;
}


namespace acqrscontrols {
namespace aqdrv4 {
    struct json_vertical {
        static boost::property_tree::ptree make_ptree( const vertical_method * p ) {
            boost::property_tree::ptree ver;
            ver.put( "fullScale",   p->fullScale );
            ver.put( "offset",      p->offset );
            ver.put( "coupling",    p->coupling );
            ver.put( "bandwidth",   p->bandwidth );
            ver.put( "invertData",  p->invertData );
            ver.put( "autoScale",   p->autoScale );
            ver.put( "enable",      p->enable );
            return ver;
        }

        static void get_vertical( vertical_method& v, const boost::property_tree::ptree& p ) {
            if ( auto a = p.get_optional< double >( "fullScale" ) )
                v.fullScale = a.get();
            if ( auto a = p.get_optional< double >( "offset" ) )
                v.offset = a.get();
            if ( auto a = p.get_optional< uint32_t >( "coupling" ) )
                v.coupling = a.get();
            if ( auto a = p.get_optional< uint32_t >( "bandwidth" ) )
                v.bandwidth = a.get();
            if ( auto a = p.get_optional< bool >( "invertData" ) )
                v.invertData = a.get();
            if ( auto a = p.get_optional< bool >( "autoScale" ) )
                v.autoScale = a.get();
            if ( auto a = p.get_optional< bool >( "enable" ) )
                v.enable = a.get();
        }
    };
}
}


bool
acqiris_method::write_json( std::ostream& o, const acqiris_method& m, bool pritty )
{
    boost::property_tree::ptree pt;
    pt.put( "clsid", clsid() );
    if ( auto p = m.trig() ) {
        boost::property_tree::ptree trig;
        trig.put( "trigClass",    p->trigClass );
        trig.put( "trigPattern",  p->trigPattern );
        trig.put( "trigCoupling", p->trigCoupling );
        trig.put( "trigSlope",    p->trigSlope );
        trig.put( "trigLevel1",   p->trigLevel1 );
        trig.put( "trigLevel2",   p->trigLevel2 );
        pt.add_child( "trig", trig );
    }
    if ( auto p = m.hor() ) {
        boost::property_tree::ptree hor;
        hor.put( "sampInterval",    p->sampInterval );
        hor.put( "delayTime",       p->delayTime );
        hor.put( "nbrSamples",      p->nbrSamples );
        hor.put( "mode",            p->mode );
        hor.put( "flags",           p->flags );
        hor.put( "nbrAvgWaveforms", p->nbrAvgWaveforms );
        pt.add_child( "hor", hor );
    }

    if ( auto p = m.ext() ) 
        pt.add_child( "ext", json_vertical::make_ptree( p.get() ) );
    if ( auto p = m.ch1() ) 
        pt.add_child( "ch1", json_vertical::make_ptree( p.get() ) );
    if ( auto p = m.ch2() ) 
        pt.add_child( "ch2", json_vertical::make_ptree( p.get() ) );

    boost::property_tree::write_json( o, pt, pritty );

    return true;
}

bool
acqiris_method::read_json( std::istream& i, acqiris_method& m )
{
    boost::property_tree::ptree pt;
    boost::property_tree::read_json( i, pt );

    if ( auto p = pt.get_child_optional( "trig" ) ) {
        auto d = m.mutable_trig();
        if ( auto a = p->get_optional< uint32_t >( "trigClass" ) )
            d->trigClass = a.get();
        if ( auto a = p->get_optional< uint32_t >( "trigPattern" ) )
            d->trigPattern = a.get();
        if ( auto a = p->get_optional< uint32_t >( "trigCoupling" ) )
            d->trigCoupling = a.get();
        if ( auto a = p->get_optional< uint32_t >( "trigSlope" ) )
            d->trigSlope = a.get();
        if ( auto a = p->get_optional< double >( "trigLevel1" ) )
            d->trigLevel1 = a.get();
        if ( auto a = p->get_optional< double >( "trigLevel2" ) )
            d->trigLevel2 = a.get();                        
    }
    
    if ( auto p = pt.get_child_optional( "hor" ) ) {
        auto d = m.mutable_hor();
        if ( auto a = p->get_optional< double >( "sampInterval" ) )
            d->sampInterval = a.get();
        if ( auto a = p->get_optional< double >( "delayTime" ) )
            d->delayTime = a.get();
        if ( auto a = p->get_optional< uint32_t >( "nbrSamples" ) )
            d->nbrSamples = a.get();                
        if ( auto a = p->get_optional< uint32_t >( "mode" ) )
            d->mode = a.get();                
        if ( auto a = p->get_optional< uint32_t >( "flags" ) )
            d->flags = a.get();                
        if ( auto a = p->get_optional< uint32_t >( "nbrAvgWaveforms" ) )
            d->nbrAvgWaveforms = a.get();                
    }
    
    if ( auto p = pt.get_child_optional( "ext" ) ) {
        auto d = m.mutable_ext();
        json_vertical::get_vertical( *d, p.get() );
    }
    
    if ( auto p = pt.get_child_optional( "ch1" ) ) {
        auto d = m.mutable_ch1();
        json_vertical::get_vertical( *d, p.get() );
    }
    
    if ( auto p = pt.get_child_optional( "ch2" ) ) {
        auto d = m.mutable_ch2();
        json_vertical::get_vertical( *d, p.get() );
    }
    
    return true;
}
