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

#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/samplinginfo.hpp>
#include <adcontrols/threshold_method.hpp>
#include <adcontrols/countingmethod.hpp>
#include <adportable/average.hpp>
#include <adportable/counting/counting_result.hpp>
#include <adportable/counting/threshold_finder.hpp>
#include <adportable/waveform_processor.hpp>
#include <adportable/debug.hpp>
#include <cstdint>

namespace adcontrols {

    class find_threshold_timepoints {
        const adcontrols::threshold_method& method;
        const adcontrols::CountingMethod& ranges;
    public:
        find_threshold_timepoints( const adcontrols::threshold_method& _method
                                   , const adcontrols::CountingMethod& _ranges ) : method( _method )
                                                                                 , ranges( _ranges )
            {}

        template< typename T = MassSpectrum >
        void operator () ( const T& data
                           , adportable::counting::counting_result& result ) {

            const bool findUp = method.slope == adcontrols::threshold_method::CrossUp;

            const unsigned int nfilter = static_cast<unsigned int>( method.response_time / data.getMSProperty().samplingInfo().fSampInterval() ) | 01;
            double level = method.threshold_level;

            result.set_algo( static_cast< enum adportable::counting::algo >( method.algo_ ) );
            result.set_threshold_level( method.threshold_level );

            auto& elements = result.indices2();

            adportable::counting::threshold_finder finder( findUp, nfilter );

            const bool average( method.algo_ == adcontrols::threshold_method::AverageRelative );
            adportable::stddev stddev;

            if ( average && method.use_filter ) {
                auto sd = stddev( data.getIntensityArray(), data.size() );
                finder( data.getIntensityArray(), data.getIntensityArray() + data.size(), elements, level + sd.second, 0 );
            } else {
                finder( data.getIntensityArray(), data.getIntensityArray() + data.size(), elements, level, 0 );
            }
        }
    };
}
