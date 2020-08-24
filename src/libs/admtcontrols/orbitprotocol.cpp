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

#include "orbitprotocol.hpp"
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

namespace admtcontrols {

    class OrbitProtocol;

    //////////////////////////////////////////
    template<typename T = OrbitProtocol >
    class OrbitProtocol_archive {
    public:
        template<class Archive>
        void serialize( Archive& ar, T& _, const unsigned int version ) {
            using namespace boost::serialization;
            if ( version >= 6 ) {
                ar & BOOST_SERIALIZATION_NVP( _.lower_mass );
                ar & BOOST_SERIALIZATION_NVP( _.upper_mass );
                ar & BOOST_SERIALIZATION_NVP( _.avgr_delay );
                ar & BOOST_SERIALIZATION_NVP( _.avgr_duration );
                ar & BOOST_SERIALIZATION_NVP( _.pulser );
                ar & BOOST_SERIALIZATION_NVP( _.inject );
                ar & BOOST_SERIALIZATION_NVP( _.exit );
                ar & BOOST_SERIALIZATION_NVP( _.gate );
                ar & BOOST_SERIALIZATION_NVP( _.additionals_ );
                ar & BOOST_SERIALIZATION_NVP( _.description_ );
                ar & BOOST_SERIALIZATION_NVP( _.nlaps_ );
                ar & BOOST_SERIALIZATION_NVP( _.reference_ );
                ar & BOOST_SERIALIZATION_NVP( _.formulae_ );

                if ( version >= 7 )
                    ar & BOOST_SERIALIZATION_NVP( _.external_adc_delay );
                if ( version >= 8 )
                    ar & BOOST_SERIALIZATION_NVP( _.replicates_ );
                if ( version >= 9 )
                    ar & BOOST_SERIALIZATION_NVP( _.exit2 );

            } else {
                ar & BOOST_SERIALIZATION_NVP( _.lower_mass );
                ar & BOOST_SERIALIZATION_NVP( _.upper_mass );
                ar & BOOST_SERIALIZATION_NVP( _.avgr_delay );
                ar & BOOST_SERIALIZATION_NVP( _.avgr_duration );
                if ( version >= 2 ) {
                    ar & BOOST_SERIALIZATION_NVP( _.pulser );
                    ar & BOOST_SERIALIZATION_NVP( _.inject );
                    ar & BOOST_SERIALIZATION_NVP( _.exit );
                    ar & BOOST_SERIALIZATION_NVP( _.gate[ 0 ] );
                }
                if ( version == 3 ) {
                    int32_t mcp, ionization;
                    ar & BOOST_SERIALIZATION_NVP( mcp );
                    ar & BOOST_SERIALIZATION_NVP( ionization );
                    ar & BOOST_SERIALIZATION_NVP( _.description_ );
                    _.additionals_.push_back( std::make_pair( admtcontrols::OrbitProtocol::MCP_V, mcp ) );
                    _.additionals_.push_back( std::make_pair( OrbitProtocol::IONIZATION_V, ionization ) );
                } else if ( version >= 4 ) {
                    ar & BOOST_SERIALIZATION_NVP( _.additionals_ );
                    ar & BOOST_SERIALIZATION_NVP( _.description_ );
                }
                if ( version >= 5 ) {
                    ar & BOOST_SERIALIZATION_NVP( _.nlaps_ );
                    ar & BOOST_SERIALIZATION_NVP( _.reference_ );
                    ar & BOOST_SERIALIZATION_NVP( _.formulae_ );
                }
            }
        }
    };

    template<> ADMTCONTROLSSHARED_EXPORT void OrbitProtocol::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
    {
        OrbitProtocol_archive<>().serialize( ar, *this, version );
    }

    template<> ADMTCONTROLSSHARED_EXPORT void OrbitProtocol::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        OrbitProtocol_archive<>().serialize( ar, *this, version );
    }

    template<> ADMTCONTROLSSHARED_EXPORT void OrbitProtocol::serialize( portable_binary_oarchive& ar, const unsigned int version )
    {
        OrbitProtocol_archive<>().serialize( ar, *this, version );
    }

    template<> ADMTCONTROLSSHARED_EXPORT void OrbitProtocol::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        OrbitProtocol_archive<>().serialize( ar, *this, version );
    }

    ///////////////////////////////////////////////////

    template<typename T = DelayMethod >
    class DelayMethod_archive {
    public:
        template<class Archive>
        void serialize( Archive& ar, T& _, const unsigned int version ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( _.delay );
            ar & BOOST_SERIALIZATION_NVP( _.width );
            if ( version >= 1 )
                ar & BOOST_SERIALIZATION_NVP( _.enable );
        }
    };

    template<> ADMTCONTROLSSHARED_EXPORT void DelayMethod::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
    {
        DelayMethod_archive<>().serialize( ar, *this, version );
    }

    template<> ADMTCONTROLSSHARED_EXPORT void DelayMethod::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        DelayMethod_archive<>().serialize( ar, *this, version );
    }

    template<> ADMTCONTROLSSHARED_EXPORT void DelayMethod::serialize( portable_binary_oarchive& ar, const unsigned int version )
    {
        DelayMethod_archive<>().serialize( ar, *this, version );
    }

    template<> ADMTCONTROLSSHARED_EXPORT void DelayMethod::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        DelayMethod_archive<>().serialize( ar, *this, version );
    }
    ///////////////////////////////////////////////////
} // namespace


using namespace admtcontrols;

//////////////////////

DelayMethod::DelayMethod( double _d, double _w, bool _e ) : delay( _d ), width( _w ), enable( _e )
{
}

DelayMethod::DelayMethod( const DelayMethod& t ) : delay( t.delay ), width( t.width ), enable( t.enable )
{
}

////////////////////////////

OrbitProtocol::OrbitProtocol() : lower_mass( 0 )
                               , upper_mass( 0 )
                               , avgr_delay( 0 )
                               , avgr_duration( 0 )
                               , exit2( 0, 0, false )
                               , gate( 2 )
                               , nlaps_( 0 )
                               , reference_( 0 )
                               , replicates_( 1 )
{
}

OrbitProtocol::OrbitProtocol( const OrbitProtocol& t ) : lower_mass(t.lower_mass)
                                                       , upper_mass(t.upper_mass)
                                                       , avgr_delay(t.avgr_delay)
                                                       , avgr_duration(t.avgr_duration)
                                                       , pulser( t.pulser )
                                                       , inject( t.inject )
                                                       , exit( t.exit )
                                                       , exit2( t.exit2 )
                                                       , gate( t.gate )
                                                       , external_adc_delay( t.external_adc_delay )
                                                       , additionals_( t.additionals_ )
                                                       , nlaps_( t.nlaps_ )
                                                       , reference_( t.reference_ )
                                                       , formulae_( t.formulae_ )
                                                       , description_( t.description_ )
                                                       , replicates_( t.replicates_ )
{
}

std::string&
OrbitProtocol::description()
{
    return description_;
}

const std::string&
OrbitProtocol::description() const
{
    return description_;
}

std::string&
OrbitProtocol::formulae()
{
    return formulae_;
}

const std::string&
OrbitProtocol::formulae() const
{
    return formulae_;
}

std::vector< std::pair< OrbitProtocol::eItem, int32_t > >&
OrbitProtocol::additionals()
{
    return additionals_;
}

const std::vector< std::pair< OrbitProtocol::eItem, int32_t > >&
OrbitProtocol::additionals() const
{
    return additionals_;
}

uint32_t&
OrbitProtocol::nlaps()
{
    return nlaps_;
}

uint32_t
OrbitProtocol::nlaps() const
{
    return nlaps_;
}

uint32_t&
OrbitProtocol::reference()
{
    return reference_;
}

uint32_t
OrbitProtocol::reference() const
{
    return reference_;
}

uint32_t&
OrbitProtocol::replicates()
{
    return replicates_;
}

uint32_t
OrbitProtocol::replicates() const
{
    return replicates_;
}

///////////////////
