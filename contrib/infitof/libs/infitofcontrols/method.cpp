/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#include "method.hpp"
#include "infitofcontrols_global.hpp"
#include <admtcontrols/scanlaw.hpp>
#include <admtcontrols/orbitprotocol.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adcontrols/tofprotocol.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/nvp.hpp>
#include <adportable/portable_binary_iarchive.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <compiler/boost/workaround.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/uuid/uuid_generators.hpp>

namespace infitofcontrols {

    template<typename T = method >
    class method_archive {
    public:
        template<class Archive>
        void serialize( Archive& ar, T& _, const unsigned int version ) {
            using namespace boost::serialization;
            if ( version >= 4 ) {
                ar & BOOST_SERIALIZATION_NVP( _.tof_ );
                ar & BOOST_SERIALIZATION_NVP( _.description_ );
            } else {
                throw boost::archive::archive_exception( boost::archive::archive_exception::unsupported_version );
            }
        }
    };

    template<> INFITOFCONTROLSSHARED_EXPORT void method::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
    {
        method_archive<>().serialize( ar, *this, version );
    }

    template<> INFITOFCONTROLSSHARED_EXPORT void method::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        method_archive<>().serialize( ar, *this, version );
    }

    template<> INFITOFCONTROLSSHARED_EXPORT void method::serialize( portable_binary_oarchive& ar, const unsigned int version )
    {
        method_archive<>().serialize( ar, *this, version );
    }

    template<> INFITOFCONTROLSSHARED_EXPORT void method::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        method_archive<>().serialize( ar, *this, version );
    }
}

using namespace infitofcontrols;

///////////////////
const boost::uuids::uuid&
method::clsid()
{
    static boost::uuids::uuid baseid = boost::uuids::string_generator()( "{3D2F180E-18E9-43D3-9A37-9E981B509CAA}" );
    static const boost::uuids::uuid myclsid = boost::uuids::name_generator( baseid )( "InfiTOF.method" );
    return myclsid;
}

std::vector<int32_t>&
method::hv()
{
    return arp_hv_;
}

const std::vector<int32_t>&
method::hv() const
{
    return arp_hv_;
}

AvgrMethod&
method::tof()
{
    return tof_;
}

const AvgrMethod&
method::tof() const
{
    return tof_;
}

std::string&
method::description()
{
    return description_;
}

const std::string&
method::description() const
{
    return description_;
}

bool
method::archive( std::ostream& os, const method& t )
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

bool
method::restore( std::istream& is, method& t )
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

bool
method::xml_archive( std::wostream& os, const method& t )
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

bool
method::xml_restore( std::wistream& is, method& t )
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

method::~method()
{
}

method::method() : arp_hv_( { 0 } )
{
}

method::method( const method& t ) : tof_( t.tof_ )
                                  , arp_hv_( t.arp_hv_ )
{
}

void
method::setup_default( method& m )
{
    using namespace adcontrols::metric;

    admtcontrols::infitof::ScanLaw scanLaw;

#if 0
    // infitofcontrols::IonSource_EI_Method ei = { 0, 0, 0, 0, 0, 0, 0, 0 };
    // m.ionSource() = ei;
    // m.tof().numAverage = 500;
    // m.tof().gain = 1;
    // m.tof().trigInterval = 1000; // (us)
#endif
    admtcontrols::OrbitProtocol p;
    p.lower_mass = 10;
    p.upper_mass = 200;
    p.formulae() = "Fix me";
    p.avgr_delay = scanLaw.getTime( p.lower_mass, 0 );
    p.avgr_duration = scanLaw.getTime( p.upper_mass, 0 ) - p.avgr_delay;

    p.inject.delay = scale_to_base( 0.0, micro ); // injection open at 100us advance
    p.inject.width = scale_to_base( 10.0, micro );

    p.pulser.delay = scale_to_base( 0.0, micro ); // AP240 To (time zero)
    p.pulser.width = scale_to_base( 10.0, micro ); // 100us duration

    for ( auto& gate : p.gate ) {
        gate.delay = 0;
        gate.width = scale_to_base( 10.0, micro );
    }

    p.exit.delay   = p.pulser.delay;
    p.exit.width   = scale_to_base( 100.0, micro );

    m.tof().nTurn = 0;
    m.tof().protocols.push_back( p );
    m.tof().protocols.front().formulae() = "N2O";
}

// static
void
method::copy_protocols( const AvgrMethod& tof, std::vector< adcontrols::TofProtocol >& v )
{
    v.resize( tof.protocols.size() );

    auto it = v.begin();
    for ( auto& proto: tof.protocols ) {

        adcontrols::TofProtocol& lhs = *it++;

        lhs.setDigitizerDelayWidth( { proto.avgr_delay, proto.avgr_duration }  );

        lhs.delay_pulses() [ adcontrols::TofProtocol::MULTUM_PUSH ] = std::make_pair( proto.pulser.delay, proto.pulser.width );
        lhs.delay_pulses() [ adcontrols::TofProtocol::MULTUM_INJECT ] = std::make_pair( proto.inject.delay, proto.inject.width );
        lhs.delay_pulses() [ adcontrols::TofProtocol::MULTUM_EXIT ] = std::make_pair( proto.exit.delay, proto.exit.width );
        lhs.delay_pulses() [ adcontrols::TofProtocol::MULTUM_GATE_0 ] = std::make_pair( proto.gate[0].delay, proto.gate[0].width );
        lhs.delay_pulses() [ adcontrols::TofProtocol::MULTUM_GATE_1 ] = std::make_pair( proto.gate[1].delay, proto.gate[1].width );
        lhs.delay_pulses() [ adcontrols::TofProtocol::EXT_ADC_TRIG ] = std::make_pair( proto.external_adc_delay.delay, proto.external_adc_delay.width );
        lhs.setNumber_of_triggers( tof.numAverage ); // replace 0 ?? if average mode

        lhs.setMode( proto.nlaps() );

        lhs.formulae().clear();
        lhs.formulae().push_back( proto.formulae() );
        lhs.setReference( proto.reference() );

        //enum eItem { MCP_V, IONIZATION_V, NAVERAGE, GAIN, NINTVAL, FIL_A };
        lhs.additionals().resize( proto.additionals().size() );
        std::copy( proto.additionals().begin(), proto.additionals().end(), lhs.additionals().begin() );
    }
}

void
method::copy_protocols( std::vector< adcontrols::TofProtocol >& t ) const
{
    copy_protocols( tof_, t );
}
