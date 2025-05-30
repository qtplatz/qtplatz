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

#include <acqrscontrols/ap240/histogram.hpp>
#include <acqrscontrols/ap240/threshold_result.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/waveform_filter.hpp>
#include <adportable/binary_serializer.hpp>
#include <adportable/debug.hpp>
#include <adportable/float.hpp>
#include <adportable/spectrum_processor.hpp>
#include <adportable/waveform_processor.hpp>
#include <adportable/advance.hpp>

#include <QObject>

using namespace ap240;

tdcdoc::tdcdoc( QObject * parent ) : QObject( parent )
                                   , histograms_( { std::make_shared<acqrscontrols::ap240::histogram>()
                                               , std::make_shared<acqrscontrols::ap240::histogram>() } )
{
}

tdcdoc::~tdcdoc()
{
}

void
tdcdoc::appendHistogram( std::array< threshold_result_ptr, acqrscontrols::ap240::nchannels > results )
{
    size_t channel = 0;
    for ( auto result: results ) {
        if ( result )
            histograms_[ channel ]->append( *result );
        ++channel;
    }
}

std::array< threshold_result_ptr, acqrscontrols::ap240::nchannels >
tdcdoc::handle_waveforms( std::array< std::shared_ptr< const acqrscontrols::ap240::waveform >, 2 > waveforms )
{
    if ( !waveforms[0] && !waveforms[1] ) // empty
        return std::array< threshold_result_ptr, 2 >();

    std::array< threshold_result_ptr, 2 > results;
    std::array< std::shared_ptr< adcontrols::threshold_method >, 2 > methods = threshold_methods_;  // todo: duplicate for thread safety

    for ( size_t i = 0; i < waveforms.size(); ++i ) {

        if ( waveforms[ i ] ) {

            results[ i ] = std::make_shared< acqrscontrols::ap240_threshold_result >( waveforms[ i ] );

            if ( methods[ i ] && methods[ i ]->enable ) {

                find_threshold_timepoints( *waveforms[ i ], *methods[ i ], results[ i ]->indices(), results[ i ]->processed() );
                
            }
        }
    }

    return results;
}


bool
tdcdoc::set_threshold_method( int channel, const adcontrols::threshold_method& m )
{
    namespace ap = adportable;
    
    if ( channel < threshold_methods_.size() ) {

        if ( auto prev = threshold_methods_[ channel ] ) {
            
            if ( prev->enable != m.enable ||
                 (!ap::compare<double>::approximatelyEqual( prev->threshold_level, m.threshold_level )) ||
                 (!ap::compare<double>::approximatelyEqual( prev->response_time, m.response_time )) ||
                 prev->slope != m.slope ||
                 prev->use_filter != m.use_filter ) {
                
                // clear histogram except for time_resolution change, which is for histogram calculation resolution
                histograms_[ channel ]->clear();
            }

            if ( m.use_filter ) {
                if ( ( prev->filter != m.filter ) ||
                     ( ( m.filter == adcontrols::threshold_method::SG_Filter ) && 
                       ( !ap::compare<double>::approximatelyEqual( prev->sgwidth, m.sgwidth ) ) ) ||
                     ( ( m.filter == adcontrols::threshold_method::DFT_Filter ) && 
                       ( ( !ap::compare<double>::approximatelyEqual( prev->hCutoffHz, m.hCutoffHz ) ) ||
                         ( !ap::compare<double>::approximatelyEqual( prev->lCutoffHz, m.lCutoffHz ) ) ||
                         ( m.complex_ != prev->complex_ ) ) ) ) {

                    // clear histogram except for time_resolution change, which is for histogram calculation resolution
                    histograms_[ channel ]->clear();                                 
                }
            }
        }

        std::lock_guard< std::mutex > lock( mutex_ );                
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
tdcdoc::find_threshold_timepoints( const acqrscontrols::ap240::waveform& data
                                   , const adcontrols::threshold_method& method
                                   , std::vector< uint32_t >& elements
                                   , std::vector<double>& processed )
{
    const bool findUp = method.slope == adcontrols::threshold_method::CrossUp;
    const unsigned int nfilter = static_cast<unsigned int>( method.response_time / data.meta_.xIncrement ) | 01;

    bool flag;

    if ( method.use_filter ) {

        processed.resize( data.size() );

        if ( data.meta_.dataType == 1 ) {
            for ( size_t i = 0; i < data.size(); ++i )
                processed[ i ] = data.toVolts( *(data.begin<int8_t>() + i) );
        } else if ( data.meta_.dataType == 4 ) {
            for ( size_t i = 0; i < data.size(); ++i )
                processed[ i ] = data.toVolts( *(data.begin<int32_t>() + i) );
        }

        if ( method.filter == adcontrols::threshold_method::SG_Filter ) {

            adcontrols::waveform_filter::sg::lowpass_filter( processed.size(), processed.data(), data.meta_.xIncrement, method.sgwidth );

        } else if ( method.filter == adcontrols::threshold_method::DFT_Filter ) {
            if ( method.complex_ )
                adcontrols::waveform_filter::fft4c::bandpass_filter( processed.size(), processed.data(), data.meta_.xIncrement, method.hCutoffHz, method.lCutoffHz );
            else
                adcontrols::waveform_filter::fft4g::bandpass_filter( processed.size(), processed.data(), data.meta_.xIncrement, method.hCutoffHz, method.lCutoffHz );
        }

        double level = method.threshold_level;
        auto it = processed.begin();
        while ( it != processed.end() ) {
            if ( ( it = adportable::waveform_processor().find_threshold_element( it, processed.end(), level, flag ) ) != processed.end() ) {
                if ( flag == findUp )
                    elements.push_back( std::distance( processed.begin(), it ) );
                adportable::advance( it, nfilter, processed.end() );
            }
        }

    } else {
            
        if ( data.meta_.dataType == 1 ) { // sizeof(int8_t)
            double level = ( method.threshold_level + data.meta_.scaleOffset ) / data.meta_.scaleFactor;
                    
            typedef int8_t T;
            auto it = data.begin<T>();
            while ( it != data.end<T>() ) {
                if ( ( it = adportable::waveform_processor().find_threshold_element( it, data.end<T>(), level, flag ) ) != data.end<T>() ) {
                    if ( flag == findUp )
                        elements.push_back( std::distance( data.begin<T>(), it ) );
                    std::advance( it, nfilter );
                }
            }
                    
        } else if ( data.meta_.dataType == 4 ) { // sizeof(int32_t)
            // scaleFactor = Volts/LSB  (1.0V FS = 0.00390625)
            double level_per_trigger = ( method.threshold_level + data.meta_.scaleOffset ) / data.meta_.scaleFactor;
            double level = level_per_trigger * data.meta_.actualAverages;
                    
            typedef int32_t T;
            auto it = data.begin<T>();
            while ( it != data.end<T>() ) {
                if ( ( it = adportable::waveform_processor().find_threshold_element( it, data.end<T>(), level, flag ) ) != data.end<T>() ) {
                    if ( flag == findUp )                        
                        elements.push_back( std::distance( data.begin<T>(), it ) );
                    adportable::advance( it, nfilter, data.end<T>() );
                }
            }
        }
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
#if 0
    acqrscontrols::ap240::metadata meta;
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
    prop.setDataInterpreterClsid( "ap240" );
        
    {
        acqrscontrols::ap240::device_data data;
        data.meta_ = meta;
        std::string ar;
        adportable::binary::serialize<>()( data, ar );
        prop.setDeviceData( ar.data(), ar.size() );
    }
        
    sp->setMSProperty( prop );

    if ( resolution > meta.xIncrement ) {

        std::vector< double > times, intens;
        acqrscontrols::ap240::histogram::average( hist, resolution, times, intens );
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
#endif
    return 0;
}

void
tdcdoc::clear_histogram()
{
    for ( auto h : histograms_ )
        h->clear();
}
