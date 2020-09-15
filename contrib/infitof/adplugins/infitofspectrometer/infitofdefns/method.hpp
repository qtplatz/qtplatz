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

#pragma once

#if defined __GNUC__ && !__APPLE__
#  pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#endif
#include <adportable_serializer/portable_binary_oarchive.hpp>
#include <adportable_serializer/portable_binary_iarchive.hpp>
#include <compiler/boost/workaround.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/noncopyable.hpp>
#include <boost/uuid/uuid_generators.hpp>

namespace infitof {

    enum eIonSource {
        eIonSource_EI
        , eIonSource_MALDI
    };

    enum ePolarity { ePolarity_Indeterminate, ePolarity_Positive, ePolarity_Negative };

    class IonSource_EI_Method {
    public:
        // EI
        double einzel_voltage;
        double va_pulse_voltage;
        double potential_lift_pulse_voltage;
        // Filament
        double filament_current; // mA
        double ionization_current; // := trap current (mA)
        double ionization_voltage; // V
        double ei_temp;
        double gc_temp;
    private:
        friend class boost::serialization::access;
        template< class Archive >
        void serialize( Archive& ar, const unsigned int ) {
            ar & BOOST_SERIALIZATION_NVP( einzel_voltage );
            ar & BOOST_SERIALIZATION_NVP( va_pulse_voltage );
            ar & BOOST_SERIALIZATION_NVP( potential_lift_pulse_voltage );
            ar & BOOST_SERIALIZATION_NVP( filament_current ); // mA
            ar & BOOST_SERIALIZATION_NVP( ionization_current );
            ar & BOOST_SERIALIZATION_NVP( ionization_voltage );
            ar & BOOST_SERIALIZATION_NVP( ei_temp );
            ar & BOOST_SERIALIZATION_NVP( gc_temp );
        }
    };

    class IonSource_MALDI_Method {
    public:
        double tba;
    private:
        friend class boost::serialization::access;
        template< class Archive >
        void serialize( Archive& ar, const unsigned int ) {
            ar  & BOOST_SERIALIZATION_NVP( tba );
        }
    };

    class ElectricSectorMethod {
    public:
        double outer_voltage;
        double inner_voltage;
        ElectricSectorMethod()
            : outer_voltage(0), inner_voltage(0)
            {}
    private:
        friend class boost::serialization::access;
        template< class Archive >
        void serialize( Archive& ar, const unsigned int ) {
            ar & BOOST_SERIALIZATION_NVP( outer_voltage );
            ar & BOOST_SERIALIZATION_NVP( inner_voltage );
        }
    };

    class AnalyzerMethod {
    public:
        static const char * dataClass() { return "WTI_HV"; }
        ElectricSectorMethod injection_sector;
        ElectricSectorMethod ejection_sector;
        ElectricSectorMethod orbit_sector;
        double matsuda_plate_voltage;
        double detector_voltage;
        double accelaration_voltage;
        double ion_gate_voltage;
        AnalyzerMethod() : matsuda_plate_voltage(0)
                         , detector_voltage(0)
                         , accelaration_voltage(0)
                         , ion_gate_voltage(0) {
        }
        AnalyzerMethod( const AnalyzerMethod& t ) : injection_sector( t.injection_sector )
                                                  , ejection_sector( t.ejection_sector )
                                                  , orbit_sector( t.orbit_sector )
                                                  , matsuda_plate_voltage( t.matsuda_plate_voltage )
                                                  , detector_voltage( t.detector_voltage )
                                                  , accelaration_voltage( t.accelaration_voltage )
                                                  , ion_gate_voltage( t.ion_gate_voltage ) {
        }
    private:
        friend class boost::serialization::access;
        template< class Archive >
        void serialize( Archive& ar, const unsigned int ) {
            ar & BOOST_SERIALIZATION_NVP( injection_sector );
            ar & BOOST_SERIALIZATION_NVP( ejection_sector );
            ar & BOOST_SERIALIZATION_NVP( orbit_sector );
            ar & BOOST_SERIALIZATION_NVP( matsuda_plate_voltage );
            ar & BOOST_SERIALIZATION_NVP( detector_voltage );
            ar & BOOST_SERIALIZATION_NVP( accelaration_voltage );
            ar & BOOST_SERIALIZATION_NVP( ion_gate_voltage );
        }
    };

    class DelayMethod {
    public:
        double delay; // seconds
        double width; // seconds
        DelayMethod() : delay(0), width( 0 ) {
        }
        DelayMethod( double d, double w ) : delay( d ), width( w ) {
        }
        DelayMethod( const DelayMethod& t ) : delay( t.delay ), width( t.width ) {
        }
    private:
        friend class boost::serialization::access;
        template< class Archive >
        void serialize( Archive& ar, const unsigned int ) {
            ar & BOOST_SERIALIZATION_NVP( delay ) & BOOST_SERIALIZATION_NVP( width );
        }
    };

	//--
    class OrbitProtocol {
    public:
        enum eItem { MCP_V, IONIZATION_V, NAVERAGE, GAIN, NINTVAL, FIL_A };
        // Gain := detector gain switch, NINTVAL := sampling interval (fold of base speed (0.5ns))

        double lower_mass;    // lower mass
        double upper_mass;    // upper mass
        double avgr_delay;    // seconds
        double avgr_duration; // seconds
        DelayMethod pulser;   // A
        DelayMethod inject;   // B
        DelayMethod exit;     // D
        std::vector< DelayMethod > gate;  // C
        DelayMethod external_adc_delay;
        std::string description_;
        std::vector< std::pair< eItem, int32_t > > additionals_;
        uint32_t nlaps_;       // number of laps
        uint32_t reference_;   // lock mass reference (bit position indicate which formula in formulae
        std::string formulae_; // formula list, separate with ';'

        OrbitProtocol() : lower_mass( 0 ), upper_mass( 0 ), avgr_delay( 0 ), avgr_duration( 0 ), nlaps_( 0 ), reference_( 0 ), gate( 2 ) {
        }
        OrbitProtocol( const OrbitProtocol& t)
            : lower_mass(t.lower_mass)
            , upper_mass(t.upper_mass)
            , avgr_delay(t.avgr_delay)
            , avgr_duration(t.avgr_duration)
            , pulser( t.pulser )
            , inject( t.inject )
            , exit( t.exit )
            , gate( t.gate )
            , description_( t.description_ )
            , additionals_( t.additionals_ )
            , nlaps_( t.nlaps_ )
            , reference_( t.reference_ )
            , formulae_( t.formulae_ )
            , external_adc_delay( t.external_adc_delay ) {
        }
    private:
        friend class boost::serialization::access;
        template< class Archive >
        void serialize( Archive& ar, const unsigned int version ) {
            if ( version >= 6 ) {
                ar & BOOST_SERIALIZATION_NVP( lower_mass );
                ar & BOOST_SERIALIZATION_NVP( upper_mass );
                ar & BOOST_SERIALIZATION_NVP( avgr_delay );
                ar & BOOST_SERIALIZATION_NVP( avgr_duration );
                ar & BOOST_SERIALIZATION_NVP( pulser );
                ar & BOOST_SERIALIZATION_NVP( inject );
                ar & BOOST_SERIALIZATION_NVP( exit );
                ar & BOOST_SERIALIZATION_NVP( gate );
                ar & BOOST_SERIALIZATION_NVP( additionals_ );
                ar & BOOST_SERIALIZATION_NVP( description_ );
                ar & BOOST_SERIALIZATION_NVP( nlaps_ );
                ar & BOOST_SERIALIZATION_NVP( reference_ );
                ar & BOOST_SERIALIZATION_NVP( formulae_ );
                if ( version >= 7 )
                    ar & BOOST_SERIALIZATION_NVP( external_adc_delay );
            } else {
                ar & BOOST_SERIALIZATION_NVP( lower_mass );
                ar & BOOST_SERIALIZATION_NVP( upper_mass );
                ar & BOOST_SERIALIZATION_NVP( avgr_delay );
                ar & BOOST_SERIALIZATION_NVP( avgr_duration );
                if ( version >= 2 ) {
                    ar & BOOST_SERIALIZATION_NVP( pulser );
                    ar & BOOST_SERIALIZATION_NVP( inject );
                    ar & BOOST_SERIALIZATION_NVP( exit );
                    ar & BOOST_SERIALIZATION_NVP( gate[ 0 ] );
                }
                if ( version == 3 ) {
                    int32_t mcp, ionization;
                    ar & BOOST_SERIALIZATION_NVP( mcp );
                    ar & BOOST_SERIALIZATION_NVP( ionization );
                    ar & BOOST_SERIALIZATION_NVP( description_ );
                    additionals_.push_back( std::make_pair( MCP_V, mcp ) );
                    additionals_.push_back( std::make_pair( IONIZATION_V, ionization ) );
                } else if ( version >= 4 ) {
                    ar & BOOST_SERIALIZATION_NVP( additionals_ );
                    ar & BOOST_SERIALIZATION_NVP( description_ );
                }
                if ( version >= 5 ) {
                    ar & BOOST_SERIALIZATION_NVP( nlaps_ );
                    ar & BOOST_SERIALIZATION_NVP( reference_ );
                    ar & BOOST_SERIALIZATION_NVP( formulae_ );
                }
            }
        }
    };

    class AvgrMethod {
    public:
        static const char * modelClass() { return "InfiTOF,Avgr"; }
        static const char * itemLabel() { return "Avgr"; }
        static const boost::uuids::uuid& clsid() {
            static boost::uuids::uuid myclsid = boost::uuids::string_generator()( "{27941579-B341-4446-A172-6DF63869988B}" );
            return myclsid;
        }

        bool isMaxNumAverage;
        bool isLinear;
        int32_t numAverage;
        int32_t gain;
        int32_t trigInterval; // microseconds (1000 := 1kHz)
        uint32_t nTurn;       // deprecated -- use protocols nturns
        OrbitProtocol linear_protocol;
        std::vector< OrbitProtocol > protocols;
        uint32_t nReplicates;
        uint32_t autoBackground;
        AvgrMethod() : isMaxNumAverage( 0 ), isLinear( true ), numAverage( 0 )
                     , gain( 0 ), trigInterval( 1000 ), nTurn( 0 ), nReplicates(1), autoBackground(0) {
        }
        AvgrMethod( const AvgrMethod& t ) : isMaxNumAverage( t.isMaxNumAverage )
                                          , isLinear( t.isLinear )
                                          , numAverage( t.numAverage )
                                          , gain( t.gain )
                                          , trigInterval( t.trigInterval )
                                          , nTurn( t.nTurn )
                                          , linear_protocol( t.linear_protocol )
                                          , protocols( t.protocols )
                                          , nReplicates( t.nReplicates )
                                          , autoBackground( t.autoBackground ) {
        }

    private:
        friend class boost::serialization::access;
        template< class Archive >
        void serialize( Archive& ar, const unsigned int version ) {
            ar & BOOST_SERIALIZATION_NVP( isMaxNumAverage );
            ar & BOOST_SERIALIZATION_NVP( isLinear );
            ar & BOOST_SERIALIZATION_NVP(numAverage );
            ar & BOOST_SERIALIZATION_NVP( gain );
            ar & BOOST_SERIALIZATION_NVP( trigInterval );
            ar & BOOST_SERIALIZATION_NVP( linear_protocol );
            ar & BOOST_SERIALIZATION_NVP( nTurn );
            ar & BOOST_SERIALIZATION_NVP( protocols );
            if ( version >= 2 )
                ar & BOOST_SERIALIZATION_NVP( nReplicates );
            if ( version >= 3 )
                ar & BOOST_SERIALIZATION_NVP( autoBackground );
        }
    };


    /*************************************/

    class ControlMethod {
    public:
        static const char * modelClass() { return "InfiTOF"; }
        static const char * itemLabel() { return "ControlMethod"; }
        static const boost::uuids::uuid& clsid() {
            static boost::uuids::uuid myclsid = boost::uuids::string_generator()( "{3474E780-466A-4D64-95F5-9E2084B06B06}" );
            return myclsid;
        }

        AnalyzerMethod analyzer; // WTI
        AvgrMethod tof;  // AP240 | ARP
        boost::variant< IonSource_EI_Method, IonSource_MALDI_Method > ionSource; // WTI
        std::vector< int32_t > arp_hv_;  // infitofinterface::arp::HVSetpts
        std::string description_;

        ControlMethod() : ionSource( IonSource_EI_Method() ) {
        }

        ControlMethod( const ControlMethod& t ) : analyzer( t.analyzer )
                                                , tof( t.tof )
                                                , ionSource( t.ionSource ) {
        }

        static bool archive( std::ostream& os , const ControlMethod& t ) {
            try {
                portable_binary_oarchive ar( os );
                ar & t;
                return true;
            } catch ( std::exception& ) {
            }
            return false;
        }

        static bool restore( std::istream& is, ControlMethod& t ) {
            try {
                portable_binary_iarchive ar( is );
                ar & boost::serialization::make_nvp( "m", t );
                return true;
            } catch ( std::exception& ) {
            }
            return false;
        }

    private:
        friend class boost::serialization::access;
        template< class Archive >
        void serialize( Archive& ar, const unsigned int version ) {
            ar & BOOST_SERIALIZATION_NVP( analyzer );
            ar & BOOST_SERIALIZATION_NVP( tof );
            ar & BOOST_SERIALIZATION_NVP( ionSource );
            if ( version >= 3 ) {
                ar & BOOST_SERIALIZATION_NVP( arp_hv_ );
                ar & BOOST_SERIALIZATION_NVP( description_ );
            }
        }
    };

};

BOOST_CLASS_VERSION( infitof::ControlMethod, 3 )
BOOST_CLASS_VERSION( infitof::OrbitProtocol, 7 )
BOOST_CLASS_VERSION( infitof::AvgrMethod, 3 )
