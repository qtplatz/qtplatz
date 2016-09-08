/**************************************************************************
 ** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
 ** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include "waveform_horizontal.hpp"
#include "waveform.hpp"
#include "metadata.hpp"
#include "method.hpp"
#include <adcontrols/threshold_method.hpp>
#include <adcontrols/countingmethod.hpp>
#include <adportable/threshold_finder.hpp>
#include <adportable/waveform_processor.hpp>
#include <adportable/peak_finder.hpp>
#include <adportable/debug.hpp>
#include <cstdint>

namespace acqrscontrols {

    template< typename waveform_type = u5303a::waveform >
    class find_threshold_peaks {

        const adcontrols::threshold_method& method;
        const adcontrols::CountingMethod& ranges;

    public:
        find_threshold_peaks( const adcontrols::threshold_method& m
                              , const adcontrols::CountingMethod& r ) : method( m )
                                                                      , ranges( r )
            {}
        
        template< typename result_value_type >
        void operator () ( const waveform_type& data
                           , std::vector< result_value_type >& elements
                           , std::vector< double >& processed ) {
            
            // const bool findUp = method.slope == adcontrols::threshold_method::CrossUp;
            assert ( method.algo_ == adcontrols::threshold_method::Deferential );

            double level = ( method.threshold_level - data.meta_.scaleOffset ) / data.meta_.scaleFactor;
            if ( method.use_filter ) {
                waveform_type::apply_filter( processed, data, method );
                level = method.threshold_level;
            }

            // adportable::threshold_finder finder( findUp, nfilter );
            adportable::peak_finder finder( method.slope == adcontrols::threshold_method::CrossUp ); // Pos | Neg

            if ( ranges.enable() ) {

                using adcontrols::CountingMethod;
                
                for ( const auto& v: ranges ) {
                    if ( std::get< CountingMethod::CountingEnable >( v ) ) {
                        const auto& time_range = std::get< CountingMethod::CountingRange >( v ); // center, width
                        auto offs = waveform_horizontal().range( data.method_, data.meta_, time_range );
                        if ( offs.second ) {
                            size_t eoffs = offs.first + offs.second;
                            adportable::average averager;
                            if ( method.use_filter ) {
                                finder( processed.begin(), processed.begin() + eoffs, elements, level, offs.first  );
                            } else if ( data.meta_.dataType == 2 ) {
                                finder( data.template begin<int16_t>(), data.template begin< int16_t >() + eoffs, elements, level, offs.first );
                            } else if ( data.meta_.dataType == 4 ) {
                                finder( data.template begin<int32_t>(), data.template begin<int32_t>() + eoffs, elements, level, offs.first );
                            }
                        }
                    }
                }

            } else {
                if ( method.use_filter ) 
                    finder( processed.begin(), processed.end(), elements, level );        
                else if ( data.meta_.dataType == 2 )
                    finder( data.template begin<int16_t>(), data.template end<int16_t>(), elements, level );
                else if ( data.meta_.dataType == 4 )
                    finder( data.template begin<int32_t>(), data.template end<int32_t>(), elements, level );
            }
        }
    };
}

