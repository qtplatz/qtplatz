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

#include "waveform.hpp"
#include "metadata.hpp"
#include "method.hpp"
#include <adcontrols/threshold_method.hpp>
#include <adcontrols/countingmethod.hpp>
#include <adportable/counting/threshold_finder.hpp>
#include <adportable/waveform_processor.hpp>
#include <adportable/debug.hpp>
#include <algorithm>
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
        inline std::pair< size_t, size_t > range( const method_type& method
                                                  , const metadata_type& meta
                                                  , const std::pair< double, double >& time ) const {

            std::pair< size_t, size_t > xrange;

            double delay = delay_time( method, meta );

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
            auto x = std::min( 0, 1 );
            xrange.second = std::min( xrange.second, size_t( meta.actualPoints - xrange.first ) );

            return xrange;
        }
    };

}

