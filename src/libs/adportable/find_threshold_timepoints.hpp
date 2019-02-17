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
#include <adportable/counting/basic_histogram.hpp>
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

    template< typename waveform_type >
    class find_threshold_timepoints {
    public:
        find_threshold_timepoints() {}

        template< typename result_type >
        void operator () ( bool findUp
                           , const typename waveform_type::value_type& level
                           , const waveform_type& data
                           , result_type& result // adportable::counting::counting_result& | histogram : waveform< counting::threshold_index >, meta_type >
                           , unsigned int nfilter = 1
                           , enum adportable::counting::algo algo = adportable::counting::Absolute ) const {

            //const unsigned int nfilter = static_cast<unsigned int>( method.response_time / data.meta_.xIncrement ) | 01;

            result.set_algo( algo );
            result.set_threshold_level( level );

            std::vector< counting::threshold_index >& v = result;

            counting::threshold_finder finder( findUp, nfilter );

            if ( algo == adcontrols::threshold_method::AverageRelative ) {
                auto sd = adportable::stddev()( data.begin(), data.size() );
                finder( data.begin(), data.end(), v, level + sd.second );
            } else {
                finder( data.begin(), data.end(), v, level );
            }
        }
    };

}
