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

#include "acqrscontrols_global.hpp"
#include <boost/serialization/version.hpp>
#include <boost/uuid/uuid.hpp>
#include <cstdint>
#include <memory>

namespace boost { namespace serialization { class access; } }

namespace acqrscontrols {
namespace aqdrv4 {

    enum SubMethodType : unsigned int;

    enum SubMethodType : unsigned int {
        allMethod
        , triggerMethod
        , horizontalMethod
        , ch1VerticalMethod
        , ch2VerticalMethod
        , extVerticalMethod
    };

    struct ACQRSCONTROLSSHARED_EXPORT trigger_method {
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
    
    struct ACQRSCONTROLSSHARED_EXPORT horizontal_method {
        double sampInterval;
        double delayTime;    // digitizer mode can be negative
        uint32_t nbrSamples;
        uint32_t mode;  // configMode, 0: normal, 2: averaging
        uint32_t flags; // configMode, if mode == 0, 0: normal, 1: start on trigger
        uint32_t nbrAvgWaveforms;

        double width() const { return sampInterval * nbrSamples; }
        
        horizontal_method() : sampInterval( 0.5e-9 )
                            , delayTime( 0.0 )
                            , nbrSamples( 10000 ) // filled when apply to device                              
                            , mode( 0 )
                            , flags( 0 )
                            , nbrAvgWaveforms( 1 )
            {}
    private:
        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int version );
    };

    struct ACQRSCONTROLSSHARED_EXPORT vertical_method {
        double fullScale;
        double offset;
        uint32_t coupling;
        uint32_t bandwidth;
        bool invertData;
        bool autoScale;
        bool enable;
        vertical_method() : fullScale( 5.0 )
                          , offset( 0.0 )
                          , coupling( 3 )
                          , bandwidth( 2 )
                          , invertData( false )
                          , autoScale( true )
                          , enable( true )
            {}
        void set_fullScale( double );
        void set_offset( double );
        void set_coupling( uint32_t );
        void set_bandwidth( uint32_t );
        void set_invertData( bool );
        void set_enable( bool );
    private:
        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int version );
    };
    

    class ACQRSCONTROLSSHARED_EXPORT acqiris_method {
    public:
        acqiris_method();
        acqiris_method( const acqiris_method& t );
        
        static const char * modelClass() { return "AqDrv4"; };
        static const char * itemLabel() { return "AqDrv4"; };
        static const boost::uuids::uuid& clsid();
        
        enum class DigiMode : uint32_t { Digitizer = 0, Averager = 2 };

        std::shared_ptr< trigger_method > mutable_trig();
        std::shared_ptr< horizontal_method > mutable_hor();
        std::shared_ptr< vertical_method > mutable_ext();
        std::shared_ptr< vertical_method > mutable_ch1();
        std::shared_ptr< vertical_method > mutable_ch2();

        std::shared_ptr< const trigger_method > trig() const;
        std::shared_ptr< const horizontal_method > hor() const;
        std::shared_ptr< const vertical_method > ext() const;
        std::shared_ptr< const vertical_method > ch1() const;
        std::shared_ptr< const vertical_method > ch2() const;
        
        boost::uuids::uuid clsid_;
        std::shared_ptr< trigger_method > trig_;
        std::shared_ptr< horizontal_method > hor_;
        std::shared_ptr< vertical_method > ext_;
        std::shared_ptr< vertical_method > ch1_;
        std::shared_ptr< vertical_method > ch2_;
    private:
        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int version );
    };
}
}

BOOST_CLASS_VERSION( acqrscontrols::aqdrv4::trigger_method, 1 )
BOOST_CLASS_VERSION( acqrscontrols::aqdrv4::horizontal_method, 1 )
BOOST_CLASS_VERSION( acqrscontrols::aqdrv4::vertical_method, 2 )
BOOST_CLASS_VERSION( acqrscontrols::aqdrv4::acqiris_method, 1 )
