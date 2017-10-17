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

#include <acqrscontrols/acqrscontrols_global.hpp>
#include <acqrscontrols/constants.hpp>
#include <acqrscontrols/tdcbase.hpp>
#include <boost/uuid/uuid.hpp>
#include <atomic>
#include <array>
#include <mutex>
#include <memory>
#include <vector>
#include <functional>

namespace adportable { struct threshold_index; }

namespace acqrscontrols { namespace u5303a { class waveform; class threshold_result; class histogram; } }

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

        class ACQRSCONTROLSSHARED_EXPORT tdcdoc : public acqrscontrols::tdcbase { 
        public:
            ~tdcdoc();
            tdcdoc();

            static constexpr size_t max_protocol = 4;
            // static constexpr boost::uuids::uuid timecount_observer = { 0x57, 0xa9, 0xeb, 0x79, 0xa6, 0xa7, 0x5e, 0x83, 0xa7, 0xe9, 0x28, 0x55, 0xf8, 0xcf, 0xb7, 0xe6 };
            // static constexpr boost::uuids::uuid softavgr_observer  = { 0xf2, 0x6e, 0xd6, 0x45, 0x7e, 0x3f, 0x50, 0x8e, 0xb8, 0x80, 0x83, 0x81, 0x2d, 0xc7, 0x59, 0x4c };
            // static constexpr boost::uuids::uuid histogram_observer = { 0xdb, 0x7c, 0xbb, 0x6b, 0xcf, 0x0b, 0x58, 0x2e, 0x9f, 0x8f, 0x89, 0x01, 0x92, 0x65, 0x27, 0x1b };
            
            typedef acqrscontrols::u5303a::waveform waveform_type;
            typedef const acqrscontrols::u5303a::waveform const_waveform_type;

            bool set_threshold_action( const adcontrols::threshold_action& ) override;
            std::shared_ptr< const adcontrols::threshold_action > threshold_action() const override;

            bool set_threshold_method( int channel, const adcontrols::threshold_method& ) override;
            std::shared_ptr< const adcontrols::threshold_method > threshold_method( int channel ) const override;

            bool setTofChromatogramsMethod( const adcontrols::TofChromatogramsMethod& ) override;
            std::shared_ptr< const adcontrols::TofChromatogramsMethod > tofChromatogramsMethod() const override;

            void setCountingMethod( std::shared_ptr< const adcontrols::CountingMethod > ) override;
            std::shared_ptr< const adcontrols::CountingMethod > countingMethod() const override;
            
            void eraseTofChromatogramsMethod() override;

            std::array< threshold_result_ptr, acqrscontrols::u5303a::nchannels >
                processThreshold( std::array< std::shared_ptr< const waveform_type >, acqrscontrols::u5303a::nchannels > );

            // find pair of raising,falling
            std::array< threshold_result_ptr, acqrscontrols::u5303a::nchannels >
                processThreshold2( std::array< std::shared_ptr< const waveform_type >, acqrscontrols::u5303a::nchannels > );

            // peak detection (on trial)
            std::array< threshold_result_ptr, acqrscontrols::u5303a::nchannels >
                processThreshold3( std::array< std::shared_ptr< const waveform_type >, acqrscontrols::u5303a::nchannels > );
            
            bool accumulate_waveform( std::shared_ptr< const waveform_type > );

            bool accumulate_histogram( const_threshold_result_ptr );

            size_t readAveragedWaveforms( std::vector< std::shared_ptr< const waveform_type > >& );
            
            size_t readTimeDigitalHistograms( std::vector< std::shared_ptr< const adcontrols::TimeDigitalHistogram > >& );
            
            std::shared_ptr< const waveform_type > averagedWaveform( uint64_t trigNumber );

            std::shared_ptr< adcontrols::TimeDigitalHistogram > longTermHistogram( int protocolIndex = 0 ) const; 
            // std::shared_ptr< adcontrols::TimeDigitalHistogram > recentHistogram( int protocolIndex = 0 ) const;
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
            
            bool makeCountingChromatogramPoints( const adcontrols::TimeDigitalHistogram&
                                                 , std::vector< uint32_t >& results );

            static bool computeCountRate( const adcontrols::TimeDigitalHistogram& histogram
                                          , const adcontrols::CountingMethod&
                                          , std::vector< std::pair< size_t, size_t > >& );

            void clear_histogram() override;
            
            std::pair< uint32_t, uint32_t > threshold_action_counts( int channel ) const override;
            void set_threshold_action_counts( int channel, const std::pair< uint32_t, uint32_t >& ) const override;
            
        private:
            class impl;
            impl * impl_;
        };

    }

}
