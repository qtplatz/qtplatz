/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
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

#include "waveformwnd.hpp"
#include "document.hpp"
#include <acqrscontrols/ap240/threshold_result.hpp>
#include <acqrscontrols/acqiris_waveform.hpp>
#include <ap240/digitizer.hpp>
#include <adplot/spectrumwidget.hpp>
#include <adplot/chromatogramwidget.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/samplinginfo.hpp>
#include <adcontrols/trace.hpp>
#include <adportable/float.hpp>
#include <adportable/spectrum_processor.hpp>
#include <coreplugin/minisplitter.h>
#include <qwt_plot_marker.h>
#include <qwt_scale_widget.h>
#include <QSplitter>
#include <QBoxLayout>
#include <boost/format.hpp>
#include <chrono>
#include <sstream>

using namespace ap240;

WaveformWnd::WaveformWnd( QWidget * parent ) : QWidget( parent )
                                             , spw_( new adplot::SpectrumWidget )
                                             , hpw_( new adplot::SpectrumWidget )
                                             , tpw_( new adplot::ChromatogramWidget )
                                             , tickCount_( 0 )
                                             , temperature_( 0 )
{
    for ( auto& tp: tp_ )
        tp = std::make_shared< adcontrols::Trace >();

    init();
}

WaveformWnd::~WaveformWnd()
{
    fini();
    delete tpw_;
    delete spw_;
}

void
WaveformWnd::init()
{
    Core::MiniSplitter * splitter = new Core::MiniSplitter;
    do {
        splitter->addWidget( tpw_ );
        splitter->addWidget( hpw_ );
        splitter->addWidget( spw_ );
        splitter->setStretchFactor( 0, 1 );
        splitter->setStretchFactor( 1, 3 );
        splitter->setOrientation( Qt::Vertical );
    } while(0);

    tpw_->setMinimumHeight( 80 );
    spw_->setMinimumHeight( 80 );
    hpw_->setMinimumHeight( 80 );
    spw_->axisWidget( QwtPlot::yLeft )->scaleDraw()->setMinimumExtent( 80 );
    hpw_->axisWidget( QwtPlot::yLeft )->scaleDraw()->setMinimumExtent( 80 );    

    spw_->setAxisTitle( QwtPlot::yLeft, tr( "<i>mV</i>" ) );
    spw_->setAxisTitle( QwtPlot::yRight, tr( "<i>mV</i>" ) );
    spw_->enableAxis( QwtPlot::yRight, true );
    
    spw_->setAxis( adplot::SpectrumWidget::HorizontalAxisTime );
    spw_->setKeepZoomed( false );

    hpw_->setAxisTitle( QwtPlot::yLeft, tr( "<i>Counts</i>" ) );
    hpw_->setAxisTitle( QwtPlot::yRight, tr( "<i>Counts</i>" ) );
    hpw_->enableAxis( QwtPlot::yRight, true );
    
    hpw_->setAxis( adplot::SpectrumWidget::HorizontalAxisTime );
    hpw_->setKeepZoomed( false );
    hpw_->setAutoAnnotation( false );
    
    spw_->link( hpw_ );
//    hpw_->link( spw_ );
    
    tpw_->setAxisTitle( QwtPlot::yLeft, tr( "<i>mV</i>" ) );
    tpw_->setAxisTitle( QwtPlot::yRight, tr( "<i>mV</i>" ) );
    tpw_->enableAxis( QwtPlot::yRight, true );

    QBoxLayout * layout = new QVBoxLayout( this );
    layout->setMargin( 0 );
    layout->setSpacing( 2 );
    layout->addWidget( splitter );

    const QColor colors[] = { QColor( 0x00, 0x00, 0xff, 0x80 ), QColor( 0xff, 0x00, 0x00, 0x80 ) };
    for ( size_t i = 0; i < threshold_markers_.size(); ++i ) {
        threshold_markers_[ i ] = new QwtPlotMarker();
        threshold_markers_[ i ]->setLineStyle( QwtPlotMarker::HLine );
        threshold_markers_[ i ]->setLinePen( colors[ i ], 0, Qt::DotLine );
        threshold_markers_[ i ]->setYAxis( ( i == 1 ) ? QwtPlot::yRight : QwtPlot::yLeft );
        threshold_markers_[ i ]->setYValue( ( i + 1 ) * 100 );
        threshold_markers_[ i ]->attach( spw_ );
    }

}

void
WaveformWnd::fini()
{
}

void
WaveformWnd::onInitialUpdate()
{
    spw_->setKeepZoomed( false );
}

void
WaveformWnd::handle_threshold_method( int ch )
{
    if ( auto method = document::instance()->threshold_method( ch ) ) {

        bool replot( false );
        double level_mV = method->threshold_level * 1.0e3;

        if ( !adportable::compare<double>::approximatelyEqual( threshold_markers_[ ch ]->yValue(), level_mV ) ) {
            threshold_markers_[ ch ]->setYValue( level_mV );
            replot = true;
        }
        if ( method->enable != threshold_markers_[ ch ]->isVisible() ) {
            threshold_markers_[ ch ]->setVisible( method->enable );
            replot = true;
        }
        if ( replot )
            spw_->replot();
    }
}

void
WaveformWnd::handle_method( const QString& )
{
    if ( auto ptr = document::instance()->controlMethod() ) {
        if ( ( ptr->channels_ & 0x01 ) == 0 && sp_[ 0 ] ) {
            sp_[ 0 ]->resize( 0 );
            spw_->setData( sp_[ 0 ], 0, false );
        }
        if ( ( ptr->channels_ & 0x02 ) == 0 && sp_[ 1 ] ) {
            sp_[ 1 ]->resize( 0 );
            spw_->setData( sp_[ 1 ], 1, true );
        }
        spw_->setAxisAutoScale( QwtPlot::yLeft, ptr->ch1_.autoScale );
        spw_->setAxisAutoScale( QwtPlot::yRight, ptr->ch2_.autoScale );
    }
}

void
WaveformWnd::handle_aqdrv4_waveform()
{
    if ( auto wform = document::instance()->findAqDrv4Waveform() ) {
        auto ms = std::make_shared< adcontrols::MassSpectrum >();
        if ( acqrscontrols::aqdrv4::waveform::translate( *ms, *wform, 1000 ) ) {
            const auto& info = ms->getMSProperty().samplingInfo();
            hpw_->setTitle( ( boost::format( "triggers: %1%;&nbsp;&nbsp;%2% triggers in que; &nbsp; rate = %3% trig/s" )
                              % info.numberOfTriggers()
                              % document::instance()->unprocessed_trigger_counts()
                              % document::instance()->triggers_per_second()).str() );
            spw_->setData( ms, 0, false );
            spw_->setKeepZoomed( true );            
        }
    }
}

void
WaveformWnd::handle_waveform()
{
    auto pair = document::instance()->findWaveform();

    double resolution = 0.0;
    if ( auto tm = document::instance()->threshold_method( 0 ) )
        resolution = tm->time_resolution;

    if ( auto ms = document::instance()->getHistogram( 0, resolution ) ) {

        if ( ms->size() > 0 )
            hpw_->setData( ms, 0 );
        
        const auto& info = ms->getMSProperty().samplingInfo();
        hpw_->setTitle( ( boost::format( "triggers: %1%;&nbsp;&nbsp;%2% triggers in que; &nbsp; rate = %3% trig/s" )
                          % info.numberOfTriggers()
                          % document::instance()->unprocessed_trigger_counts()
                          % document::instance()->triggers_per_second()).str() );
        if ( ( tickCount_++ % 5 ) == 0 )
            document::instance()->save_histogram( tickCount_, *ms );
    }
    
    if ( auto ms = document::instance()->getHistogram( 1, resolution ) ) {

        if ( ms->size() > 0 )
            hpw_->setData( ms, 1 );

    }    

// #if defined DEBUG || defined _DEBUG && 0
//     if ( auto ms = document::instance()->getHistogram( 0.0 ) ) {
//         hpw_->setData( ms, 1 );
//         hpw_->setAlpha( 1, 0x20 );
//     }
// #endif

    std::ostringstream o;

    double levels_mV[] = { document::instance()->threshold_method(0)->threshold_level * 1000.0
                         , document::instance()->threshold_method( 1 )->threshold_level * 1000.0 };

    for ( auto result: { pair.first, pair.second } ) {
        
        if ( result ) {
            auto waveform = result->data();
            
            double timestamp = waveform->meta_.initialXTimeSeconds;
            int channel = waveform->meta_.channel - 1;
            if ( channel > 2 )
                continue; // this should not be happend.
            
            sp_[ channel ] = std::make_shared< adcontrols::MassSpectrum >();
            auto sp = sp_[ channel ];
            auto tp = tp_[ channel ];

            sp->setCentroid( adcontrols::CentroidNone );
            sp->resize( waveform->size() );

            if ( result->processed().size() == waveform->size() ) {
                size_t idx = 0;
                for ( auto it = result->processed().begin(); it != result->processed().end(); ++it )
                    sp->setIntensity( idx++, *it * 1000 ); // V
            } else  if ( waveform->meta_.dataType == 1 ) {
                size_t idx = 0;
                for ( auto it = waveform->begin<int8_t>(); it != waveform->end<int8_t>(); ++it )
                    sp->setIntensity( idx++, waveform->toVolts( *it ) * 1000.0 ); // mV
            } else if ( waveform->meta_.dataType == 2 ) {
                size_t idx = 0;                
                for ( auto it = waveform->begin<int16_t>(); it != waveform->end<int16_t>(); ++it )
                    sp->setIntensity( idx++, *it );
            } else if ( waveform->meta_.dataType == 4 ) {
                double dbase, rms;
                double tic = adportable::spectrum_processor::tic( waveform->size(), waveform->begin<int32_t>(), dbase, rms );
                
                size_t idx = 0;                
                for ( auto it = waveform->begin<int32_t>(); it != waveform->end<int32_t>(); ++it ) {
                    sp->setIntensity( idx++, waveform->toVolts( *it - dbase ) * 1000 );
                }
            }
            
            adcontrols::MSProperty prop = sp->getMSProperty();
            double zhalf = waveform->meta_.initialXOffset < 0 ? -0.5 : 0.5;
            adcontrols::SamplingInfo info( waveform->meta_.xIncrement
                                           , waveform->meta_.initialXOffset
                                           , uint32_t( waveform->meta_.initialXOffset / waveform->meta_.xIncrement + zhalf )
                                           , uint32_t( waveform->size() )
                                           , waveform->meta_.actualAverages
                                           , 0 );
            
            //info.fSampInterval( waveform->meta_.xIncrement );
            info.horPos( waveform->meta_.horPos );
            
            prop.setSamplingInfo( info );
            prop.setAcceleratorVoltage( 3000 );

            using namespace adcontrols::metric;
            prop.setTimeSinceInjection( timestamp * 1.0e6 ); // microseconds
            prop.setTimeSinceEpoch( waveform->timeSinceEpoch_ );
            prop.setDataInterpreterClsid( "ap240" );
            sp->setMSProperty( prop );

            if ( o.str().empty() )
                o << boost::format( "%ddegC Time: %.3lf" ) % temperature_ % waveform->meta_.initialXTimeSeconds;

            if ( !result->indices().empty() ) {
                o << boost::format( " CH%d level: [%.0fmV]= " )  % ( channel + 1 ) % levels_mV[ channel ];
                for ( int i = 0; i < 5 && i < result->indices().size(); ++i ) {
                    double t0 = sp->getTime( result->indices()[i] ) * 1.0e6;
                    o << boost::format( "(%.4lf)" )  % t0;
                }
            }

            double dbase(0), rms(0), tic(0);            
            tic = adportable::spectrum_processor::tic( sp->size(), sp->getIntensityArray(), dbase, rms );
            tp->append( waveform->serialnumber_, timestamp, tic );
            tpw_->setData( tp, channel, channel );
            
            // prop.setDeviceData(); TBA
            spw_->setData( sp, channel, channel );
            spw_->setKeepZoomed( true );
        }

        spw_->setTitle( o.str() );
    }
    document::instance()->waveform_drawn();
}

void
WaveformWnd::setTemperature( int temp )
{
    temperature_ = temp;
}
