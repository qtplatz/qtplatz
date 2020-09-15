/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
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
#include <adportable/debug.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/nvp.hpp>
#include <adportable_serializer/portable_binary_oarchive.hpp>
#include <adportable_serializer/portable_binary_iarchive.hpp>
#include <compiler/boost/workaround.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/uuid/uuid_generators.hpp>

#if defined __GNUC__
# pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

namespace infitofcontrols {

    template<typename T >
    class AvgrMethod_archive {
    public:
        template<class Archive>
        void serialize( Archive& ar, T& _, const unsigned int version ) {
            using namespace boost::serialization;

            bool isLinear_deprecated;
            admtcontrols::OrbitProtocol linear_protocol;

            ar & BOOST_SERIALIZATION_NVP( _.isMaxNumAverage );

            if ( version < 5 )
                ar & BOOST_SERIALIZATION_NVP( isLinear_deprecated );

            ar & BOOST_SERIALIZATION_NVP( _.numAverage );
            ar & BOOST_SERIALIZATION_NVP( _.gain );
            ar & BOOST_SERIALIZATION_NVP( _.trigInterval );

            if ( version < 5 )
                ar & BOOST_SERIALIZATION_NVP( linear_protocol );

            ar & BOOST_SERIALIZATION_NVP( _.nTurn );
            ar & BOOST_SERIALIZATION_NVP( _.protocols );
            if ( version >= 2 )
                ar & BOOST_SERIALIZATION_NVP( _.nReplicates );
            if ( version >= 3 )
                ar & BOOST_SERIALIZATION_NVP( _.autoBackground );
            if ( version >= 6 ) {
                ar & BOOST_SERIALIZATION_NVP( _.enableGateWindow );
                ar & BOOST_SERIALIZATION_NVP( _.gateWindow );
            }
        }
    };

    template<> INFITOFCONTROLSSHARED_EXPORT void AvgrMethod::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
    {
        AvgrMethod_archive<>().serialize( ar, *this, version );
    }
    template<> INFITOFCONTROLSSHARED_EXPORT void AvgrMethod::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        AvgrMethod_archive<>().serialize( ar, *this, version );
    }

    template<> INFITOFCONTROLSSHARED_EXPORT void AvgrMethod::serialize( portable_binary_oarchive& ar, const unsigned int version )
    {
        AvgrMethod_archive<>().serialize( ar, *this, version );
    }

    template<> INFITOFCONTROLSSHARED_EXPORT void AvgrMethod::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        AvgrMethod_archive<>().serialize( ar, *this, version );
    }
}

using namespace infitofcontrols;

// static
const boost::uuids::uuid&
AvgrMethod::clsid()
{
    static boost::uuids::uuid baseid = boost::uuids::string_generator()( "{3D2F180E-18E9-43D3-9A37-9E981B509CAA}" );
    static const boost::uuids::uuid myclsid = boost::uuids::name_generator( baseid )( "InfiTOF.Avgr" );
    return myclsid;
}

bool
AvgrMethod::archive( std::ostream& os, const AvgrMethod& t )
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
AvgrMethod::restore( std::istream& is, AvgrMethod& t )
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
AvgrMethod::xml_archive( std::wostream& os, const AvgrMethod& t )
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
AvgrMethod::xml_restore( std::wistream& is, AvgrMethod& t )
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

AvgrMethod::AvgrMethod() : isMaxNumAverage( 0 ), numAverage( 0 )
                         , gain( 0 )
                         , trigInterval( 1000 )
                         , nTurn( 0 )
                         , nReplicates( 1 )
                         , autoBackground( 0 )
                         , enableGateWindow( false )
                         , gateWindow( 0 )
{
}

AvgrMethod::AvgrMethod( const AvgrMethod& t ) : isMaxNumAverage( t.isMaxNumAverage )
                                              , numAverage( t.numAverage )
                                              , gain( t.gain )
                                              , trigInterval( t.trigInterval )
                                              , nTurn( t.nTurn )
                                              , protocols( t.protocols )
                                              , nReplicates( t.nReplicates )
                                              , autoBackground( t.autoBackground )
                                              , enableGateWindow( t.enableGateWindow )
                                              , gateWindow( t.gateWindow )
{
}
