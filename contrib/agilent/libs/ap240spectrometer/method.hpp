/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

#pragma once

#include "ap240spectrometer_global.hpp"
#include "threshold_method.hpp"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <cstdint>
#include <functional>
#include <iostream>
#include <vector>
#include <memory>

namespace ap240spectrometer {
    
    namespace ap240x {
        
        class AP240SPECTROMETERSHARED_EXPORT method {
        public:
            struct AP240SPECTROMETERSHARED_EXPORT trigger_method {
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
        
            struct AP240SPECTROMETERSHARED_EXPORT horizontal_method {
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

            struct AP240SPECTROMETERSHARED_EXPORT vertical_method {
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
                , slope1_( t.slope1_ )
                , slope2_( t.slope2_ )
            { }
            uint32_t channels_;
            horizontal_method hor_;
            trigger_method trig_;
            vertical_method ext_;
            vertical_method ch1_;
            vertical_method ch2_;
            ap240spectrometer::algo::threshold_method slope1_;
            ap240spectrometer::algo::threshold_method slope2_;

            static bool archive( std::ostream&, const method& );
            static bool restore( std::istream&, method& );
            static bool xml_archive( std::wostream&, const method& );
            static bool xml_restore( std::wistream&, method& );
        
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
                ar & BOOST_SERIALIZATION_NVP( slope1_ );
                ar & BOOST_SERIALIZATION_NVP( slope2_ );
            }
        };

    } // ap240
} // ap240spectrometer
