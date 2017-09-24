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

    template< typename waveform_type >
    class ACQRSCONTROLSSHARED_EXPORT tdcdoc_ : public tdcdoc_type< waveform_type >::_type { 
    public:
        ~tdcdoc_();
        tdcdoc_();

        // typedef std::shared_ptr< threshold_result_type< waveform_type > > threshold_result_ptr;
        // typedef std::shared_ptr< const threshold_result_type< waveform_type > > const_threshold_result_ptr;
        typedef std::shared_ptr< threshold_result_< waveform_type > > threshold_result_ptr;
        typedef std::shared_ptr< const threshold_result_< waveform_type > > const_threshold_result_ptr;
        
        static constexpr size_t max_protocol = 4;

        bool set_threshold_action( const adcontrols::threshold_action& );
        std::shared_ptr< const adcontrols::threshold_action > threshold_action() const;

        bool set_threshold_method( int channel, const adcontrols::threshold_method& );
        std::shared_ptr< const adcontrols::threshold_method > threshold_method( int channel ) const;

        bool setTofChromatogramsMethod( const adcontrols::TofChromatogramsMethod& );
        std::shared_ptr< const adcontrols::TofChromatogramsMethod > tofChromatogramsMethod() const;

        void setCountingMethod( std::shared_ptr< const adcontrols::CountingMethod > );
        std::shared_ptr< const adcontrols::CountingMethod > countingMethod() const;
            
        void eraseTofChromatogramsMethod();

#if 0
        std::array< std::shared_ptr< threshold_result_<waveform_type> >, 2 >
            processThreshold( std::array< std::shared_ptr< const waveform_type >, 2> ) {

            return std::array< std::shared_ptr< threshold_result_<waveform_type> >, 2 >{0,0};
        }

        // find pair of raising,falling
        std::array< threshold_result_ptr, 2 >
            processThreshold2( std::array< std::shared_ptr< const waveform_type >, 2> );

        // peak detection (on trial)
        std::array< threshold_result_ptr, 2 >
            processThreshold3( std::array< std::shared_ptr< const waveform_type >, 2> );
#endif
            
        bool accumulate_waveform( std::shared_ptr< const waveform_type > );

        // bool accumulate_histogram( const_threshold_result_ptr );
        bool accumulate_histogram( std::shared_ptr< const ap240::threshold_result > ptr );

        size_t readAveragedWaveforms( std::vector< std::shared_ptr< const waveform_type > >& );

        size_t readTimeDigitalHistograms( std::vector< std::shared_ptr< const adcontrols::TimeDigitalHistogram > >& );
            
        std::shared_ptr< const waveform_type > averagedWaveform( uint64_t trigNumber );

        std::shared_ptr< adcontrols::TimeDigitalHistogram > longTermHistogram( int protocolIndex = 0 ) const; 
        std::shared_ptr< adcontrols::TimeDigitalHistogram > recentHistogram( int protocolIndex ) const;
        double triggers_per_second() const;

        // return as protocol sequence
        std::vector< std::shared_ptr< adcontrols::TimeDigitalHistogram > > longTermHistograms() const;

        // protocol sequence but no order garanteed
        std::vector< std::shared_ptr< adcontrols::TimeDigitalHistogram > > recentHistograms() const;

        typedef std::function< double( double, int ) > mass_assignee_t;

        enum SpectrumType { Raw, Profile, PeriodicHistogram, LongTermHistogram };

        std::shared_ptr< adcontrols::MassSpectrum >
            recentSpectrum( SpectrumType, mass_assignee_t = mass_assignee_t(), int protocolIndex = (-1) ) const;
            
        // bool makeChromatogramPoints( const std::shared_ptr< const waveform_type >&
        //                              , std::vector< std::pair<double, double> >& results );
        bool makeChromatogramPoints( std::shared_ptr< const waveform_type > waveform
                                     , const adcontrols::TofChromatogramsMethod& method
                                     , std::vector< std::pair< uint32_t, double > >& values );
            
        bool makeCountingChromatogramPoints( const adcontrols::TimeDigitalHistogram&
                                             , std::vector< uint32_t >& results );

        static bool computeCountRate( const adcontrols::TimeDigitalHistogram& histogram
                                      , const adcontrols::CountingMethod&
                                      , std::vector< std::pair< size_t, size_t > >& );

        void clear_histogram();
        std::pair< uint32_t, uint32_t > threshold_action_counts( int channel ) const;

    };

}
