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

#include "avgrdata.hpp"
#include "avgr_acqiris.hpp"
#include "avgr_u5303a.hpp"
#include "method.hpp"
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/version.hpp>
#include <adportable_serializer/portable_binary_oarchive.hpp>
#include <adportable_serializer/portable_binary_iarchive.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>

namespace infitof {

    template< typename T = DelayData >
    class DelayData_archive {
    public:
        template<class Archive>
        void serialize( Archive& ar, T& _, const unsigned int version ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( _.delay_ );
            ar & BOOST_SERIALIZATION_NVP( _.duration_ );
        }
    };

    template<> void DelayData::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
    {
        DelayData_archive<>().serialize( ar, *this, version );
    }
    template<> void DelayData::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        DelayData_archive<>().serialize( ar, *this, version );
    }

    template<> void DelayData::serialize( portable_binary_oarchive& ar, const unsigned int version )
    {
        DelayData_archive<>().serialize( ar, *this, version );
    }

    template<> void DelayData::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        DelayData_archive<>().serialize( ar, *this, version );
    }

    /////////////////////////
    template< typename T = AveragerData >
    class AveragerData_archive {
    public:
        template<class Archive>
        void serialize( Archive& ar, T& _, const unsigned int version ) {

            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( _.npos );               // 0
            if ( version < 5 ) {
                int32_t usec;
                ar & BOOST_SERIALIZATION_NVP( usec );
            } else {
                // V5
                ar & BOOST_SERIALIZATION_NVP( _.timeSinceInject ); // 1  Since Version 5, change this to 64bit signed
                ar & BOOST_SERIALIZATION_NVP( _.timeSinceEpoch );
            }
            ar & BOOST_SERIALIZATION_NVP( _.uptime );             // 2,3
            ar & BOOST_SERIALIZATION_NVP( _.nbrSamples );          // 4 (total number in waveform) --> will obsolte
            ar & BOOST_SERIALIZATION_NVP( _.nbrAverage );          // 5
            ar & BOOST_SERIALIZATION_NVP( _.nSamplingDelay );      // 6 (point to 1st segment data)
            ar & BOOST_SERIALIZATION_NVP( _.sampInterval );        // 7 (ps)
            ar & BOOST_SERIALIZATION_NVP( _.wellKnownEvents );     // 8

            ar & BOOST_SERIALIZATION_NVP( _.nTurns_deprecated );
            ar & BOOST_SERIALIZATION_NVP( _.protocolId );          // current protocol (a.k.a. segment) number in sequence of protocols
            ar & BOOST_SERIALIZATION_NVP( _.nProtocols );          // number of protocols
            ar & BOOST_SERIALIZATION_NVP( _.avgrType );

            ar & BOOST_SERIALIZATION_NVP( _.desc );

            if ( version == 2 || version == 3 ) {
                std::vector<double> coeffs; // deprecated
                ar & BOOST_SERIALIZATION_NVP( coeffs );
            }

            if ( version >= 3 )
                ar & BOOST_SERIALIZATION_NVP( _.protocol_ );

            if ( version >= 4 ) {
                ar & BOOST_SERIALIZATION_NVP( _.kAcceleratorVoltage );
                ar & BOOST_SERIALIZATION_NVP( _.tDelay );
            }
            ar & BOOST_SERIALIZATION_NVP( _.mark );
            ar & BOOST_SERIALIZATION_NVP( _.waveform );

            _.archive_version_ = version;
        }

    };

    template<> void AveragerData::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
    {
        AveragerData_archive<>().serialize( ar, *this, version );
    }
    template<> void AveragerData::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        AveragerData_archive<>().serialize( ar, *this, version );
    }

    template<> void AveragerData::serialize( portable_binary_oarchive& ar, const unsigned int version )
    {
        AveragerData_archive<>().serialize( ar, *this, version );
    }

    template<> void AveragerData::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        AveragerData_archive<>().serialize( ar, *this, version );
    }

}

using namespace infitof;

AveragerData::AveragerData() : npos(0)
                             , timeSinceInject( 0 )
                             , timeSinceEpoch( 0 )
                             , uptime( 0 )
                             , nbrSamples( 0 )
                             , nbrAverage( 0 )
                             , nSamplingDelay( 0 )
                             , sampInterval( 0 )
                             , wellKnownEvents( 0 )
                             , nTurns_deprecated( 0 )
                             , protocolId( 0 )
                             , nProtocols( 0 )
                             , avgrType( Averager_AP240 )
                             , kAcceleratorVoltage( 0 )
                             , tDelay( 0 )
                             , mark( 0x20130828 )
                             , archive_version_( -1 )
{
}

AveragerData::~AveragerData()
{
}
