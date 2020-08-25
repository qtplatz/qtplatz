/**************************************************************************
** Copyright (C) 2010-2020 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2020 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "aqmd3controls_global.hpp"
#include <boost/serialization/version.hpp>
#include <boost/property_tree/ptree_fwd.hpp>
#include <cstdint>
#include <ostream>

namespace boost { namespace serialization { class access; } }

namespace aqmd3controls {

    class AQMD3CONTROLSSHARED_EXPORT device_method;

    class device_method {
    public:
        device_method();

        // no copy ctor necessary since all members are POD
        double front_end_range;
        double front_end_offset;
        double ext_trigger_level;
        double samp_rate; // HZ
        int32_t nbr_of_s_to_acquire_;
        int32_t nbr_of_averages;
        double delay_to_first_sample_;
        int32_t invert_signal;
        uint32_t nsa_threshold;

        double digitizer_delay_to_first_sample; // actual delay set to u5303a
        uint32_t digitizer_nbr_of_s_to_acquire; // actual number of samples per waveform

        uint32_t nbr_records; // MultiRecord Acquisition

        bool TSR_enabled;     // Triggered simultaneous acquisition and readout

        bool nsa_enabled;
        bool pkd_enabled;
        int32_t pkd_raising_delta;
        int32_t pkd_falling_delta;
        bool pkd_amplitude_accumulation_enabled;
        static bool write_ptree( boost::property_tree::ptree&, const device_method& );
        static bool write_json( std::ostream&, const device_method& );
    private:

        friend class boost::serialization::access;
        template<class Archive>
        void serialize( Archive& ar, const unsigned int version );
    };

}

BOOST_CLASS_VERSION( aqmd3controls::device_method, 1 )
