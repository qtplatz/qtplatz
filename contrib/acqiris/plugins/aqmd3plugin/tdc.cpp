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

#include "tdc.hpp"
#include <aqmd3controls/waveform.hpp>
#include <aqmd3controls/waveform_adder.hpp>
#include <aqmd3controls/histogram_adder.hpp>
#include <adcontrols/controlmethod.hpp>

#include <algorithm>
#include <deque>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace aqmd3 {
    std::mutex tdc::mutex_;
}

using namespace aqmd3;

tdc::tdc() : protocolCount_( 1 )
{
}

std::shared_ptr< adcontrols::TimeDigitalHistogram >
tdc::longTermHistogram( int protocolIndex ) const
{
    std::lock_guard< std::mutex > lock( mutex_ );
    if ( recent_longterm_histograms_.empty() )
        return nullptr;
    // deep copy
    auto hgrm = std::make_shared< adcontrols::TimeDigitalHistogram >( *recent_longterm_histograms_[ protocolIndex ] );
    return hgrm;
}

std::vector< std::shared_ptr< adcontrols::TimeDigitalHistogram > >
tdc::longTermHistograms() const
{
    std::lock_guard< std::mutex > lock( mutex_ );

    std::vector< std::shared_ptr< adcontrols::TimeDigitalHistogram > > d( recent_longterm_histograms_.size() );
    std::transform( recent_longterm_histograms_.begin()
                    , recent_longterm_histograms_.begin() + protocolCount_, d.begin()
                    , []( const std::shared_ptr< const adcontrols::TimeDigitalHistogram >& h ){
                        return std::make_shared< adcontrols::TimeDigitalHistogram >( *h ); // deep copy
                    });
    return d;
}

void
tdc::set_periodic_avgd_waveforms( uint32_t protocol, std::shared_ptr< const waveform_t > w )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    recent_periodic_avgd_waveforms_[ protocol ] = w;
}

void
tdc::add_longterm_avgd_waveforms( uint32_t protocol, std::shared_ptr< const waveform_t > w )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    if ( recent_longterm_avgd_waveforms_[ protocol ] == nullptr )
        recent_longterm_avgd_waveforms_[ protocol ] = std::make_shared< waveform_t >( *w );
    else
        (*recent_longterm_avgd_waveforms_[ protocol ]) += *w;
}

void
tdc::add_pkd_waveforms( uint32_t protocol, std::shared_ptr< waveform_t > w )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    if ( pkd_waveform_adder_[ protocol ] == nullptr )
        pkd_waveform_adder_[ protocol ] = std::make_shared< waveform_adder_t >();
    pkd_waveform_adder_[ protocol ]->add( *w );
}

void
tdc::set_recent_periodic_histograms( uint32_t protocol, std::shared_ptr< adcontrols::TimeDigitalHistogram > hgrm )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    recent_periodic_histograms_[ protocol ] = hgrm;
}

void
tdc::add_recent_longterm_histograms( uint32_t protocol, std::shared_ptr< adcontrols::TimeDigitalHistogram > hgrm )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    if ( recent_longterm_histograms_[ protocol ] == nullptr )
        recent_longterm_histograms_[ protocol ] = std::make_shared< adcontrols::TimeDigitalHistogram >( *hgrm );
    else
        (*recent_longterm_histograms_[ protocol ]) += *hgrm;
}


std::vector< std::shared_ptr< adcontrols::TimeDigitalHistogram > >
tdc::recentHistograms() const
{
    std::lock_guard< std::mutex > lock( mutex_ );
    std::vector< std::shared_ptr< adcontrols::TimeDigitalHistogram > > d( protocolCount_ );
    std::transform( recent_periodic_histograms_.begin()
                    , recent_periodic_histograms_.begin() + protocolCount_
                    , d.begin()
                    , []( const std::shared_ptr< const adcontrols::TimeDigitalHistogram >& h ){
                        return std::make_shared< adcontrols::TimeDigitalHistogram >( *h );
                    });
    return d;
}

// queue for data writter
void
tdc::enqueue( std::shared_ptr< const adcontrols::TimeDigitalHistogram > hgrm )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    periodic_histogram_que_.emplace_back( hgrm );
}

// queue for data writter
void
tdc::enqueue( std::shared_ptr< const waveform_t > avgd )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    periodic_waveform_que_.emplace_back( avgd );
}

size_t
tdc::readTimeDigitalHistograms( std::vector< std::shared_ptr< const adcontrols::TimeDigitalHistogram > >& a )
{
    if ( ! periodic_histogram_que_.empty() ) {
        std::lock_guard< std::mutex > lock( mutex_ );
        a.reserve( a.size() + periodic_histogram_que_.size() );
        std::move( periodic_histogram_que_.begin(), periodic_histogram_que_.end(), std::back_inserter( a ) );
        periodic_histogram_que_.clear();
        return a.size();
    }
    return 0;
}

size_t
tdc::read_avgd_waveforms( std::vector< std::shared_ptr< const waveform_t > >& a )
{
    if ( ! periodic_waveform_que_.empty() ) {
        std::lock_guard< std::mutex > lock( mutex_ );
        a.reserve( a.size() + periodic_waveform_que_.size() );
        std::move( periodic_waveform_que_.begin(), periodic_waveform_que_.end(), std::back_inserter( a ) );
        periodic_waveform_que_.clear();
        return a.size();
    }
    return 0;
}

//std::array< AverageData,   max_protocol >& accumulator() { return accumulator_; }

void
tdc::clear_histogram()
{
    std::lock_guard< std::mutex > lock( mutex_ );
    for ( auto& p: recent_longterm_histograms_ )
        p = std::make_shared< adcontrols::TimeDigitalHistogram >();
    for ( auto& p: recent_longterm_avgd_waveforms_ )
        p.reset();
}

size_t
tdc::histogram_size() const
{
    return periodic_histogram_que_.size();
}

size_t
tdc::histogram_empty() const
{
    return periodic_histogram_que_.empty();
}

size_t
tdc::waveform_size() const
{
    return periodic_waveform_que_.size();
}

size_t
tdc::waveform_empty() const
{
    return periodic_waveform_que_.empty();
}

std::shared_ptr< adcontrols::MassSpectrum >
tdc::recentSpectrum( SpectrumType choice, adportable::mass_assign_t assigner ) const
{
    auto ms = std::make_shared< adcontrols::MassSpectrum >();
    std::lock_guard< std::mutex > lock( mutex_ );
    switch ( choice ) {
    case Profile:
        return recentSpectrum_( recent_waveforms_, assigner );
    case ProfileAvgd:
        return recentSpectrum_( recent_periodic_avgd_waveforms_, assigner );
    case ProfileLongTerm:
        return recentSpectrum_( recent_longterm_avgd_waveforms_, assigner );
    case Histogram:
        return recentSpectrum_( recent_periodic_histograms_, assigner );
    case HistogramLongTerm:
        return recentSpectrum_( recent_longterm_histograms_, assigner );
    default:
        break;
    }
    return nullptr;
}

std::shared_ptr< histogram_adder_t >
tdc::histogram_adder( int protocol )
{
    if ( histogram_adder_[ protocol ] == nullptr )
        histogram_adder_[ protocol ] = std::make_shared< histogram_adder_t >();
    return histogram_adder_[ protocol ];
}

std::shared_ptr< waveform_adder_t >
tdc::waveform_adder( int protocol )
{
    if ( waveform_adder_[ protocol ] == nullptr )
        waveform_adder_[ protocol ] = std::make_shared< waveform_adder_t >();
    return waveform_adder_[ protocol ];
}
