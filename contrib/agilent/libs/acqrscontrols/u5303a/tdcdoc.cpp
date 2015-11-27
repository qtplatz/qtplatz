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

#include "tdcdoc.hpp"

#include <acqrscontrols/u5303a/histogram.hpp>
#include <acqrscontrols/u5303a/threshold_result.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/tofchromatogramsmethod.hpp>
#include <adcontrols/tofchromatogrammethod.hpp>
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
            
            impl() : histograms_( { std::make_shared< histogram_type >(), std::make_shared<histogram_type>() } ) {
            }

            std::array< std::shared_ptr< adcontrols::threshold_method >, 2 > threshold_methods_;
            std::array< std::shared_ptr< acqrscontrols::u5303a::histogram >, 2 > histograms_;
            std::array< std::pair< uint32_t, uint32_t >, 2 > threshold_action_counts_;
            std::shared_ptr< adcontrols::threshold_action > threshold_action_;
            std::shared_ptr< adcontrols::TofChromatogramsMethod > tofChromatogramsMethod_;
            std::atomic< double > trig_per_seconds_;
            std::deque< std::shared_ptr< acqrscontrols::u5303a::waveform > > averaged_;
            std::shared_ptr< averager_type > averager_;

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

void
tdcdoc::appendHistogram( std::array< threshold_result_ptr, acqrscontrols::u5303a::nchannels > results )
{
    size_t channel = 0;
    for ( auto result: results ) {
        if ( result )
            impl_->histograms_[ channel ]->append( *result );
        ++channel;
    }
}

bool
tdcdoc::average( std::shared_ptr< const acqrscontrols::u5303a::waveform > waveform )
{
    typedef adportable::waveform_wrapper< int16_t, acqrscontrols::u5303a::waveform > u16wrap;
    typedef adportable::waveform_wrapper< int32_t, acqrscontrols::u5303a::waveform > u32wrap;

    std::lock_guard< std::mutex > lock( impl_->mutex_ );
    
    if ( ! impl_->averager_ ) {
        
        if ( waveform->dataType() == 2 )
            impl_->averager_ = std::make_shared< averager_type >( u16wrap( *waveform ) );
        else
            impl_->averager_ = std::make_shared< averager_type >( u32wrap( *waveform ) );
        
    } else {
        
        if ( waveform->dataType() == 2 )
            (*impl_->averager_) += u16wrap( *waveform );
        else
            (*impl_->averager_) += u32wrap( *waveform );                
        
        if ( impl_->averager_->actualAverages() >= impl_->tofChromatogramsMethod_->numberOfTriggers() ) {
            
            auto w = std::make_shared< acqrscontrols::u5303a::waveform >(*waveform, impl_->averager_->data(), impl_->averager_->size(), true );
            
            impl_->averaged_.emplace_back( w );
            impl_->averager_.reset();
        }
        
    }
    return !impl_->averaged_.empty();
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

                results.emplace_back( std::make_pair( 0, 0 ) );  // out of range

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
                ADDEBUG() << "a=" << a << ", " << height;
                results.emplace_back( std::make_pair( a, height ) );
            }
            
        }
    }
    
    return true;
}

std::shared_ptr< const waveform_type >
tdcdoc::averagedWaveform( uint64_t trigNumber )
{
    // TBD: trigNumber check,
    // return last one for now
    
    if ( ! impl_->averaged_.empty() )
        return impl_->averaged_.back();
    
    return 0;
}

size_t
tdcdoc::readAveragedWaveforms( std::vector< std::shared_ptr< const waveform_type > >& a )
{
    typedef std::shared_ptr< const waveform_type > waveform_ptr;
    
    if ( ! impl_->averaged_.empty() ) {

        a.reserve( a.size() + impl_->averaged_.size() );

        std::lock_guard< std::mutex > lock( impl_->mutex_ );

        std::sort( impl_->averaged_.begin(), impl_->averaged_.end(), []( const waveform_ptr& a, const waveform_ptr& b ){
                return a->serialnumber_ < b->serialnumber_;
            });

        std::move( impl_->averaged_.begin(), impl_->averaged_.end(), std::back_inserter( a ) );

        impl_->averaged_.clear();

        return a.size();
    }

    return 0;
}

std::array< threshold_result_ptr, acqrscontrols::u5303a::nchannels >
tdcdoc::handle_waveforms( std::array< std::shared_ptr< const acqrscontrols::u5303a::waveform >, 2 > waveforms )
{
    if ( !waveforms[0] && !waveforms[1] ) // empty
        return std::array< threshold_result_ptr, 2 >();

    std::array< threshold_result_ptr, 2 > results;
    std::array< std::shared_ptr< adcontrols::threshold_method >, 2 > methods = impl_->threshold_methods_;  // todo: duplicate for thread safety

    for ( size_t i = 0; i < waveforms.size(); ++i ) {

        if ( waveforms[ i ] ) {
            
            results[ i ] = std::make_shared< acqrscontrols::u5303a::threshold_result >( waveforms[ i ] );

            if ( methods[ i ] && methods[ i ]->enable ) {

                find_threshold_timepoints( *waveforms[ i ], *methods[ i ], results[ i ]->indecies(), results[ i ]->processed() );
                
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
                impl_->histograms_[ channel ]->clear();
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
        else
            finder( data.begin<int32_t>(), data.end<int32_t>(), elements, level );
    }
}

void
tdcdoc::update_rate( size_t trigCount, const std::pair<uint64_t, uint64_t>& timeSinceEpoch )
{
    int64_t duration = timeSinceEpoch.second - timeSinceEpoch.first;
    if ( duration )
        impl_->trig_per_seconds_ = double( trigCount ) / ( double( timeSinceEpoch.second - timeSinceEpoch.first ) * 1.0e-9 );
}

double
tdcdoc::trig_per_seconds() const
{
    return impl_->trig_per_seconds_;
}


std::shared_ptr< adcontrols::MassSpectrum >
tdcdoc::getHistogram( double resolution, int channel, size_t& trigCount, std::pair<uint64_t, uint64_t>& timeSinceEpoch ) const
{
    acqrscontrols::u5303a::metadata meta;
    std::vector< std::pair< double, uint32_t > > hist;

    auto sp = std::make_shared< adcontrols::MassSpectrum >();
    sp->setCentroid( adcontrols::CentroidNative );

    std::pair<uint32_t,uint32_t> serialnumber;

    trigCount = impl_->histograms_[ channel ]->getHistogram( hist, meta, serialnumber, timeSinceEpoch );

    const auto& histogram = impl_->histograms_[ channel ];

    using namespace adcontrols::metric;
    
    adcontrols::MSProperty prop = sp->getMSProperty();
    
    adcontrols::MSProperty::SamplingInfo info( 0 /* int interval (must be zero) */
                                               , uint32_t( meta.initialXOffset / meta.xIncrement + 0.5 )
                                               , uint32_t( meta.actualPoints ) // this is for acq. time range calculation
                                               , uint32_t( trigCount )
                                               , 0 /* mode */);
    info.fSampInterval( meta.xIncrement );
    prop.acceleratorVoltage( 3000 );
    prop.setSamplingInfo( info );
        
    prop.setTimeSinceInjection( meta.initialXTimeSeconds );
    prop.setTimeSinceEpoch( histogram->timeSinceEpoch() );
    prop.setNumAverage( uint32_t( trigCount ) );
    prop.setTrigNumber( histogram->trigNumber() );
    prop.setDataInterpreterClsid( "u5303a" );
        
    {
        acqrscontrols::u5303a::device_data data;
        data.meta_ = meta;
        std::string ar;
        adportable::binary::serialize<>()( data, ar );
        prop.setDeviceData( ar.data(), ar.size() );
    }
        
    sp->setMSProperty( prop );

    if ( resolution > meta.xIncrement ) {

        std::vector< double > times, intens;
        acqrscontrols::u5303a::histogram::average( hist, resolution, times, intens );
        sp->resize( times.size() );
        sp->setTimeArray( times.data() );
        sp->setIntensityArray( intens.data() );

    } else {
        sp->resize( hist.size() );
        for ( size_t idx = 0; idx < hist.size(); ++idx ) {
            sp->setTime( idx, hist[ idx ].first );
            sp->setIntensity( idx, hist[ idx ].second );
        }
    }
    return sp;
}

void
tdcdoc::clear_histogram()
{
    impl_->threshold_action_counts_ = { { { 0, 0 } } };

    for ( auto h : impl_->histograms_ )
        h->clear();
}

std::pair< uint32_t, uint32_t >
tdcdoc::threshold_action_counts( int channel ) const
{
    return impl_->threshold_action_counts_[ channel ];
}
