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

#pragma once

#include "../acqrscontrols_global.hpp"
#include "../acqiris_method.hpp"
#include <adcontrols/threshold_action.hpp>
#include <adcontrols/threshold_method.hpp>
#include <adcontrols/tofprotocol.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <cstdint>
#include <functional>
#include <iostream>
#include <vector>
#include <memory>

namespace boost { namespace uuids { struct uuid; } }

namespace acqrscontrols {
    namespace ap240 {

        template< typename T >  class method_archive;

        struct ACQRSCONTROLSSHARED_EXPORT trigger_method : public aqdrv4::trigger_method {
            // uint32_t trigClass;
            // uint32_t trigPattern;
            // uint32_t trigCoupling;
            // uint32_t trigSlope;
            // double trigLevel1;
            // double trigLevel2;
            // trigger_method() : trigClass( 0 ) // edge trigger
            //     , trigPattern( 0x80000000 ) // Ext 1
            //     , trigCoupling( 0 ) // DC
            //     , trigSlope( 0 ) // positive
            //     , trigLevel1( 1000.0 ) // mV for Ext, %FS for CHn
            //     , trigLevel2( 0.0 )    // only if window for trigSlope (3)
            // {}
        private:
            // friend class boost::serialization::access;
            // template<class Archive> void serialize( Archive& ar, const unsigned int version );
        };

        struct ACQRSCONTROLSSHARED_EXPORT horizontal_method {
            double sampInterval;
            double delay;
            double width;
            uint32_t mode;  // configMode, 0: normal, 2: averaging
            uint32_t flags; // configMode, if mode == 0, 0: normal, 1: start on trigger
            uint32_t nbrAvgWaveforms;
            int32_t  nStartDelay; // digitizer mode can be negative
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

        struct ACQRSCONTROLSSHARED_EXPORT vertical_method : public aqdrv4::vertical_method {
            // double fullScale;
            // double offset;
            // uint32_t coupling;
            // uint32_t bandwidth;
            // bool invertData;
            // bool autoScale;
            // vertical_method() : fullScale( 5.0 )
            //     , offset( 0.0 )
            //     , coupling( 3 )
            //     , bandwidth( 2 )
            //     , invertData( false )
            //     , autoScale( true )
            // {}
        private:
            // friend class boost::serialization::access;
            // template<class Archive>  void serialize( Archive& ar, const unsigned int version );
        };


        class ACQRSCONTROLSSHARED_EXPORT method {
        public:
            method();
            method( const method& t );

            static const char * modelClass() { return "ap240"; };
            static const char * itemLabel() { return "ap240"; };
            static const boost::uuids::uuid& clsid();

            enum class DigiMode : uint32_t { Digitizer = 0, Averager = 2 };

            uint32_t channels_;
            horizontal_method hor_;
            trigger_method trig_;
            vertical_method ext_;
            vertical_method ch1_;
            vertical_method ch2_;
            adcontrols::threshold_method slope1_;
            adcontrols::threshold_method slope2_;
            adcontrols::threshold_action action_;

            DigiMode mode() const;

            uint32_t protocolIndex() const;
            bool setProtocolIndex( uint32_t, bool modifyDeviceMethod );

            std::vector< adcontrols::TofProtocol >& protocols();
            const std::vector< adcontrols::TofProtocol >& protocols() const;

            static bool archive( std::ostream&, const method& );
            static bool restore( std::istream&, method& );
            static bool xml_archive( std::wostream&, const method& );
            static bool xml_restore( std::wistream&, method& );

        private:
            // 2016-JUN-13 added for u5303a compatibility
            uint32_t protocolIndex_;
            std::vector< adcontrols::TofProtocol > protocols_;

            friend class method_archive< method >;
            friend class method_archive< const method >;

            friend class boost::serialization::access;
            template<class Archive> void serialize( Archive& ar, const unsigned int version );
        };

    }
}

BOOST_CLASS_VERSION( acqrscontrols::ap240::method, 2 )
