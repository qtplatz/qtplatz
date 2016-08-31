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

#include "waveform.hpp"
#include "metadata.hpp"
#include "method.hpp"
#include <adcontrols/threshold_method.hpp>
#include <adcontrols/countingmethod.hpp>
#include <adportable/threshold_finder.hpp>
#include <adportable/debug.hpp>
#include <cstdint>

namespace acqrscontrols {

    struct waveform_horizontal {
        
        template< typename method_type, typename metadata_type >
        inline double delay_time( const method_type& method, const metadata_type& meta ) const {
            auto& pulses = method.protocols()[ method.protocolIndex() ].delay_pulses();
            if ( pulses.size() >= adcontrols::TofProtocol::EXT_ADC_TRIG )
                return meta.initialXOffset + pulses[ adcontrols::TofProtocol::EXT_ADC_TRIG ].first;
            else
                return meta.initialXOffset;
        }

        template< typename method_type, typename metadata_type >
        inline std::pair< size_t, size_t > range( const method_type& method, const metadata_type& meta
                                           , const std::pair< double, double >& time ) const {

            std::pair< size_t, size_t > xrange;

            double delay = delay_time( method, meta );

            ADDEBUG() << "protocol: " << method.protocolIndex() << "/" << method.protocols().size() << " delay: " << delay * std::micro::den;
            
            double t0 = time.first - time.second / 2;
            double t1 = t0 + time.second;
            if ( t1 < delay ) {
                return { 0, 0 };
            } else if ( delay < t0 ) {
                xrange.first = ( t0 - delay ) / meta.xIncrement;
                xrange.second = size_t( ( time.second / meta.xIncrement ) + 0.5 );                
            } else {
                xrange.first = 0;
                xrange.second = size_t( ( ( t1 - delay ) / meta.xIncrement ) + 0.5 ); 
            }
            if ( xrange.first > meta.actualPoints )
                return { 0, 0 };
            
            xrange.second = std::min( xrange.second, meta.actualPoints - xrange.first );

            return xrange;
        }
    };

    template< typename waveform_type = u5303a::waveform >
    class find_threshold_timepoints {
        const adcontrols::threshold_method& method;
        const adcontrols::CountingMethod& ranges;
    public:
        find_threshold_timepoints( const adcontrols::threshold_method& _method
                                   , const adcontrols::CountingMethod& _ranges ) : method( _method )
                                                                                , ranges( _ranges )
            {}

        template< typename result_value_type >
        void operator () ( const waveform_type& data
                           , std::vector< result_value_type >& elements
                           , std::vector< double >& processed ) {
            
            const bool findUp = method.slope == adcontrols::threshold_method::CrossUp;
            const unsigned int nfilter = static_cast<unsigned int>( method.response_time / data.meta_.xIncrement ) | 01;

            double level;
            if ( method.use_filter ) {
                waveform_type::apply_filter( processed, data, method );
                level = method.threshold_level;
            } else {
                size_t nAverages = data.meta_.actualAverages ? data.meta_.actualAverages : 1;
                level = ( ( method.threshold_level - data.meta_.scaleOffset ) / data.meta_.scaleFactor ) * nAverages;
            }

            adportable::threshold_finder finder( findUp, nfilter );

            if ( ranges.enable() ) {

                using adcontrols::CountingMethod;

                for ( const auto& v: ranges ) {
                    if ( std::get< CountingMethod::CountingEnable >( v ) ) {
                        const auto& time_range = std::get< CountingMethod::CountingRange >( v ); // center, width
                        auto offs = waveform_horizontal().range( data.method_, data.meta_, time_range );
                        if ( offs.second ) {
                            size_t eoffs = offs.first + offs.second;
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

