/**************************************************************************
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
#include <adportable/basic_waveform.hpp>
#include <adportable/counting/threshold_finder.hpp>
#include <adportable/counting/counting_result.hpp>
#include <adportable/counting/peak_finder.hpp>
#include <adcontrols/threshold_method.hpp>
#include <adportable/waveform_processor.hpp>
#include <adportable/debug.hpp>
#include <cstdint>

namespace adportable {

    // Algo = Differntial
    template< bool findPositive, typename T = int32_t >
    class find_threshold_peaks {

    public:
        find_threshold_peaks() {}

        void operator () ( const T& level
                           , const basic_waveform< T >& data
                           , adportable::counting::counting_result& result ) {

            adportable::counting::peak_finder< findPositive >()( data.begin(), data.end(), result.indices2(), level );
            // if ( ranges.enable() ) {
            //     using adcontrols::CountingMethod;
            //     for ( const auto& v: ranges ) {
            //         if ( std::get< CountingMethod::CountingEnable >( v ) ) {
            //             const auto& time_range = std::get< CountingMethod::CountingRange >( v ); // center, width
            //             auto offs = waveform_horizontal().range( data.method_, data.meta_, time_range );
            //             if ( offs.second ) {
            //                 size_t eoffs = offs.first + offs.second;
            //                 if ( method.use_filter ) {
            //                     finder( processed.begin(), processed.begin() + eoffs, result.indices2(), level, offs.first  );
            //                 } else if ( data.meta_.dataType == 2 ) {
            //                     finder( data.template begin<int16_t>(), data.template begin< int16_t >() + eoffs, result.indices2(), level, offs.first );
            //                 } else if ( data.meta_.dataType == 4 ) {
            //                     finder( data.template begin<int32_t>(), data.template begin<int32_t>() + eoffs, result.indices2(), level, offs.first );
            //                 }
            //             }
            //         }
            //     }
        }
    };
}
