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
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
// #include <cstdint>
// #include <functional>
// #include <iostream>
// #include <vector>
// #include <memory>

namespace ap240spectrometer {

    namespace algo {

        // Method for 'Slope Time Converter'

        class AP240SPECTROMETERSHARED_EXPORT threshold_method {
        public:
            enum FilterAlgo { SG_Filter, DFT_Filter };
            enum Slope { CrossUp, CrossDown };
            bool enable;
            double threshold_level;   // V
            double time_resolution;   // seconds --> for histogram (does not affect for acquiring waveforms)
            double response_time;     // seconds
            Slope slope;              // POS(CrossUp) | NEG(CrossDown)
            bool use_filter;
            FilterAlgo filter;
            double sgwidth;           // SG-smooth width
            double cutoffHz;          // DFT
            bool complex_;

            threshold_method();

            friend class boost::serialization::access;
            template<class Archive>
            void serialize( Archive& ar, const unsigned int version ) {
                using namespace boost::serialization;
                ar & BOOST_SERIALIZATION_NVP( enable );
                ar & BOOST_SERIALIZATION_NVP( threshold_level );
                ar & BOOST_SERIALIZATION_NVP( time_resolution );
                ar & BOOST_SERIALIZATION_NVP( response_time );
                ar & BOOST_SERIALIZATION_NVP( slope );
                ar & BOOST_SERIALIZATION_NVP( use_filter );
                ar & BOOST_SERIALIZATION_NVP( filter );
                ar & BOOST_SERIALIZATION_NVP( sgwidth );
                ar & BOOST_SERIALIZATION_NVP( cutoffHz );
                ar & BOOST_SERIALIZATION_NVP( complex_ );
            }
        };
    }
}

