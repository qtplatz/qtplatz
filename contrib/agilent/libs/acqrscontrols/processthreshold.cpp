/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "processthreshold.hpp"
#include "constants.hpp"
#include "find_threshold_peaks.hpp"
#include "find_threshold_timepoints.hpp"
#include "tdcbase.hpp"
#include "threshold_result.hpp"
#include "threshold_action_finder.hpp"
#include "ap240/waveform.hpp"
#include "u5303a/waveform.hpp"
#include "ap240/tdcdoc.hpp"
#include "u5303a/tdcdoc.hpp"
#include <boost/format.hpp>
#include <type_traits>

namespace acqrscontrols {

    namespace impl {

        template< unsigned int algo >
        class processThreshold {
        public:

            template< typename waveform_type, size_t nchannels >
            std::array< std::shared_ptr< threshold_result_< waveform_type > >, nchannels >
            operator()( std::array< std::shared_ptr< const waveform_type >, nchannels > waveforms
                        , acqrscontrols::tdcbase& t ) const {
                return {{ 0 }};
            }
        };


        ///////////////////////////// implementation //////////////////////////
        template<>
        template< typename waveform_type, size_t nchannels >
        std::array< std::shared_ptr< threshold_result_< waveform_type > >, nchannels >
        processThreshold<3>::operator()( std::array< std::shared_ptr< const waveform_type >, nchannels > waveforms
                                         , acqrscontrols::tdcbase& tdc ) const {

            typedef std::shared_ptr< threshold_result_< waveform_type > > threshold_result_ptr;

            typedef typename std::conditional< std::is_same< waveform_type, ap240::waveform >::value
                                               , threshold_result_< ap240::waveform >
                                               , threshold_result_< u5303a::waveform > >::type threshold_result_type;

            if ( ! waveforms[0] && ! waveforms[1] ) // empty
                return std::array< threshold_result_ptr, nchannels >();

            std::array< threshold_result_ptr, nchannels > results;

            auto counting_method = tdc.countingMethod();
            auto threshold_action = tdc.threshold_action();

            for ( size_t i = 0; i < waveforms.size(); ++i ) {

                if ( waveforms[ i ] ) {

                    auto counts = tdc.threshold_action_counts( i );
                    auto threshold_method = tdc.threshold_method( i );

                    results[ i ] = std::make_shared< threshold_result_type >( waveforms[ i ] );
                    results[ i ]->setFindUp( threshold_method->slope == adcontrols::threshold_method::CrossUp );
                    results[ i ]->set_threshold_level( threshold_method->threshold_level );
                    results[ i ]->set_algo( static_cast< enum adportable::counting::counting_result::algo >( threshold_method->algo_ ) );

                    ADDEBUG() << "Threshold_level: " << threshold_method->threshold_level;

                    const auto idx = waveforms[ i ]->method_.protocolIndex();
                    if ( idx == 0 )
                        counts.second++;

                    if ( threshold_method && threshold_method->enable ) {

                        if ( threshold_method->algo_ == adcontrols::threshold_method::Differential ) {
                            if  ( threshold_method->slope == adcontrols::threshold_method::CrossUp ) {
                                acqrscontrols::find_threshold_peaks< true, waveform_type > find_peaks( *threshold_method, *counting_method );
                                find_peaks( *waveforms[ i ], *results[ i ], results[ i ]->processed() );
                            } else {
                                acqrscontrols::find_threshold_peaks< false, waveform_type > find_peaks( *threshold_method, *counting_method );
                                find_peaks( *waveforms[ i ], *results[ i ], results[ i ]->processed() );
                            }
                        } else {
                            acqrscontrols::find_threshold_timepoints< waveform_type > find_threshold( *threshold_method, *counting_method );
                            find_threshold( *waveforms[ i ], *results[ i ], results[ i ]->processed() );
                        }

                        // copy from vector< adportable::threshold_index > ==> vector< uint32_t > for compatibility
                        results[ i ]->indices().resize( results[ i ]->indices2().size() );
                        std::transform( results[ i ]->indices2().begin()
                                        , results[ i ]->indices2().end()
                                        , results[ i ]->indices().begin()
                                        , []( const adportable::counting::threshold_index& a ){ return a.apex; } ); // <-- apex

                        bool result = acqrscontrols::threshold_action_finder()( results[i], threshold_action );

                        if ( result )
                            counts.first++;
                        tdc.set_threshold_action_counts( i, counts );
                    }
                }
            }

            return results;
        }
        //////////
    }


    //////////////////////////////////
    template<>
    template<>
    std::array< std::shared_ptr< threshold_result_< ap240::waveform > >, ap240::nchannels >
    processThreshold_<3>::operator()< ap240::waveform >(
        std::array< std::shared_ptr< const ap240::waveform >, ap240::nchannels > waveforms
        , acqrscontrols::tdcbase& tdc ) const {

        return impl::processThreshold<3>()( waveforms, tdc );

    }

    //////////////////////////////////
    template<>
    template<>
    std::array< std::shared_ptr< threshold_result_< u5303a::waveform > >, u5303a::nchannels >
    processThreshold_<3>::operator()< u5303a::waveform >(
        std::array< std::shared_ptr< const u5303a::waveform >, u5303a::nchannels > waveforms
        , acqrscontrols::tdcbase& tdc ) const {

        return impl::processThreshold<3>()( waveforms, tdc );

    }
}
