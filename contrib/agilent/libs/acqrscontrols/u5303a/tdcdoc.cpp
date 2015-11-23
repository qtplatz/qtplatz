/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include <adcontrols/waveform.hpp>
#include <adportable/binary_serializer.hpp>
#include <adportable/debug.hpp>
#include <adportable/float.hpp>
#include <adportable/spectrum_processor.hpp>
#include <adportable/threshold_finder.hpp>
#include <adportable/waveform_processor.hpp>

using namespace acqrscontrols::u5303a;

tdcdoc::tdcdoc() : histograms_( { std::make_shared< histogram_type >(), std::make_shared<histogram_type>() } )
{
}

tdcdoc::~tdcdoc()
{
}

void
tdcdoc::appendHistogram( std::array< threshold_result_ptr, acqrscontrols::u5303a::nchannels > results )
{
    size_t channel = 0;
    for ( auto result: results ) {
        if ( result )
            histograms_[ channel ]->append( *result );
        ++channel;
    }
}

std::array< threshold_result_ptr, acqrscontrols::u5303a::nchannels >
tdcdoc::handle_waveforms( std::array< std::shared_ptr< const acqrscontrols::u5303a::waveform >, 2 > waveforms )
{
    if ( !waveforms[0] && !waveforms[1] ) // empty
        return std::array< threshold_result_ptr, 2 >();

    std::array< threshold_result_ptr, 2 > results;
    std::array< std::shared_ptr< adcontrols::threshold_method >, 2 > methods = threshold_methods_;  // todo: duplicate for thread safety

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
    threshold_action_ = std::make_shared< adcontrols::threshold_action >( m );
    
    for ( auto& counts: threshold_action_counts_ )
        counts = std::make_pair( 0, 0 );
    
    return true;
}

std::shared_ptr< const adcontrols::threshold_action >
tdcdoc::threshold_action() const
{
    return threshold_action_;
}

bool
tdcdoc::set_threshold_method( int channel, const adcontrols::threshold_method& m )
{
    if ( channel < threshold_methods_.size() ) {
        
        if ( auto prev = threshold_methods_[ channel ] ) {
            if ( *prev != m )
                histograms_[ channel ]->clear();
        }
        
        threshold_methods_[ channel ] = std::make_shared< adcontrols::threshold_method >( m );
        return true;
    }

    return false;
}

std::shared_ptr< const adcontrols::threshold_method > 
tdcdoc::threshold_method( int channel ) const
{
    if ( channel < threshold_methods_.size() )
        return threshold_methods_[ channel ];
    return 0;
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
    
    bool flag;

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
        trig_per_seconds_ = double( trigCount ) / ( double( timeSinceEpoch.second - timeSinceEpoch.first ) * 1.0e-9 );
}

double
tdcdoc::trig_per_seconds() const
{
    return trig_per_seconds_;
}


std::shared_ptr< adcontrols::MassSpectrum >
tdcdoc::getHistogram( double resolution, int channel, size_t& trigCount, std::pair<uint64_t, uint64_t>& timeSinceEpoch ) const
{
    acqrscontrols::u5303a::metadata meta;
    std::vector< std::pair< double, uint32_t > > hist;

    auto sp = std::make_shared< adcontrols::MassSpectrum >();
    sp->setCentroid( adcontrols::CentroidNative );

    std::pair<uint32_t,uint32_t> serialnumber;

    trigCount = histograms_[ channel ]->getHistogram( hist, meta, serialnumber, timeSinceEpoch );

    const auto& histogram = histograms_[ channel ];

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
    threshold_action_counts_ = { { { 0, 0 } } };

    for ( auto h : histograms_ )
        h->clear();
}

std::pair< uint32_t, uint32_t >
tdcdoc::threshold_action_counts( int channel ) const
{
    return threshold_action_counts_[ channel ];
}
