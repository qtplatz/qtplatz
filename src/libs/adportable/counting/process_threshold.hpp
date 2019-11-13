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

#include "counting_result.hpp"
#include "constants.hpp"
#include <adportable/find_threshold_peaks.hpp>
#include <adportable/find_threshold_timepoints.hpp>
#include <memory>

namespace adportable {
    namespace counting {

        template< typename waveform_t, typename pkd_result_t >
        class process_threshold {
        public:

            std::shared_ptr< pkd_result_t > operator()( std::shared_ptr< const waveform_t > waveform ) {

                auto result = std::make_shared< pkd_result_t >( waveform );
                result->set_threshold_level( dlevel_ );

                if ( algo_ == adportable::counting::Differential ) {
                    if  ( slope_ == adportable::counting::CrossUp )
                        adportable::find_threshold_peaks< true, waveform_t >()( level_, *waveform, *result );
                    else
                        adportable::find_threshold_peaks< false, waveform_t >()( level_, *waveform, *result );
                } else {
                    // Absolute || AverageRelative
                    adportable::find_threshold_timepoints< waveform_t >()( slope_ == CrossUp, level_, *waveform, *result );
                }
                return result;
            }

            void set_threshold_level( double dlevel, typename waveform_t::value_type level ) {
                dlevel_ = dlevel;
                level_ = level;
            }

            void set_algo( counting::algo algo ) {
                algo_ = algo;
            }

            void set_slope( counting::Slope slope ) {
                slope_ = slope;
            }

            process_threshold( double dlevel = 0
                               , typename waveform_t::value_type level = 0
                               , adportable::counting::Slope slope = CrossDown
                               , adportable::counting::algo algo = Absolute ) : dlevel_( dlevel )
                                                                              , level_( level )
                                                                              , slope_( CrossDown )
                                                                              , algo_( Absolute ) {
            }

        private:
            double dlevel_;
            typename waveform_t::value_type level_;
            adportable::counting::Slope slope_;
            adportable::counting::algo algo_;
        };

    }
}
