/**************************************************************************
** Copyright (C) 2010-2022 MS-Cheminformatics LLC
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

#include "adcontrols/adcontrols_global.h"
#include <boost/optional.hpp>
#include <boost/serialization/version.hpp>
#include <boost/variant.hpp>
#include <boost/json/value_to.hpp>
#include <boost/json/value_from.hpp>
#include <cstdint>
#include <vector>
#include <string>

namespace boost { namespace serialization { class access; } }

namespace adcontrols {

    template<typename T> class TofProtocol_archive;

    //--
    class ADCONTROLSSHARED_EXPORT TofProtocol {
    public:
        typedef boost::variant< int32_t, double > additional_value_type;
        typedef std::pair< double, double > delay_pulse_type;

        enum MULTUM_PULSE_CONFIG { MULTUM_PUSH, MULTUM_INJECT, MULTUM_EXIT, MULTUM_GATE_0, MULTUM_GATE_1, EXT_ADC_TRIG };

        std::vector< delay_pulse_type >& delay_pulses();
        const std::vector< delay_pulse_type >& delay_pulses() const;
        void set_delay_pulses( std::vector< delay_pulse_type >&& );

        uint32_t number_of_triggers() const;
        void setNumber_of_triggers( uint32_t );

        uint32_t mode() const;
        void setMode( uint32_t );

        std::vector< std::pair< int32_t, additional_value_type > >& additionals();
        const std::vector< std::pair< int32_t, additional_value_type > >& additionals() const;

        std::vector< std::string >& formulae();
        const std::vector< std::string >& formulae() const;

        void setDevicedata( const std::string& );
        const std::string& devicedata() const;

        void setReference( uint32_t );
        uint32_t reference() const;

        void setMassRange( double lower, double upper );
        std::pair< double, double > massRange() const;

        void setDigitizerDelayWidth( std::pair< double, double >&& );
        const std::pair<double, double>& digitizerDelayWidth() const;  // actual tof start will be "delay_pulse()[EXT_ADC_TRIG].first + digitizerRange().first"
        void setIndex( int );
        int index() const;

    private:
        double lower_mass_;
        double upper_mass_;
        uint32_t mode_;                         // analyzer mode, 'number of laps' for multum; or "linear|reflectron" mode // V1
        uint32_t number_of_triggers_;           // 0 if averager mode
        std::pair< double, double > digitizer_delay_width_;
        std::vector< delay_pulse_type > delay_pulses_;
        std::vector< std::pair< int32_t, additional_value_type > > additionals_;
        uint32_t reference_;                    // lock mass reference (bit position indicate which formula in formulae
        std::vector< std::string > formulae_;   // formula list, separate with ';'
        std::string devicedata_;                // device specific data
        int index_;
    public:
        TofProtocol();
        TofProtocol( const TofProtocol& t );
    private:
        friend class boost::serialization::access;
        template< class Archive >
            void serialize( Archive& ar, const unsigned int version );

        friend class TofProtocol_archive < TofProtocol > ;
        friend class TofProtocol_archive < const TofProtocol > ;
        friend ADCONTROLSSHARED_EXPORT void tag_invoke( boost::json::value_from_tag, boost::json::value&, const TofProtocol& );
        friend ADCONTROLSSHARED_EXPORT TofProtocol tag_invoke( boost::json::value_to_tag< TofProtocol >&, const boost::json::value& jv );
    };

    // TofProtocol
    ADCONTROLSSHARED_EXPORT
    void tag_invoke( boost::json::value_from_tag, boost::json::value&, const TofProtocol& );

    ADCONTROLSSHARED_EXPORT
    TofProtocol tag_invoke( boost::json::value_to_tag< TofProtocol >&, const boost::json::value& jv );
};

BOOST_CLASS_VERSION( adcontrols::TofProtocol, 2 )
