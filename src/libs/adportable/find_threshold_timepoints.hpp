/**************************************************************************
 ** Copyright (C) 2010-2019 Toshinobu Hondo, Ph.D.
 ** Copyright (C) 2013-2019 MS-Cheminformatics LLC, Toin, Mie Japan
 *
 ** Contact: toshi.hondo@qtplatz.com
 **
 ** Commercial Usage
 **
 ** Licensees holding valid ScienceLiaison commercial licenses may use this file in
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

#pragma once
#include <adportable/counting/counting_result.hpp>
#include <adportable/basic_waveform.hpp>
#include <adcontrols/threshold_method.hpp>
#include <adcontrols/countingmethod.hpp>
#include <adportable/average.hpp>
#include <adportable/counting/counting_result.hpp>
#include <adportable/counting/threshold_finder.hpp>
#include <adportable/waveform_processor.hpp>
#include <adportable/debug.hpp>
#include <cstdint>

namespace adportable {

    template< typename T >
    class find_threshold_timepoints {
    public:
        find_threshold_timepoints() {}

        void operator () ( bool findUp
                           , const T& level
                           , const basic_waveform< T >& data
                           , adportable::counting::counting_result& result
                           , std::vector< double >& processed
                           , unsigned int nfilter = 1
                           , enum adportable::counting::counting_result::algo algo = adportable::counting::counting_result::AverageRelative ) {
            //const unsigned int nfilter = static_cast<unsigned int>( method.response_time / data.meta_.xIncrement ) | 01;

            result.setAlgo( algo );
            result.setThreshold_level( level );

            auto& elements = result.indices2();

            counting::threshold_finder finder( findUp, nfilter );

            adportable::stddev stddev;

            if ( algo == adcontrols::threshold_method::AverageRelative ) {
                auto sd = stddev( data.begin(), data.size() );
                finder( data.begin(), data.end(), elements, level + sd.second );
            } else {
                finder( data.begin(), data.end(), elements, level );
            }
        }
    };
}
