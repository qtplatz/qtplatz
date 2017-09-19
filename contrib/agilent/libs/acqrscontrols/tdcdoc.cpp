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

#include "tdcdoc.hpp"

#include <acqrscontrols/acqrscontrols_global.hpp>
#include <acqrscontrols/constants.hpp>
#include <adcontrols/countingmethod.hpp>
#include <atomic>
#include <array>
#include <mutex>
#include <memory>
#include <vector>
#include <functional>

namespace acqrscontrols {

    // typedef acqrscontrols::u5303a::threshold_result threshold_result_type;
    // typedef std::shared_ptr< threshold_result_type > threshold_result_ptr;
    // typedef std::shared_ptr< const threshold_result_type > const_threshold_result_ptr;
    // typedef acqrscontrols::u5303a::waveform waveform_type;
    // typedef acqrscontrols::u5303a::histogram histogram_type;

    template<>
    tdcdoc_< ap240::waveform >::tdcdoc_()
    {
    }

    template<>
    tdcdoc_< ap240::waveform >::~tdcdoc_()
    {
    }
    
    template<> bool
    tdcdoc_< ap240::waveform >::set_threshold_action( const adcontrols::threshold_action& a )
    {
        return ap240::tdcdoc::set_threshold_action( a );
    }
    
    template<> std::shared_ptr< const adcontrols::threshold_action >
    tdcdoc_< ap240::waveform >::threshold_action() const
    {
        return ap240::tdcdoc::threshold_action();
    }

    template<> bool
    tdcdoc_< ap240::waveform >::set_threshold_method( int channel, const adcontrols::threshold_method& a )
    {
        return ap240::tdcdoc::set_threshold_method( channel, a );
    }
    
    template<> std::shared_ptr< const adcontrols::threshold_method >
    tdcdoc_< ap240::waveform >::threshold_method( int channel ) const
    {
        return ap240::tdcdoc::threshold_method( channel );
    }
    
    template<> bool
    tdcdoc_< ap240::waveform >::setTofChromatogramsMethod( const adcontrols::TofChromatogramsMethod& a )
    {
        return ap240::tdcdoc::setTofChromatogramsMethod( a );
    }
    
    template<> std::shared_ptr< const adcontrols::TofChromatogramsMethod >
    tdcdoc_< ap240::waveform >::tofChromatogramsMethod() const
    {
        return ap240::tdcdoc::tofChromatogramsMethod();
    }
    
    template<> void
    tdcdoc_< ap240::waveform >::setCountingMethod( std::shared_ptr< const adcontrols::CountingMethod > a )
    {
        // ap240::tdcdoc::setCountingMethod( a );
    }
    
    template<> std::shared_ptr< const adcontrols::CountingMethod >
    tdcdoc_< ap240::waveform >::countingMethod() const
    {
        return std::make_shared< adcontrols::CountingMethod >();
        // return ap240::tdcdoc::countingMethod();
    }
    
    template<> void
    tdcdoc_< ap240::waveform >::eraseTofChromatogramsMethod()
    {
        // ap240::tdcdoc::eraseTofChromatogramsMethod();
    }

    template<> std::array< std::shared_ptr< threshold_result_< ap240::waveform> >, 2 >
    tdcdoc_< ap240::waveform >::processThreshold( std::array< std::shared_ptr< const waveform_type >, 2> )
    {
        return std::array< std::shared_ptr< threshold_result_<waveform_type> >, 2 >{0,0};
    }

    template<> std::array< std::shared_ptr< threshold_result_< ap240::waveform > >, 2 >
    tdcdoc_< ap240::waveform >::processThreshold2( std::array< std::shared_ptr< const waveform_type >, 2 > )
    {
        return std::array< std::shared_ptr< threshold_result_<waveform_type> >, 2 >{0,0};        
    }
    
    // peak detection (on trial)
    template<> std::array< std::shared_ptr< threshold_result_< ap240::waveform > >, 2 >
    tdcdoc_< ap240::waveform >::processThreshold3( std::array< std::shared_ptr< const waveform_type >, 2> )
    {
        return std::array< std::shared_ptr< threshold_result_<waveform_type> >, 2 >{0,0};
    }

    template<> bool
    tdcdoc_< ap240::waveform >::accumulate_waveform( std::shared_ptr< const waveform_type > a )
    {
        return ap240::tdcdoc::accumulate_waveform( a );
    }
    
    template<> bool
    tdcdoc_< ap240::waveform >::accumulate_histogram( tdcdoc_::const_threshold_result_ptr a )
    {
        ap240::const_threshold_result_ptr x;
        ap240::tdcdoc::accumulate_histogram( x );
        return false; // return ap240::tdcdoc::accumulate_histogram( a );
    }
    
    template<> size_t
    tdcdoc_< ap240::waveform >::readAveragedWaveforms( std::vector< std::shared_ptr< const waveform_type > >& a )
    {
        return 0;
    }

    template<> size_t
    tdcdoc_< ap240::waveform >::readTimeDigitalHistograms(
        std::vector< std::shared_ptr< const adcontrols::TimeDigitalHistogram > >& a )
    {
        return 0;
    }
    
    template<> std::shared_ptr< const ap240::waveform >
    tdcdoc_< ap240::waveform >::averagedWaveform( uint64_t trigNumber )
    {
        return ap240::tdcdoc::averagedWaveform( trigNumber );
    }

    template<> std::shared_ptr< adcontrols::TimeDigitalHistogram >
    tdcdoc_< ap240::waveform >::longTermHistogram( int protocolIndex ) const
    {
        return ap240::tdcdoc::longTermHistogram( protocolIndex );
    }

    template<> std::shared_ptr< adcontrols::TimeDigitalHistogram >
    tdcdoc_< ap240::waveform >::recentHistogram( int protocolIndex ) const
    {
        return 0;
        // return ap240::tdcdoc::recentHistogram( protocolIndex );
    }

    template<> double
    tdcdoc_< ap240::waveform >::triggers_per_second() const
    {
        return ap240::tdcdoc::triggers_per_second();
    }

    template<> std::vector< std::shared_ptr< adcontrols::TimeDigitalHistogram > >
    tdcdoc_< ap240::waveform >::longTermHistograms() const
    {
        return ap240::tdcdoc::longTermHistograms();
    }
    
    // protocol sequence but no order garanteed
    template<> std::vector< std::shared_ptr< adcontrols::TimeDigitalHistogram > >
    tdcdoc_< ap240::waveform >::recentHistograms() const
    {
        return ap240::tdcdoc::recentHistograms();
    }

    template<> std::shared_ptr< adcontrols::MassSpectrum >
    tdcdoc_< ap240::waveform >::recentSpectrum( SpectrumType a
                                                , mass_assignee_t b
                                                , int protocolIndex ) const
    {
        return 0;
        // return ap240::tdcdoc::recentSpectrum( a, b, protocolIndex );
    }

    template<> bool
    tdcdoc_< ap240::waveform >::makeChromatogramPoints( std::shared_ptr< const waveform_type > waveform
                                                        , const adcontrols::TofChromatogramsMethod& method
                                                        , std::vector< std::pair< uint32_t, double > >& values )
    {
        return false; //ap240::tdcdoc::makeChromatogramPoints( waveform, method, values );
    }

    template<> bool
    tdcdoc_< ap240::waveform >::makeCountingChromatogramPoints( const adcontrols::TimeDigitalHistogram& a
                                                                , std::vector< uint32_t >& results )
    {
        return ap240::tdcdoc::makeCountingChromatogramPoints( a, results );
    }

    template<> bool
    tdcdoc_< ap240::waveform >::computeCountRate( const adcontrols::TimeDigitalHistogram& histogram
                                                  , const adcontrols::CountingMethod& m
                                                  , std::vector< std::pair< size_t, size_t > >& v )
    {
        return false;
        // return ap240::tdcdoc::computeCountRate( histogram, m, v );
    }
    
    template<> void
    tdcdoc_< ap240::waveform >::clear_histogram()
    {
        return ap240::tdcdoc::clear_histogram();
    }
    
    template<> std::pair< uint32_t, uint32_t >
    tdcdoc_< ap240::waveform >::threshold_action_counts( int channel ) const
    {
        return ap240::tdcdoc::threshold_action_counts( channel );
    }
}
