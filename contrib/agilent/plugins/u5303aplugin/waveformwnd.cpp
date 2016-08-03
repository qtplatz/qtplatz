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

#include "waveformwnd.hpp"
#include "constants.hpp"
#include "document.hpp"
#include <acqrscontrols/u5303a/tdcdoc.hpp>
#include <acqrscontrols/u5303a/threshold_result.hpp>
#include <acqrscontrols/u5303a/method.hpp>
#include <u5303a/digitizer.hpp>
#include <adcontrols/controlmethod/tofchromatogramsmethod.hpp>
#include <adcontrols/controlmethod/tofchromatogrammethod.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/trace.hpp>
#include <adplot/chromatogramwidget.hpp>
#include <adplot/peakmarker.hpp>
#include <adplot/spectrumwidget.hpp>
#include <adplot/spanmarker.hpp>
#include <adportable/float.hpp>
#include <adportable/spectrum_processor.hpp>
#include <adportable/debug.hpp>
#include <coreplugin/minisplitter.h>
#include <qwt_plot_marker.h>
#include <qwt_scale_widget.h>
#include <QSplitter>
#include <QBoxLayout>
#include <boost/format.hpp>
#include <boost/locale.hpp>
#include <boost/uuid/uuid.hpp>
#include <chrono>
#include <sstream>

using namespace u5303a;

WaveformWnd::WaveformWnd( QWidget * parent ) : QWidget( parent )
                                             , spw_( new adplot::SpectrumWidget )
                                             , hpw_( new adplot::SpectrumWidget )
                                             , tpw_( new adplot::ChromatogramWidget )
                                             , tickCount_( 0 )
{
    for ( auto& tp: tp_ )
        tp = std::make_shared< adcontrols::Trace >();

    // for ( auto& marker : peak_marker_ )
    //     marker = std::make_unique< adplot::PeakMarker >();

    init();
    connect( document::instance(), &document::dataChanged, this, &WaveformWnd::dataChanged );
}

WaveformWnd::~WaveformWnd()
{
    fini();
    delete tpw_;
    delete hpw_;
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

    // Histogram window marker
    int viewId = 0;
    for ( auto& histogram_window_marker : histogram_window_markers_ ) {
        int idx = 0;
        for ( auto& marker: histogram_window_marker ) {
            QColor color( tpw_->color( idx++ ) );
            color.setAlpha( 0x20 );
            marker = std::make_unique< adplot::SpanMarker >( color, QwtPlotMarker::VLine, 2.0 );
            marker->visible( false );
            marker->attach( viewId == 0 ? spw_ : hpw_ );
        }
        ++viewId;
    }
}

void
WaveformWnd::fini()
{
    for ( auto& histogram_window_marker : histogram_window_markers_ ) {
        for ( auto& marker: histogram_window_marker )
            marker->detach();
    }
}

void
WaveformWnd::onInitialUpdate()
{
    spw_->setKeepZoomed( false );
    hpw_->setKeepZoomed( false );

    handle_method( QString() );
    handle_threshold_method( 0 );
}

void
WaveformWnd::handle_threshold_method( int ch )
{
    if ( auto th = document::instance()->tdc()->threshold_method( ch ) ) {

        bool replot( false );
        double level_mV = th->threshold_level * 1.0e3;

        if ( !adportable::compare<double>::approximatelyEqual( threshold_markers_[ ch ]->yValue(), level_mV ) ) {
            threshold_markers_[ ch ]->setYValue( level_mV );
            replot = true;
        }
        if ( th->enable != threshold_markers_[ ch ]->isVisible() ) {
            threshold_markers_[ ch ]->setVisible( th->enable );
            replot = true;
        }
        if ( replot )
            spw_->replot();
    }
}

void
WaveformWnd::handle_method( const QString& )
{
    if ( auto ptr = document::instance()->method() ) {
        if ( ( ptr->channels() & 0x01 ) == 0 && sp_[ 0 ] ) {
            sp_[ 0 ]->resize( 0 );
            spw_->setData( sp_[ 0 ], 0, false );
        }
        if ( ( ptr->channels() & 0x02 ) == 0 && sp_[ 1 ] ) {
            sp_[ 1 ]->resize( 0 );
            spw_->setData( sp_[ 1 ], 1, true );
        }
        spw_->setAxisAutoScale( QwtPlot::yLeft, true );// ptr->threshold_.autoScale );
        //spw_->setAxisAutoScale( QwtPlot::yRight, ptr->ch2_.autoScale );
    }
}

void
WaveformWnd::dataChanged( const boost::uuids::uuid& uuid, int idx )
{
    if ( uuid == trace_observer ) {
        auto method = document::instance()->tdc()->tofChromatogramsMethod();

        std::vector< std::shared_ptr< adcontrols::Trace > > traces;

        document::instance()->getTraces( traces );
        
        QString footer = QString( "#Method: %1, #Traces: %2" ).arg( QString::number( method->size() ), QString::number( traces.size() ) );
        auto item = method->begin();
        QVector< QwtText > titles;
        for ( uint32_t fcn = 0; fcn < traces.size() && fcn < method->size(); ++fcn, ++item ) {

            auto& trace = traces[ fcn ];
            if ( fcn == 0 && trace->size() > 0 ) {
                double seconds = trace->x( trace->size() - 1 );
                footer += QString( "  Time: %1  " ).arg( QString::number( seconds / 60, 'f', 3 ) );
                tpw_->setTitle( footer );
            }

            bool yRight = trace->isCountingTrace();
            tpw_->setData( *trace, fcn, yRight );

            // title for legends
            char c = item->intensityAlgorithm() == item->eCounting ? 'C' : item->intensityAlgorithm() == item->ePeakAreaOnProfile ? 'A' : 'H';
            auto formula = adcontrols::ChemicalFormula::formatFormula( item->formula() );
            if ( formula.empty() )
                titles << QwtText( QString( "%1: %2[%3]" ).arg( QString::number( ++idx )
                                                                , QString::fromStdString( item->formula() ), QString( c ) ) );
            else
                titles << QwtText( QString( "%1: %2[%3]" ).arg( QString::number( ++idx )
                                                                , QString::fromStdString( formula ), QString( c ) ), QwtText::RichText );
        }

        int idx( 0 );
        for ( auto curve : tpw_->itemList( QwtPlotItem::Rtti_PlotCurve ) ) {
            if ( titles.size() > idx )
                curve->setTitle( titles [ idx++ ] );
            else
                curve->setTitle( QwtText() );
        }
        
    } else {    
        if ( auto sp = document::instance()->recentSpectrum( uuid, idx ) ) {

            if ( uuid == u5303a_observer ) {
                // waveform (analog)
                double seconds = sp->getMSProperty().timeSinceInjection();
                QString title = QString( "U5303A: Elapsed time: %1s, Trig# %2" ).arg( QString::number( seconds, 'f', 4 )
                                                                                      , QString::number( sp->getMSProperty().trigNumber() ) );
            
                spw_->setTitle( title );
                spw_->setData( sp, idx, bool( idx ) );
                spw_->setKeepZoomed( true );

            } else if ( uuid == histogram_observer ) {
                // histogram

                double rate = document::instance()->triggers_per_second();
            
                QString title = QString( "U5303A: %1 samples since trig# %2, at rate %3/s" ).arg(
                    QString::number( sp->getMSProperty().numAverage() )
                    , QString::number( sp->getMSProperty().trigNumber() )
                    , QString::number( rate, 'f', 2 ) );

                hpw_->setTitle( title );
                hpw_->setData( sp, idx, bool( idx ) );
                hpw_->setKeepZoomed( true );

            } else if ( uuid == ap240_observer ) {

                double seconds = sp->getMSProperty().timeSinceInjection();
                QString title = QString( "AP240: Elapsed time: %1s, Trig# %2" ).arg( QString::number( seconds, 'f', 4 )
                                                                                     , QString::number( sp->getMSProperty().trigNumber() ) );
            
                spw_->setTitle( title );
                spw_->setData( sp, idx, bool( idx ) );

            } else {
                ADDEBUG() << "Unhandled observer";
            }
        
        } else {

            // clear spectrum
            static auto empty = std::make_shared< adcontrols::MassSpectrum >();
        
            if ( uuid == u5303a_observer ) {
            
                spw_->setData( empty, idx, bool( idx ) );

            } else if ( uuid == histogram_observer ) {

                hpw_->setData( empty, idx, bool( idx ) );
                                                                      
            }

        }
    }
}

void
WaveformWnd::setMethod( const adcontrols::TofChromatogramsMethod& m )
{
    ADDEBUG() << "setMethod .....";
    
    int idx = 0;
    for ( auto& item : m ) {

        if ( idx < histogram_window_markers_[0].size() ) {

            if ( item.formula() == "TIC" || ( item.time() < 1.0e-9 ) ) {
                
                for ( size_t i = 0; i < histogram_window_markers_.size(); ++i ) {
                    auto& marker = histogram_window_markers_[ i ][ idx ];
                    marker->visible( false );
                }
                
            } else {
                using adcontrols::metric::scale_to_micro;
                
                double lower = item.time() - item.timeWindow() / 2;
                double upper = item.time() + item.timeWindow() / 2;

                for ( size_t i = 0; i < histogram_window_markers_.size(); ++i ) {
                    auto& marker = histogram_window_markers_[ i ][ idx ];
                    marker->setValue( scale_to_micro( lower ), scale_to_micro( upper ) );
                    marker->visible( true );
                }
            }
        }
        ++idx;
    }
    spw_->replot();
    hpw_->replot();
}
