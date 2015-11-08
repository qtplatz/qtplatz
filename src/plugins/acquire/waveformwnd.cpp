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

#include "waveformwnd.hpp"
#include "constants.hpp"
#include "document.hpp"
#include <adplot/spectrumwidget.hpp>
#include <adplot/chromatogramwidget.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/descriptions.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/trace.hpp>
#include <adcontrols/traceaccessor.hpp>
#include <adportable/float.hpp>
#include <adportable/spectrum_processor.hpp>
#include <adportable/debug.hpp>
#include <coreplugin/minisplitter.h>
#include <qwt_plot_marker.h>
#include <qwt_scale_widget.h>
#include <QSplitter>
#include <QBoxLayout>
#include <boost/format.hpp>
#include <boost/uuid/uuid.hpp>
#include <chrono>
#include <sstream>

namespace acquire {

    class WaveformWnd::impl {
        WaveformWnd * this_;
    public:
        impl( WaveformWnd * parent ) : this_( parent )
                                     , spw_( new adplot::SpectrumWidget )
                                     , hpw_( new adplot::SpectrumWidget )
                                     , tpw_( new adplot::ChromatogramWidget )
                                     , tickCount_( 0 ) {
            
        }

        void init();
        void fini() {}
        
        std::unique_ptr< adplot::SpectrumWidget> spw_;
        std::unique_ptr< adplot::SpectrumWidget> hpw_;
        std::unique_ptr< adplot::ChromatogramWidget > tpw_;
        size_t tickCount_;
        
        std::map< boost::uuids::uuid, std::shared_ptr< const adcontrols::MassSpectrum > > sp_;
        std::map< std::pair<boost::uuids::uuid, int >, std::shared_ptr< adcontrols::Trace > > traces_;

    };
    
}

using namespace acquire;

WaveformWnd::WaveformWnd( QWidget * parent ) : QWidget( parent )
                                             , impl_( new impl( this ) )
{
    impl_->init();
}

WaveformWnd::~WaveformWnd()
{
    impl_->fini();
}

void
WaveformWnd::impl::init()
{
    Core::MiniSplitter * splitter = new Core::MiniSplitter;
    do {
        splitter->addWidget( tpw_.get() );
        splitter->addWidget( spw_.get() );
        splitter->addWidget( hpw_.get() );
        //splitter->setStretchFactor( 0, 1 );
        //splitter->setStretchFactor( 1, 3 );
        splitter->setOrientation( Qt::Vertical );
    } while(0);

    tpw_->setMinimumHeight( 80 );
    spw_->setMinimumHeight( 80 );
    hpw_->setMinimumHeight( 80 );
    spw_->axisWidget( QwtPlot::yLeft )->scaleDraw()->setMinimumExtent( 80 );
    hpw_->axisWidget( QwtPlot::yLeft )->scaleDraw()->setMinimumExtent( 80 );

    hpw_->hide();

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
    
    spw_->link( hpw_.get() );
    
    tpw_->setAxisTitle( QwtPlot::yLeft, tr( "<i>mV</i>" ) );
    tpw_->setAxisTitle( QwtPlot::yRight, tr( "<i>mV</i>" ) );
    tpw_->enableAxis( QwtPlot::yRight, true );

    QBoxLayout * layout = new QVBoxLayout( this_ );
    layout->setMargin( 0 );
    layout->setSpacing( 2 );
    layout->addWidget( splitter );

#if 0
    const QColor colors[] = { QColor( 0x00, 0x00, 0xff, 0x80 ), QColor( 0xff, 0x00, 0x00, 0x80 ) };
    for ( size_t i = 0; i < threshold_markers_.size(); ++i ) {
        threshold_markers_[ i ] = new QwtPlotMarker();
        threshold_markers_[ i ]->setLineStyle( QwtPlotMarker::HLine );
        threshold_markers_[ i ]->setLinePen( colors[ i ], 0, Qt::DotLine );
        threshold_markers_[ i ]->setYAxis( ( i == 1 ) ? QwtPlot::yRight : QwtPlot::yLeft );
        threshold_markers_[ i ]->setYValue( ( i + 1 ) * 100 );
        threshold_markers_[ i ]->attach( spw_ );
    }
#endif

}

void
WaveformWnd::onInitialUpdate()
{
    impl_->spw_->setKeepZoomed( false );
    impl_->hpw_->setKeepZoomed( false );
}

void
WaveformWnd::dataChanged( const boost::uuids::uuid& uuid, int idx )
{
}

void
WaveformWnd::setData( const boost::uuids::uuid& objid, std::shared_ptr< const adcontrols::MassSpectrum > sp, int idx, bool axisRight )
{
    impl_->sp_[ objid ] = sp;

    double elapsed_time = sp->getMSProperty().timeSinceInjection();

    QString title = QString( "Elapsed time: %1 min; " ).arg( QString::number(elapsed_time / 60.0, 'f', 3 ) );
    auto& descs = sp->getDescriptions();
    for ( auto& d: descs )
        title += QString::fromStdWString( d.text() ) + "; ";

    impl_->spw_->setTitle( title );
    impl_->spw_->setData( sp, idx, axisRight );
}

void
WaveformWnd::setData( const boost::uuids::uuid& objid, const adcontrols::TraceAccessor& ta, int fcn )
{
    if ( ! impl_->traces_[ std::make_pair( objid, fcn ) ] ) {
        impl_->traces_[ std::make_pair( objid, fcn ) ] = std::make_shared< adcontrols::Trace >();
    }
    adcontrols::Trace& trace = *impl_->traces_[ std::make_pair( objid, fcn ) ];
    ta >> trace;

    impl_->tpw_->setData( trace, fcn );
}
