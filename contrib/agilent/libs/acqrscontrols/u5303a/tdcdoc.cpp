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

#include "tdcdoc.hpp"
#include "averagedata.hpp"
#include <acqrscontrols/u5303a/histogram.hpp>
#include <acqrscontrols/u5303a/threshold_result.hpp>
#include <acqrscontrols/threshold_action_finder.hpp>
#include <adcontrols/controlmethod/tofchromatogramsmethod.hpp>
#include <adcontrols/controlmethod/tofchromatogrammethod.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/samplinginfo.hpp>
#include <adcontrols/timedigitalhistogram.hpp>
#include <adcontrols/waveform_filter.hpp>
#include <adportable/binary_serializer.hpp>
#include <adportable/debug.hpp>
#include <adportable/float.hpp>
#include <adportable/spectrum_processor.hpp>
#include <adportable/threshold_finder.hpp>
#include <adportable/waveform_averager.hpp>
#include <adportable/waveform_peakfinder.hpp>
#include <adportable/waveform_processor.hpp>
#include <adportable/waveform_wrapper.hpp>
#include <deque>

namespace acqrscontrols {
    namespace u5303a {

        typedef adportable::waveform_averager< int32_t > averager_type;
        typedef waveform waveform_type;

        class tdcdoc::impl {
        public:
            ~impl() {}
            
            impl() : threshold_action_( std::make_shared< adcontrols::threshold_action >() )
                   , tofChromatogramsMethod_( std::make_shared< adcontrols::TofChromatogramsMethod >() )
                   , protocolCount_( 1 )
                   , trig_per_seconds_( 0 ) {

                for ( auto& p: recent_longterm_histograms_ )
                    p = std::make_shared< adcontrols::TimeDigitalHistogram >();                
            }

            bool recentProfile( adcontrols::MassSpectrum& ms
                                , const std::array< std::shared_ptr< const acqrscontrols::u5303a::waveform >, max_protocol >& v
                                , tdcdoc::mass_assignee_t assignee ) const {

                if ( !v.empty() && v[ 0 ] ) {

                    waveform::translate( ms, v[ 0 ], assignee );
                    
                    for ( uint32_t proto = 1; proto < protocolCount_; ++proto ) {
                        auto sp = std::make_shared< adcontrols::MassSpectrum >();                    
                        if ( auto& w = v[ proto ] )
                            waveform::translate( *sp, w, assignee );
                        ms << std::move(sp);
                    }
                    return true;
                }
                return false;
            }
            
            bool recentHistogram( adcontrols::MassSpectrum& ms
                                  , const std::array< std::shared_ptr< adcontrols::TimeDigitalHistogram >, max_protocol >& v
                                  , tdcdoc::mass_assignee_t assignee ) const {
                
                if ( !v.empty() && v[ 0 ] ) {

                    adcontrols::TimeDigitalHistogram::translate( ms, *v[ 0 ], assignee );

                    for ( uint32_t proto = 1; proto < protocolCount_; ++proto ) {

                        auto sp = std::make_shared< adcontrols::MassSpectrum >();

                        if ( auto& hgrm = v[ proto ] )
                            adcontrols::TimeDigitalHistogram::translate( *sp, *hgrm, assignee );

                        ms << std::move(sp);
                    }
                }
                return true;
            }

            void reset_accumulators( size_t count ) {

                protocolCount_ = count;

                std::for_each( accumulator_.begin(), accumulator_.end(), []( AverageData& d ){
                        d.reset();
                    });

                for ( auto& p: recent_longterm_histograms_ )
                    p = std::make_shared< adcontrols::TimeDigitalHistogram >();

            }
            
            size_t average_waveform( const acqrscontrols::u5303a::waveform& waveform );

            bool push_averaged_waveform( AverageData& d ) {
                
                const bool invertData = d.method_.mode() == acqrscontrols::u5303a::method::DigiMode::Digitizer;
                
                auto w = std::make_shared< acqrscontrols::u5303a::waveform >( d.method_
                                                                              , d.meta_
                                                                              , d.serialnumber_
                                                                              , d.wellKnownEvents_
                                                                              , d.timeSinceEpoch_
                                                                              , 0
                                                                              , d.timeSinceInject_
                                                                              , d.ident_
                                                                              , d.waveform_register_->data()
                                                                              , d.waveform_register_->size()
                                                                              , invertData );
                d.waveform_register_.reset();

                accumulated_waveforms_.emplace_back( w );
                
                // display data
                recent_waveforms_[ d.protocolIndex_ ] = w;

                return true;
            }

            std::array< std::shared_ptr< adcontrols::threshold_method >, 2 > threshold_methods_;
            std::array< std::pair< uint32_t, uint32_t >, 2 > threshold_action_counts_;
            std::shared_ptr< adcontrols::threshold_action > threshold_action_;
            std::shared_ptr< adcontrols::TofChromatogramsMethod > tofChromatogramsMethod_;
            std::atomic< double > trig_per_seconds_;
            std::vector< std::shared_ptr< acqrscontrols::u5303a::waveform > > accumulated_waveforms_;

            // periodic histograms (primary que) [time array] := (proto 0, proto 1, ...), (proto 0, proto 1, ...),...
            std::vector< std::shared_ptr< adcontrols::TimeDigitalHistogram > > periodic_histogram_que_;

            // long term averaged histograms
            std::array< std::shared_ptr< adcontrols::TimeDigitalHistogram >, max_protocol > recent_longterm_histograms_;
            
            // recent protocol sequence histograms (periodic); (proto 0, proto 1, ...)
            std::array< std::shared_ptr< adcontrols::TimeDigitalHistogram >, max_protocol > recent_periodic_histograms_;
            
            // recent protocol sequence waveforms (averaged)
            std::array< std::shared_ptr< const acqrscontrols::u5303a::waveform >, max_protocol > recent_waveforms_;

            // recent protocol sequence waveforms (raw)
            std::array< std::shared_ptr< const acqrscontrols::u5303a::waveform >, max_protocol > recent_raw_waveforms_;
            
            std::array< AverageData,   max_protocol > accumulator_;

            // meta data for initial waveform in the current accumulation range
            uint32_t protocolCount_;

            std::mutex mutex_;
        };
        
    }
}

using namespace acqrscontrols::u5303a;

tdcdoc::tdcdoc() : impl_( new impl() )
{
}

tdcdoc::~tdcdoc()
{
    delete impl_;
}

/*
  Master entry point from 'task' module
 */
bool
tdcdoc::accumulate_histogram( const_threshold_result_ptr timecounts )
{
    auto proto = timecounts->data()->method_.protocolIndex();
    auto count = timecounts->data()->method_.protocols().size();

    if ( count > max_protocol )
        return false;

    std::lock_guard< std::mutex > lock( impl_->mutex_ );
    
    if ( impl_->protocolCount_ != count )
        impl_->reset_accumulators( count );
    
    auto& d = impl_->accumulator_[ proto ];
    
    if ( ! d.histogram_register_ )
        d.histogram_register_ = std::make_shared< histogram_type >();
    
    if ( d.histogram_register_->append( *timecounts ) >= impl_->tofChromatogramsMethod_->numberOfTriggers() ) {
        
        // periodic histograms
        auto hgrm = std::make_shared< adcontrols::TimeDigitalHistogram >();

        d.histogram_register_->move( *hgrm );
        impl_->periodic_histogram_que_.emplace_back( hgrm );

        impl_->trig_per_seconds_ = hgrm->triggers_per_second();
        
        // dispatch histograms
        uint32_t index = hgrm->protocolIndex();
        
        // copy recent periodic histogram
        impl_->recent_periodic_histograms_[ index ] = hgrm;

        // accumulate into long term histogram
        (*impl_->recent_longterm_histograms_[ index ] ) += *hgrm;
    }
    
    return !impl_->periodic_histogram_que_.empty();
}

bool
tdcdoc::accumulate_waveform( std::shared_ptr< const acqrscontrols::u5303a::waveform > waveform )
{
    auto proto = waveform->method_.protocolIndex();
    auto count = waveform->method_.protocols().size();

    if ( ( count > max_protocol ) || ( proto >= count ) || count >= max_protocol )
        return false;

    std::lock_guard< std::mutex > lock( impl_->mutex_ );

    if ( impl_->protocolCount_ != count )
        impl_->reset_accumulators( count );
    
    auto& datum = impl_->accumulator_[ proto ];
    
    impl_->recent_raw_waveforms_[ proto ] = waveform; // data for display

    size_t nacc = datum.average_waveform( *waveform );
    size_t avrg = impl_->tofChromatogramsMethod_->numberOfTriggers();

    if ( datum.average_waveform( *waveform ) >= impl_->tofChromatogramsMethod_->numberOfTriggers() ) {

        impl_->push_averaged_waveform( datum );

    }

    return ! impl_->accumulated_waveforms_.empty();
}


bool
tdcdoc::makeChromatogramPoints( const std::shared_ptr< const waveform_type >& waveform
                                , std::vector< std::pair< double, double > >& results )
{
    typedef acqrscontrols::u5303a::waveform waveform_type;

    results.clear();

    const double xIncrement = waveform->meta_.xIncrement;

    auto wrap = adportable::waveform_wrapper< int32_t, waveform_type >( *waveform );

    double rms(0), dbase(0);
    double tic = adportable::spectrum_processor::tic( wrap.size(), wrap.begin(), dbase, rms, 7 );

    for ( auto& item: (*impl_->tofChromatogramsMethod_) ) {

        bool found( false );
            
        double time = item.time();
        double window = item.timeWindow();

        if ( time < 1.0e-9 || window < 1.0e-11 ) { // assume TIC requsted

            auto height = waveform->toVolts( *std::max_element( wrap.begin(), wrap.end() ) - dbase ) * 1000; // mV

            results.emplace_back( std::make_pair( tic, height ) );

        } else {

            double wsta = time - window / 2;
            double wend = time + window / 2;

            if ( wend < waveform->xy( 0 ).first || wsta > waveform->xy( waveform->size() - 1 ).first ) {

                results.emplace_back(  0, 0 );  // out of range

            } else {

                size_t ista = size_t( wsta < waveform->xy( 0 ).first ? 0 : ( wsta - waveform->xy( 0 ).first ) / xIncrement + 0.5 );
                size_t iend = size_t( ( wend - waveform->xy( 0 ).first ) / xIncrement + 0.5 ) + 1;
                if ( iend > waveform->size() )
                    iend = waveform->size();

                auto it = std::max_element( wrap.begin() + ista, wrap.begin() + iend );
                auto height = waveform->toVolts( *it - dbase ) * 1000;

                adportable::spectrum_processor::areaFraction fraction;

                fraction.lPos = ista;
                fraction.uPos = iend;
                // tbd
                fraction.lFrac = 0; // ( masses[ frac.lPos ] - lMass ) / ( masses[ frac.lPos ] - masses[ frac.lPos - 1 ] );
                fraction.uFrac = 0; // ( hMass - masses[ frac.uPos ] ) / ( masses[ frac.uPos + 1 ] - masses[ frac.uPos ] );
                
                double a = adportable::spectrum_processor::area( fraction, dbase, wrap.begin(), waveform->size() );
#if 0
                size_t n = iend - ibeg + 1;
                double area = a * waveform->meta_.scaleFactor + ( waveform->meta_.scaleOffset * n );
                if ( waveform->meta_.actualAverages )
                    area /= waveform->meta_.actualAverages;

                ADDEBUG() << "a=" << a << ", " << area;
#endif                
                results.emplace_back( a, height );
            }
            
        }
    }
    
    return true;
}

bool
tdcdoc::makeCountingChromatogramPoints( const adcontrols::TimeDigitalHistogram& histogram, std::vector< uint32_t >& results )
{
    for ( auto& item: (*impl_->tofChromatogramsMethod_) ) {

        double time = item.time();
        double window = item.timeWindow();

        results.emplace_back( histogram.accumulate( time, window ) );
    }

    return true;
}


std::shared_ptr< const waveform_type >
tdcdoc::averagedWaveform( uint64_t trigNumber )
{
    // TBD: trigNumber check,
    // return last one for now
    
    if ( ! impl_->accumulated_waveforms_.empty() )
        return impl_->accumulated_waveforms_.back();
    
    return 0;
}

size_t
tdcdoc::readAveragedWaveforms( std::vector< std::shared_ptr< const waveform_type > >& a )
{
    typedef std::shared_ptr< const waveform_type > waveform_ptr;
    
    if ( impl_->accumulated_waveforms_.size() > 1 ) {

        a.reserve( a.size() + impl_->accumulated_waveforms_.size() );

        std::lock_guard< std::mutex > lock( impl_->mutex_ );
        
        std::sort( impl_->accumulated_waveforms_.begin()
                   , impl_->accumulated_waveforms_.end(), []( const waveform_ptr& x, const waveform_ptr& y ){
                       return x->serialnumber_ < y->serialnumber_;
                   });
        
        std::move( impl_->accumulated_waveforms_.begin(), impl_->accumulated_waveforms_.end(), std::back_inserter( a ) );
        
        impl_->accumulated_waveforms_.clear();
        
        return a.size();
    }
    
    return 0;
}

size_t
tdcdoc::readTimeDigitalHistograms( std::vector< std::shared_ptr< const adcontrols::TimeDigitalHistogram > >& a )
{
    if ( ! impl_->periodic_histogram_que_.empty() ) {

        a.reserve( a.size() + impl_->periodic_histogram_que_.size() );

        std::lock_guard< std::mutex > lock( impl_->mutex_ );

        std::move( impl_->periodic_histogram_que_.begin(), impl_->periodic_histogram_que_.end(), std::back_inserter( a ) );

        impl_->periodic_histogram_que_.clear();

        return a.size();
    }

    return 0;
}

std::shared_ptr< adcontrols::TimeDigitalHistogram >
tdcdoc::longTermHistogram( int protocolIndex ) const
{
    std::lock_guard< std::mutex > lock( impl_->mutex_ );
    
    if ( impl_->recent_longterm_histograms_.empty() )
        return nullptr;

    // deep copy
    auto hgrm = std::make_shared< adcontrols::TimeDigitalHistogram >( *impl_->recent_longterm_histograms_[ protocolIndex ] );

    return hgrm;
}

std::vector< std::shared_ptr< adcontrols::TimeDigitalHistogram > >
tdcdoc::longTermHistograms() const
{
    std::lock_guard< std::mutex > lock( impl_->mutex_ );

    std::vector< std::shared_ptr< adcontrols::TimeDigitalHistogram > > d( impl_->recent_longterm_histograms_.size() );

    // deep copy
    std::transform( impl_->recent_longterm_histograms_.begin()
                    , impl_->recent_longterm_histograms_.begin() + impl_->protocolCount_, d.begin()
                    , []( const std::shared_ptr< const adcontrols::TimeDigitalHistogram >& h ){
                        return std::make_shared< adcontrols::TimeDigitalHistogram >( *h );
                    });
    return d;
}

std::vector< std::shared_ptr< adcontrols::TimeDigitalHistogram > >
tdcdoc::recentHistograms() const
{
    std::lock_guard< std::mutex > lock( impl_->mutex_ );

    std::vector< std::shared_ptr< adcontrols::TimeDigitalHistogram > > d( impl_->protocolCount_ );

    std::transform( impl_->recent_periodic_histograms_.begin()
                    , impl_->recent_periodic_histograms_.begin() + impl_->protocolCount_, d.begin()
                    , []( const std::shared_ptr< const adcontrols::TimeDigitalHistogram >& h ){
                        return std::make_shared< adcontrols::TimeDigitalHistogram >( *h );
                    });
    return d;
}

std::array< threshold_result_ptr, acqrscontrols::u5303a::nchannels >
tdcdoc::processThreshold( std::array< std::shared_ptr< const acqrscontrols::u5303a::waveform >, 2 > waveforms )
{
    if ( !waveforms[0] && !waveforms[1] ) // empty
        return std::array< threshold_result_ptr, 2 >();

    std::array< threshold_result_ptr, 2 > results;
    std::array< std::shared_ptr< adcontrols::threshold_method >, 2 > methods = impl_->threshold_methods_;  // todo: duplicate for thread safety

    for ( size_t i = 0; i < waveforms.size(); ++i ) {

        if ( waveforms[ i ] ) {

            auto& counts = impl_->threshold_action_counts_[ i ];
            
            results[ i ] = std::make_shared< acqrscontrols::u5303a::threshold_result >( waveforms[ i ] );

            const auto idx = waveforms[ i ]->method_.protocolIndex();
            if ( idx == 0 )
                counts.second++;
            
            if ( methods[ i ] && methods[ i ]->enable ) {
                
                find_threshold_timepoints( *waveforms[ i ], *methods[ i ], results[ i ]->indecies(), results[ i ]->processed() );

                bool result = acqrscontrols::threshold_action_finder()( results[i], impl_->threshold_action_ );
                
                if ( result )
                    counts.first++;
            }
        }
    }

    return results;
}


bool
tdcdoc::set_threshold_action( const adcontrols::threshold_action& m )
{
    impl_->threshold_action_ = std::make_shared< adcontrols::threshold_action >( m );
    
    for ( auto& counts: impl_->threshold_action_counts_ )
        counts = std::make_pair( 0, 0 );
    
    return true;
}

std::shared_ptr< const adcontrols::threshold_action >
tdcdoc::threshold_action() const
{
    return impl_->threshold_action_;
}

bool
tdcdoc::set_threshold_method( int channel, const adcontrols::threshold_method& m )
{
    if ( channel < impl_->threshold_methods_.size() ) {
        
        if ( auto prev = impl_->threshold_methods_[ channel ] ) {
            if ( *prev != m )
                clear_histogram();
        }
        
        impl_->threshold_methods_[ channel ] = std::make_shared< adcontrols::threshold_method >( m );
        return true;
    }

    return false;
}

std::shared_ptr< const adcontrols::threshold_method > 
tdcdoc::threshold_method( int channel ) const
{
    if ( channel < impl_->threshold_methods_.size() )
        return impl_->threshold_methods_[ channel ];
    return 0;
}

bool
tdcdoc::setTofChromatogramsMethod( const adcontrols::TofChromatogramsMethod& m )
{
    impl_->tofChromatogramsMethod_ = std::make_shared< adcontrols::TofChromatogramsMethod >( m );
    //-----------
    auto ptr( impl_->tofChromatogramsMethod_ );
    ADDEBUG() << "ChromatogramMethod: ";
    for ( size_t fcn = 0; fcn < ptr->size(); ++fcn ) {
        auto item = ptr->begin() + fcn;
        ADDEBUG() << "     " << fcn << ": " << item->formula() << ", " << int( item->intensityAlgorithm() );
    }
    //-----------
    return true;
}

std::shared_ptr< const adcontrols::TofChromatogramsMethod >
tdcdoc::tofChromatogramsMethod() const
{
    return impl_->tofChromatogramsMethod_;
}

void
tdcdoc::eraseTofChromatogramsMethod()
{
    impl_->tofChromatogramsMethod_.reset();
}

// static
void
tdcdoc::find_threshold_timepoints( const acqrscontrols::u5303a::waveform& data
                                   , const adcontrols::threshold_method& method
                                   , std::vector< uint32_t >& elements
                                   , std::vector<double>& processed )
{
    const bool findUp = method.slope == adcontrols::threshold_method::CrossUp;
    const unsigned int nfilter = static_cast<unsigned int>( method.response_time / data.meta_.xIncrement ) | 01;

    adportable::threshold_finder finder( findUp, nfilter );
    
    // workaround
    // if ( data.isDEAD() )
    //     return;
    // <<-- workaround

    if ( method.use_filter ) {

        waveform_type::apply_filter( processed, data, method );

        double level = method.threshold_level;
        finder( processed.begin(), processed.end(), elements, level );        
        
    } else {

        double level_per_trigger = ( method.threshold_level - data.meta_.scaleOffset ) / data.meta_.scaleFactor;
        double level = level_per_trigger;
        if ( data.meta_.actualAverages )
            level = level_per_trigger * data.meta_.actualAverages;

        if ( data.meta_.dataType == 2 )
            finder( data.begin<int16_t>(), data.end<int16_t>(), elements, level );
        else if ( data.meta_.dataType == 4 )
            finder( data.begin<int32_t>(), data.end<int32_t>(), elements, level );
    }
}

void
tdcdoc::clear_histogram()
{
    std::lock_guard< std::mutex > lock( impl_->mutex_ );
    
    impl_->threshold_action_counts_ = { { { 0, 0 } } };

    for ( auto& p: impl_->recent_longterm_histograms_ )
        p = std::make_shared< adcontrols::TimeDigitalHistogram >();
}

std::pair< uint32_t, uint32_t >
tdcdoc::threshold_action_counts( int channel ) const
{
    return impl_->threshold_action_counts_[ channel ];
}

std::shared_ptr< adcontrols::MassSpectrum >
tdcdoc::recentSpectrum( SpectrumType choice, mass_assignee_t assignee, int protocolIndex ) const
{
    auto ms = std::make_shared< adcontrols::MassSpectrum >();

    std::lock_guard< std::mutex > lock( impl_->mutex_ );

    if ( choice == Raw ) {
        if ( impl_->recentProfile( *ms, impl_->recent_raw_waveforms_, assignee ) )
            return ms;
    } else if ( choice == Profile ) {
        if ( impl_->recentProfile( *ms, impl_->recent_waveforms_, assignee ) )
            return ms;
    } else if ( choice == PeriodicHistogram ) {
        if ( impl_->recentHistogram( *ms, impl_->recent_periodic_histograms_, assignee ) )
            return ms;
    } else if ( choice == LongTermHistogram ) {
        if ( impl_->recentHistogram( *ms, impl_->recent_longterm_histograms_, assignee ) )
            return ms;
    }
    return nullptr;
}

double
tdcdoc::triggers_per_second() const
{
    return impl_->trig_per_seconds_;
}
