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

#pragma once

#include "acqrscontrols_global.hpp"
#include "threshold_result.hpp"
#include "ap240/tdcdoc.hpp"
#include "u5303a/tdcdoc.hpp"
#include <acqrscontrols/constants.hpp>
#include <adcontrols/controlmethod/tofchromatogrammethod.hpp>
#include <adcontrols/controlmethod/tofchromatogramsmethod.hpp>
#include <adportable/waveform_wrapper.hpp>
#include <adportable/spectrum_processor.hpp>
#include <algorithm>
#include <atomic>
#include <array>
#include <mutex>
#include <memory>
#include <vector>
#include <functional>

namespace adportable { struct threshold_index; }

namespace acqrscontrols { namespace u5303a { class waveform; class threshold_result; class histogram; class tdcdoc; } }
namespace acqrscontrols { namespace ap240 { class waveform; class threshold_result; class histogram; class tdcdoc; } }

namespace adcontrols {
    class threshold_action;
    class threshold_method;
    class MassSpectrometer;
    class MassSpectrum;
    class TofChromatogramsMethod;
    class TimeDigitalHistogram;
    class CountingMethod;
}

namespace acqrscontrols {

    template< typename waveform_type > struct tdcdoc_type {};
    template<> struct tdcdoc_type< u5303a::waveform > { typedef u5303a::tdcdoc _type; };
    template<> struct tdcdoc_type< ap240::waveform > { typedef ap240::tdcdoc _type; };
    
    template< typename waveform_type > struct threshold_result_type {};
    template<> struct threshold_result_type< u5303a::waveform > { typedef u5303a::threshold_result _type; };
    template<> struct threshold_result_type< ap240::waveform > { typedef ap240::threshold_result _type; };

    template< typename waveform_type > struct histogram_type {};
    template<> struct histogram_type< u5303a::waveform > { typedef u5303a::histogram _type; };
    template<> struct histogram_type< ap240::waveform > { typedef ap240::histogram _type; };

    class MakeChromatogramPoints {
    public:
        template< typename waveform_type >
        bool operator()( std::shared_ptr< const waveform_type > waveform
                         , const adcontrols::TofChromatogramsMethod& method
                         , std::vector< std::pair< uint32_t, double > >& values ) {
            values.clear();

            const double xIncrement = waveform->meta_.xIncrement;
            auto wform = adportable::waveform_wrapper< int32_t, waveform_type >( *waveform );

            for ( auto& trace: method ) {
                if ( trace.enable() && trace.protocol() == waveform->meta_.protocolIndex ) {
                    if ( trace.intensityAlgorithm() == adcontrols::TofChromatogramMethod::ePeakAreaOnProfile ) {
                        double a = waveform->accumulate( trace.time(), trace.timeWindow() );
                        values.emplace_back( trace.id(), a );
                    } else if ( trace.intensityAlgorithm() == adcontrols::TofChromatogramMethod::ePeakHeightOnProfile ) {
                        double rms(0), dbase(0);
                        adportable::spectrum_processor::tic( wform.size(), wform.begin(), dbase, rms, 7 );
                        if ( trace.time() < 1.0e-9 ) {
                            // base peak height
                            double v = waveform->toVolts( *std::max_element( wform.begin(), wform.end() ) - dbase ) * 1000; // mV
                            values.emplace_back( trace.id(), v );
                        } else {
                            // base peak in time range
                            if ( *wform.begin() < trace.time() && trace.time() < *(wform.end() - 1) ) {
                                size_t i0 = ( ( trace.time() - trace.timeWindow() / 2 ) - *wform.begin() ) / xIncrement;
                                size_t i1 = ( ( trace.time() + trace.timeWindow() / 2 ) - *wform.begin() ) / xIncrement + 0.5;
                                double v = waveform->toVolts( *std::max_element( wform.begin() + i0, wform.begin() + i1 ) - dbase ) * 1000;
                                values.emplace_back( trace.id(), v );
                            }
                        }
                    }
                }
            }
            return true;
        }
    };

}
