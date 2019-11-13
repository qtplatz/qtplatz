/**************************************************************************
** Copyright (C) 2010-2020 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2020 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include <adcontrols/timedigitalhistogram.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adportable/mass_assign_t.hpp>
#include <deque>
#include <mutex>
#include <memory>
#include <vector>

namespace adcontrols {
    namespace ControlMethod { class Method; }
    class TimeDigitalHistogram;
    class MassSpectrum;
}

namespace aqmd3controls {
    class waveform;
    class waveform_adder;
    class histogram_adder;
}

namespace aqmd3 {

    typedef aqmd3controls::waveform waveform_t;
    typedef aqmd3controls::waveform_adder waveform_adder_t;
    typedef aqmd3controls::histogram_adder histogram_adder_t;

    class tdc {
        tdc( tdc& t ) = delete;

    public:
        static constexpr size_t max_protocol = 4;
        enum SpectrumType { Profile, ProfileAvgd, ProfileLongTerm, Histogram, HistogramLongTerm, PkdWaveformLongTerm };

        static tdc * instance() {
            static tdc __instance;
            return &__instance;
        }

        tdc();

        std::shared_ptr< adcontrols::TimeDigitalHistogram > longTermHistogram( int protocolIndex ) const;
        std::vector< std::shared_ptr< adcontrols::TimeDigitalHistogram > > longTermHistograms() const;

        std::shared_ptr< histogram_adder_t > histogram_adder( int protocol );
        std::shared_ptr< waveform_adder_t > waveform_adder( int protocol );

        void set_periodic_avgd_waveforms( uint32_t protocol, std::shared_ptr< const waveform_t > w );
        void add_longterm_avgd_waveforms( uint32_t protocol, std::shared_ptr< const waveform_t > w );

        void add_pkd_waveforms( uint32_t protocol, std::shared_ptr< waveform_t > w );
        void set_recent_periodic_histograms( uint32_t protocol, std::shared_ptr< adcontrols::TimeDigitalHistogram > hgrm );
        void add_recent_longterm_histograms( uint32_t protocol, std::shared_ptr< adcontrols::TimeDigitalHistogram > hgrm );

        std::vector< std::shared_ptr< adcontrols::TimeDigitalHistogram > > recentHistograms() const;

        // queue for data writter
        void enqueue( std::shared_ptr< const adcontrols::TimeDigitalHistogram > hgrm );

        // queue for data writter
        void enqueue( std::shared_ptr< const waveform_t > avgd );

        size_t readTimeDigitalHistograms( std::vector< std::shared_ptr< const adcontrols::TimeDigitalHistogram > >& a );

        size_t read_avgd_waveforms( std::vector< std::shared_ptr< const waveform_t > >& a );

        void clear_histogram();

        size_t histogram_size() const;
        size_t histogram_empty() const;

        size_t waveform_size() const;
        size_t waveform_empty() const;

        std::shared_ptr< adcontrols::MassSpectrum > recentSpectrum( SpectrumType choice, adportable::mass_assign_t assigner ) const;

    private:
        // pkd
        std::shared_ptr< waveform_t > accumulated_pkd_waveform_;
        std::vector< std::shared_ptr< const adcontrols::TimeDigitalHistogram > > periodic_histogram_que_;

        // periodic waveforms (primary que) [time array] := (proto 0, proto 1, ...), (proto 0, proto 1, ...),...
        std::vector< std::shared_ptr< const waveform_t > > periodic_waveform_que_;

        std::array< std::shared_ptr< const adcontrols::TimeDigitalHistogram >, max_protocol > recent_periodic_histograms_;
        std::array< std::shared_ptr< adcontrols::TimeDigitalHistogram >, max_protocol > recent_longterm_histograms_;

        // recent protocol sequence waveforms (single trigger)
        std::array< std::shared_ptr< const waveform_t >, max_protocol > recent_waveforms_;

        // recent protocol sequence waveforms (software averaged)
        std::array< std::shared_ptr< const waveform_t >, max_protocol > recent_periodic_avgd_waveforms_;
        std::array< std::shared_ptr< waveform_t >, max_protocol > recent_longterm_avgd_waveforms_;

        //std::array< AverageData, max_protocol > accumulator_;
        std::array< std::shared_ptr< waveform_adder_t >, max_protocol > pkd_waveform_adder_;
        std::array< std::shared_ptr< waveform_adder_t >, max_protocol > waveform_adder_;
        std::array< std::shared_ptr< histogram_adder_t >, max_protocol > histogram_adder_;

        // meta data for initial waveform in the current accumulation range
        uint32_t protocolCount_;
        static std::mutex mutex_;

    private:
        template< typename T >
        std::shared_ptr< adcontrols::MassSpectrum >
        recentSpectrum_( const std::array< std::shared_ptr< T >, max_protocol >& v
                         , adportable::mass_assign_t assigner ) const {

            if ( v.at(0) == nullptr )
                return nullptr;

            auto ms = std::make_shared< adcontrols::MassSpectrum >();
            int protocol(0);
            for ( const auto& w: v ) {
                if ( w ) {
                    auto sp = ( protocol == 0 ) ? ms : std::make_shared< adcontrols::MassSpectrum >();
                    T::translate( *sp, *w, assigner );
                    sp->setProtocol( protocol, protocolCount_ );
                    if ( sp != ms )
                        (*ms) << std::move( sp );
                }
                ++protocol;
            }
            return ms;
        }
    };

}
