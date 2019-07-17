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

#include <acqrscontrols/acqrscontrols_global.hpp>
#include <acqrscontrols/constants.hpp>
#include <acqrscontrols/tdcbase.hpp>
#include <compiler/boost/workaround.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <atomic>
#include <array>
#include <mutex>
#include <memory>
#include <vector>
#include <functional>

namespace acqrscontrols {
    namespace ap240 { class waveform; class histogram; }
    template< typename T > class threshold_result_;
}

namespace adcontrols { class threshold_action; class threshold_method;
    class MassSpectrum; class TofChromatogramsMethod; class TimeDigitalHistogram; }

namespace acqrscontrols {

    namespace ap240 {

        // typedef acqrscontrols::ap240::threshold_result deprecated_threshold_result_type;
        // typedef std::shared_ptr< const deprecated_threshold_result_type > const_deprecated_threshold_result_ptr;
        // typedef std::shared_ptr< deprecated_threshold_result_type > deprecated_threshold_result_ptr;
        
        typedef acqrscontrols::threshold_result_< ap240::waveform > threshold_result_type;
        typedef std::shared_ptr< threshold_result_type > threshold_result_ptr;
        typedef std::shared_ptr< const threshold_result_type > const_threshold_result_ptr;
        typedef acqrscontrols::ap240::waveform waveform_type;
        typedef acqrscontrols::ap240::histogram histogram_type;

#if defined _MSC_VER
        ACQRSCONTROLSSHARED_TEMPLATE_EXPORT template class ACQRSCONTROLSSHARED_EXPORT std::weak_ptr < threshold_result_type > ;
        ACQRSCONTROLSSHARED_TEMPLATE_EXPORT template class ACQRSCONTROLSSHARED_EXPORT std::weak_ptr < histogram_type > ;
#endif

        class ACQRSCONTROLSSHARED_EXPORT tdcdoc : public acqrscontrols::tdcbase { 
        public:
            ~tdcdoc();
            tdcdoc();

            typedef acqrscontrols::ap240::waveform waveform_type;
            typedef const acqrscontrols::ap240::waveform const_waveform_type;

            static constexpr size_t max_protocol = 4;
            // static constexpr boost::uuids::uuid timecount_observer = { 0x4f, 0x43, 0x1f, 0x91, 0xb0, 0x8c, 0x54, 0xba, 0x94, 0xf0, 0xe1, 0xd1, 0x3e, 0xba, 0x29, 0xd7 };
            // static constexpr boost::uuids::uuid softavgr_observer  = { 0x89, 0xa3, 0x96, 0xe5, 0x2f, 0x58, 0x57, 0x1a, 0x8f, 0x0c, 0x9d, 0xa6, 0x8d, 0xd3, 0x1a, 0xe4 };
            // static constexpr boost::uuids::uuid histogram_observer = { 0xeb, 0x9d, 0x55, 0x89, 0xa3, 0xa4, 0x58, 0x2c, 0x94, 0xc6, 0xf7, 0xaf, 0xfb, 0xe8, 0x34, 0x8a };

            bool set_threshold_action( const adcontrols::threshold_action& ) override;
            std::shared_ptr< const adcontrols::threshold_action > threshold_action() const override;

            bool set_threshold_method( int channel, const adcontrols::threshold_method& ) override;
            std::shared_ptr< const adcontrols::threshold_method > threshold_method( int channel ) const override;

            std::array< threshold_result_ptr, acqrscontrols::ap240::nchannels >
                processThreshold( std::array< std::shared_ptr< const waveform_type >, acqrscontrols::ap240::nchannels > );

            // find pair of raising,falling
            std::array< threshold_result_ptr, acqrscontrols::ap240::nchannels >
                processThreshold2( std::array< std::shared_ptr< const waveform_type >, acqrscontrols::ap240::nchannels > );
            
            std::array< threshold_result_ptr, acqrscontrols::ap240::nchannels >
                processThreshold3( std::array< std::shared_ptr< const waveform_type >, acqrscontrols::ap240::nchannels > );

            // peak detection (on trial)
            // std::array< threshold_result_ptr, acqrscontrols::ap240::nchannels >
            //     processThreshold3( std::array< std::shared_ptr< const waveform_type >, acqrscontrols::ap240::nchannels > );

            bool accumulate_waveform( std::shared_ptr< const waveform_type > );

            // bool accumulate_histogram( const_deprecated_threshold_result_ptr );

            bool accumulate_histogram( const_threshold_result_ptr ); // std::shared_ptr< const acqrscontrols::threshold_result_< waveform > > p );
            
            size_t readAveragedWaveforms( std::vector< std::shared_ptr< const waveform_type > >& );

            size_t readTimeDigitalHistograms( std::vector< std::shared_ptr< const adcontrols::TimeDigitalHistogram > >& );
            
            std::shared_ptr< const waveform_type > averagedWaveform( uint64_t trigNumber );

            std::shared_ptr< adcontrols::TimeDigitalHistogram > longTermHistogram( int protocolIndex = 0 ) const; 
            // std::shared_ptr< adcontrols::TimeDigitalHistogram > recentHistogram( int protocolIndex ) const;
            double triggers_per_second() const;

            // return as protocol sequence
            std::vector< std::shared_ptr< adcontrols::TimeDigitalHistogram > > longTermHistograms() const;

            // protocol sequence but no order garanteed
            std::vector< std::shared_ptr< adcontrols::TimeDigitalHistogram > > recentHistograms() const;

            typedef std::function< double( double, int ) > mass_assignee_t;

            std::shared_ptr< adcontrols::MassSpectrum >
                recentSpectrum( SpectrumType, mass_assignee_t = mass_assignee_t(), int protocolIndex = (-1) ) const;
            
            bool makeChromatogramPoints( std::shared_ptr< const waveform_type > waveform
                                         , const adcontrols::TofChromatogramsMethod& method
                                         , std::vector< std::pair< uint32_t, double > >& values );
            
            // bool makeChromatogramPoints( const std::shared_ptr< const waveform_type >&, std::vector< std::pair<double, double> >& results );

            bool makeCountingChromatogramPoints( const adcontrols::TimeDigitalHistogram&, std::vector< uint32_t >& results );

            void clear_histogram() override;

            std::pair< uint32_t, uint32_t > threshold_action_counts( int channel ) const override;
            void set_threshold_action_counts( int channel, const std::pair< uint32_t, uint32_t >& ) const override;

            static void find_threshold_timepoints( const acqrscontrols::ap240::waveform& data
                                                   , const adcontrols::threshold_method& method
                                                   , std::vector< uint32_t >& elements
                                                   , std::vector<double>& processed );

        private:
            class impl;
            impl * impl_;
        };

    }

}
