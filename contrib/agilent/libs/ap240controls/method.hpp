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

#include "ap240controls_global.hpp"
#include <adcontrols/threshold_method.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <cstdint>
#include <functional>
#include <iostream>
#include <vector>
#include <memory>

namespace ap240spectrometer {
    
    namespace ap240 {

        struct AP240CONTROLSSHARED_EXPORT trigger_method {
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
            template<class Archive> void serialize( Archive& ar, const unsigned int version );
        };

        struct AP240CONTROLSSHARED_EXPORT horizontal_method {
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
            template<class Archive> void serialize( Archive& ar, const unsigned int version );
        };

        struct AP240CONTROLSSHARED_EXPORT vertical_method {
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
            template<class Archive>  void serialize( Archive& ar, const unsigned int version );
        };        
        
        
        class AP240CONTROLSSHARED_EXPORT method {
        public:
            method();
            method( const method& t );
            uint32_t channels_;
            horizontal_method hor_;
            trigger_method trig_;
            vertical_method ext_;
            vertical_method ch1_;
            vertical_method ch2_;
            adcontrols::threshold_method slope1_;
            adcontrols::threshold_method slope2_;

            static bool archive( std::ostream&, const method& );
            static bool restore( std::istream&, method& );
            static bool xml_archive( std::wostream&, const method& );
            static bool xml_restore( std::wistream&, method& );
        
        private:
            friend class boost::serialization::access;
            template<class Archive> void serialize( Archive& ar, const unsigned int version );
        };

    } // ap240
} // ap240spectrometer

namespace ap240x = ap240spectrometer::ap240;
