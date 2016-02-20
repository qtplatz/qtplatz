/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include <acqrscontrols/acqrscontrols_global.hpp>
#include <acqrscontrols/constants.hpp>
#include <atomic>
#include <array>
#include <mutex>
#include <memory>
#include <vector>
#include <functional>

namespace acqrscontrols { namespace u5303a { class waveform; class threshold_result; class histogram; } }

namespace adcontrols { class threshold_action; class threshold_method;
    class MassSpectrum; class TofChromatogramsMethod; class TimeDigitalHistogram; }

namespace acqrscontrols {

    namespace u5303a {

        typedef acqrscontrols::u5303a::threshold_result threshold_result_type;
        typedef std::shared_ptr< threshold_result_type > threshold_result_ptr;
        typedef std::shared_ptr< const threshold_result_type > const_threshold_result_ptr;
        typedef acqrscontrols::u5303a::waveform waveform_type;
        typedef acqrscontrols::u5303a::histogram histogram_type;

#if defined _MSC_VER
        ACQRSCONTROLSSHARED_TEMPLATE_EXPORT template class ACQRSCONTROLSSHARED_EXPORT std::weak_ptr < threshold_result_type > ;
        ACQRSCONTROLSSHARED_TEMPLATE_EXPORT template class ACQRSCONTROLSSHARED_EXPORT std::weak_ptr < histogram_type > ;
#endif

        class ACQRSCONTROLSSHARED_EXPORT tdcdoc { 
        public:
            ~tdcdoc();
            tdcdoc();

            typedef acqrscontrols::u5303a::waveform waveform_type;
            typedef const acqrscontrols::u5303a::waveform const_waveform_type;

            bool set_threshold_action( const adcontrols::threshold_action& );
            std::shared_ptr< const adcontrols::threshold_action > threshold_action() const;

            bool set_threshold_method( int channel, const adcontrols::threshold_method& );
            std::shared_ptr< const adcontrols::threshold_method > threshold_method( int channel ) const;

            bool setTofChromatogramsMethod( const adcontrols::TofChromatogramsMethod& );
            std::shared_ptr< const adcontrols::TofChromatogramsMethod > tofChromatogramsMethod() const;

            void eraseTofChromatogramsMethod();
            
            std::array< threshold_result_ptr, acqrscontrols::u5303a::nchannels >
                processThreshold( std::array< std::shared_ptr< const waveform_type >, acqrscontrols::u5303a::nchannels > );

            bool accumulate_waveform( std::shared_ptr< const waveform_type > );

            bool accumulate_histogram( const_threshold_result_ptr );

            size_t readAveragedWaveforms( std::vector< std::shared_ptr< const waveform_type > >& );

            size_t readTimeDigitalHistograms( std::vector< std::shared_ptr< const adcontrols::TimeDigitalHistogram > >& );
            
            std::shared_ptr< const waveform_type > averagedWaveform( uint64_t trigNumber );

            std::shared_ptr< const adcontrols::TimeDigitalHistogram > longTermHistogram() const; 

            bool makeChromatogramPoints( const std::shared_ptr< const waveform_type >&, std::vector< std::pair<double, double> >& results );

            bool makeCountingChromatogramPoints( const adcontrols::TimeDigitalHistogram&, std::vector< uint32_t >& results );

            // strand required
            void appendHistogram( std::array< threshold_result_ptr, acqrscontrols::u5303a::nchannels > results );
            
            [[deprecated]] std::shared_ptr< adcontrols::MassSpectrum >
            getHistogram( double resolution, int channel, size_t& trigCount, std::pair<uint64_t, uint64_t>& timeSinceEpoch ) const;

            void update_rate( size_t, const std::pair<uint64_t, uint64_t>& timeSinceEpoch );

            void clear_histogram();

            double trig_per_seconds() const;

            std::pair< uint32_t, uint32_t > threshold_action_counts( int channel ) const;

            static void find_threshold_timepoints( const acqrscontrols::u5303a::waveform& data
                                                   , const adcontrols::threshold_method& method
                                                   , std::vector< uint32_t >& elements
                                                   , std::vector<double>& processed );

        private:
            class impl;
            impl * impl_;
        };

    }

}
