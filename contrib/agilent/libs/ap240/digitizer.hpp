/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#ifndef DIGITIZER_HPP
#define DIGITIZER_HPP

#include "ap240_global.hpp"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <cstdint>
#include <functional>
#include <vector>
#include <memory>

namespace adcontrols { class ControlMethod; }
namespace adportable { class TimeSquaredScanLaw; }

#if defined _MSC_VER
# pragma warning(disable:4251)
#endif

namespace ap240 {

    namespace detail { class task; struct device_ap240; }

    class AP240SHARED_EXPORT identify {
    public:
        identify();
        identify( const identify& );
        uint32_t bus_number_;
        uint32_t slot_number_;
        uint32_t serial_number_;
    private:
        friend class boost::serialization::access;
        template<class Archive>
            void serialize( Archive& ar, const unsigned int version ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( serial_number_ );
        }
    };

	class /* AP240SHARED_EXPORT */ threshold_method {
    public:
        bool enable;
        double threshold; // mV
        bool sgFilter;
        int sgPoints;
        threshold_method() : enable( false ), threshold( 100 ), sgFilter( false ), sgPoints( 5 ) {
        }
        friend class boost::serialization::access;
        template<class Archive>
        void serialize( Archive& ar, const unsigned int version ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( enable );
            ar & BOOST_SERIALIZATION_NVP( threshold );
            ar & BOOST_SERIALIZATION_NVP( sgFilter );
            ar & BOOST_SERIALIZATION_NVP( sgPoints );
        }
    };

	class /* AP240SHARED_EXPORT */ method {
    public:
        struct trigger_method {
            uint32_t trigClass;
            uint32_t trigPattern;
            uint32_t trigCoupling;
            uint32_t trigSlope;
            double trigLevel1;
            double trigLevel2;
            trigger_method() : trigClass( 0 ) // edge trigger
                             , trigPattern( 0x80000000 ) // Ext 1
                             , trigCoupling( 0 ) // DC
                             , trigSlope( 0 ) // positive
                             , trigLevel1( 1000.0 ) // mV for Ext, %FS for CHn
                             , trigLevel2( 0.0 )    // only if window for trigSlope (3)
                {}
        private:
            friend class boost::serialization::access;
            template<class Archive>
            void serialize( Archive& ar, const unsigned int version ) {
                using namespace boost::serialization;
                ar & BOOST_SERIALIZATION_NVP( trigClass );
                ar & BOOST_SERIALIZATION_NVP( trigPattern );
                ar & BOOST_SERIALIZATION_NVP( trigCoupling );
                ar & BOOST_SERIALIZATION_NVP( trigSlope );
                ar & BOOST_SERIALIZATION_NVP( trigLevel1 );
                ar & BOOST_SERIALIZATION_NVP( trigLevel2 );
            }
        };
        
        struct horizontal_method {
            double sampInterval;
            double delay;
            double width;
            uint32_t mode;  // configMode, 0: normal, 2: averaging
            uint32_t flags; // configMode, if mode == 0, 0: normal, 1: start on trigger
            uint32_t nbrAvgWaveforms;
            uint32_t nStartDelay;
            uint32_t nbrSamples;
            horizontal_method() : sampInterval( 0.5e-9 )
                                , delay( 0.0e-6 )
                                , width( 10.0e-6 )
                                , mode( 0 )
                                , flags( 0 )
                                , nbrAvgWaveforms( 1 )
                                , nStartDelay( 0 )
                                , nbrSamples( 0 ) // filled when apply to device
                {}
        private:
            friend class boost::serialization::access;
            template<class Archive>
            void serialize( Archive& ar, const unsigned int version ) {
                using namespace boost::serialization;
                ar & BOOST_SERIALIZATION_NVP( sampInterval );
                ar & BOOST_SERIALIZATION_NVP( delay );
                ar & BOOST_SERIALIZATION_NVP( width );
                ar & BOOST_SERIALIZATION_NVP( mode );
                ar & BOOST_SERIALIZATION_NVP( flags );
                ar & BOOST_SERIALIZATION_NVP( nbrAvgWaveforms );
                ar & BOOST_SERIALIZATION_NVP( nStartDelay );
                ar & BOOST_SERIALIZATION_NVP( nbrSamples );
            }
        };        

        struct vertical_method {
            double fullScale;
            double offset;
            uint32_t coupling;
            uint32_t bandwidth;
            bool invertData;
            bool autoScale;
            vertical_method() : fullScale( 5.0 )
                              , offset( 0.0 )
                              , coupling( 3 )
                              , bandwidth( 2 )
                              , invertData( false )
                              , autoScale( true )
                {}
        private:
            friend class boost::serialization::access;
            template<class Archive>
            void serialize( Archive& ar, const unsigned int version ) {
                using namespace boost::serialization;
                ar & BOOST_SERIALIZATION_NVP( fullScale );
                ar & BOOST_SERIALIZATION_NVP( offset );
                ar & BOOST_SERIALIZATION_NVP( coupling );
                ar & BOOST_SERIALIZATION_NVP( bandwidth );
                ar & BOOST_SERIALIZATION_NVP( invertData );
                ar & BOOST_SERIALIZATION_NVP( autoScale );
            }
        };        
        
        method() : channels_( 0x01 )
        { }
        method( const method& t ) : channels_( t.channels_ )
                                  , hor_( t.hor_ )
                                  , trig_( t.trig_ )
                                  , ext_( t.ext_ )
                                  , ch1_( t.ch1_ )
                                  , ch2_( t.ch2_ )
            { }
        uint32_t channels_;
        horizontal_method hor_;
        trigger_method trig_;
        vertical_method ext_;
        vertical_method ch1_;
        vertical_method ch2_;
        
    private:
        friend class boost::serialization::access;
        template<class Archive>
            void serialize( Archive& ar, const unsigned int version ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( channels_ );            
            ar & BOOST_SERIALIZATION_NVP( trig_ );
            ar & BOOST_SERIALIZATION_NVP( hor_ );
            ar & BOOST_SERIALIZATION_NVP( ext_ );
            ar & BOOST_SERIALIZATION_NVP( ch1_ );
            ar & BOOST_SERIALIZATION_NVP( ch2_ );
        }
    };

    class /* AP240SHARED_EXPORT */ metadata {
    public:
        metadata() : initialXTimeSeconds( 0 )
                   , actualPoints( 0 )
                   , flags( 0 )
                   , actualAverages( 0 )
                   , initialXOffset( 0 )
                   , scaleFactor( 0 )
                   , scaleOffset(0)
                   , horPos( 0 )
                   , indexFirstPoint(0)
                   , channel( 1 )
                   , dataType( 1 )
            { }
        int64_t actualPoints;
        int32_t flags;           // IO pin states
        int32_t actualAverages;  // 0 = digitizer data, 1..n averaged data
        int32_t indexFirstPoint; // firstValidPoint in U5303A
        int16_t channel;         // 1|2
        int16_t dataType;        // 1, 2, 4 := int8_t, int16_t, int32_t
        double initialXTimeSeconds; 
        double initialXOffset;
        double xIncrement;
        double scaleFactor;
        double scaleOffset;
        double horPos;
    private:
        friend class boost::serialization::access;
        template<class Archive>
            void serialize( Archive& ar, const unsigned int version ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( actualPoints );
            ar & BOOST_SERIALIZATION_NVP( flags );            
            ar & BOOST_SERIALIZATION_NVP( actualAverages );
            ar & BOOST_SERIALIZATION_NVP( indexFirstPoint );
            ar & BOOST_SERIALIZATION_NVP( channel );
            ar & BOOST_SERIALIZATION_NVP( dataType );
            ar & BOOST_SERIALIZATION_NVP( initialXTimeSeconds );
            ar & BOOST_SERIALIZATION_NVP( initialXOffset );
            ar & BOOST_SERIALIZATION_NVP( xIncrement );
            ar & BOOST_SERIALIZATION_NVP( scaleFactor );
            ar & BOOST_SERIALIZATION_NVP( scaleOffset );
            ar & BOOST_SERIALIZATION_NVP( horPos );
        }
    };
    
	class AP240SHARED_EXPORT waveform : public std::enable_shared_from_this< waveform > {
		waveform( const waveform& ); // = delete;
		void operator = ( const waveform& ); // = delete;
	public:
        waveform( std::shared_ptr< identify >& id ) : ident_( id ), wellKnownEvents_( 0 ), serialnumber_( 0 ), timeSinceEpoch_( 0 ) {
        }

        size_t size() const;
        template<typename T> const T* begin() const;
        template<typename T> const T* end() const;

        std::pair<double,int> operator [] ( size_t ) const;
        double toVolts( int ) const;
        double toVolts( double ) const;        
        
        method method_;
        metadata meta_;
        uint32_t serialnumber_;
        uint32_t wellKnownEvents_;
        uint64_t timeSinceEpoch_;
        std::shared_ptr< identify > ident_;
    private:
        std::vector< int32_t > d_;
        friend struct detail::device_ap240;
    };
    
    template<> AP240SHARED_EXPORT const int8_t * waveform::begin() const;
    template<> AP240SHARED_EXPORT const int8_t * waveform::end() const;    
    template<> AP240SHARED_EXPORT const int16_t * waveform::begin() const;
    template<> AP240SHARED_EXPORT const int16_t * waveform::end() const;    
    template<> AP240SHARED_EXPORT const int32_t * waveform::begin() const;
    template<> AP240SHARED_EXPORT const int32_t * waveform::end() const;    

	class AP240SHARED_EXPORT device_data {
    public:
        identify ident;
        metadata meta;
    private:
        friend class boost::serialization::access;
        template<class Archive>
            void serialize( Archive& ar, const unsigned int ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( ident );
            ar & BOOST_SERIALIZATION_NVP( meta );           
        }
    };
    

    class AP240SHARED_EXPORT digitizer {
    public:
        digitizer();
        ~digitizer();

        bool peripheral_initialize();
        bool peripheral_prepare_for_run( const ap240::method& );
        bool peripheral_run();
        bool peripheral_stop();
        bool peripheral_trigger_inject();
        bool peripheral_terminate();
        void setScanLaw( std::shared_ptr< adportable::TimeSquaredScanLaw > );

        typedef std::function< void( const std::string, const std::string ) > command_reply_type;
        typedef std::function< bool( const waveform *, const waveform *, ap240::method& ) > waveform_reply_type;

        void connect_reply( command_reply_type ); // method,reply
        void disconnect_reply( command_reply_type );

        void connect_waveform( waveform_reply_type );
        void disconnect_waveform( waveform_reply_type );
        //-------
        // bool findDevice();
        // bool initial_setup();
        // bool acquire();
        // bool stop();
        enum result_code { success, error_timeout, error_overload, error_io_read, error_stopped };
        result_code waitForEndOfAcquisition( size_t timeout );
    };

}

BOOST_CLASS_VERSION( ap240::method, 1 )
BOOST_CLASS_VERSION( ap240::metadata, 1 )
BOOST_CLASS_VERSION( ap240::identify, 1 )

#endif
