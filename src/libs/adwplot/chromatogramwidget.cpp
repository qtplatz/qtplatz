// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

#include "chromatogramwidget.hpp"
#include "annotation.hpp"
#include "annotations.hpp"
//#include "trace.h"
//#include "traces.h"
#include "zoomer.hpp"
#include "peak.hpp"
#include "baseline.hpp"
#include "plotcurve.hpp"
#include "plotpicker.hpp"
#include "plotpanner.hpp"
#include "seriesdata.hpp"
#include <adcontrols/trace.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/peaks.hpp>
#include <adcontrols/peak.hpp>
#include <adcontrols/baselines.hpp>
#include <adcontrols/baseline.hpp>
#include <adcontrols/descriptions.hpp>
#include <adcontrols/description.hpp>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/smart_ptr.hpp>
#include <queue>

using namespace adwplot;

namespace adwplot { namespace chromatogram_internal {

    class SeriesData : public QwtSeriesData<QPointF> {
    public:
        virtual ~SeriesData() {
        }
        SeriesData( const QVector< QPointF >& v, const QRectF& rc ) : v_( v ), rect_(rc) {
        }
        SeriesData( const SeriesData& t ) : v_( t.v_ ) {
        }
        // implements QwtSeriesData<>
        virtual size_t size() const { return v_.size(); }
        virtual QPointF sample( size_t idx ) const { return v_[ idx ]; }
        virtual QRectF boundingRect() const { return rect_; }
        void boundingRect( const QRectF& rc ) { rect_ = rc; }
    private:
        QRectF rect_;
        const QVector< QPointF >& v_;
    };

    struct SeriesDataImpl {
        QVector< QPointF > d_;
        SeriesData * series_;  // for real time trace
        SeriesDataImpl() : series_(0) {}
    };

    class TraceData {
    public:
        TraceData( Dataplot& plot ) : curve_( plot ) {
        }
        TraceData( const TraceData& t ) : data_( t.data_ ), curve_( t.curve_ ) {
        }
        void setData( const adcontrols::Chromatogram& );
        void setData( const adcontrols::Trace& );
    private:
        PlotCurve curve_;
        SeriesDataImpl data_;
    };
  

}
}

ChromatogramWidget::ChromatogramWidget(QWidget *parent) :
    Dataplot(parent)
{
    setAxisTitle(QwtPlot::xBottom, "Time[min]");
    setAxisTitle(QwtPlot::yLeft, "Intensity[uV]");
}

void
ChromatogramWidget::setData( const adcontrols::Trace& d, int idx, bool yaxis2 )
{
    if ( d.size() < 2 )
        return;

    using chromatogram_internal::TraceData;

    while ( int( traces_.size() ) <= idx )
        traces_.push_back( TraceData( *this ) );  // create QwtPlotCurve & Data

    TraceData& trace = traces_[ idx ];
    trace.setData( d );

    replot();
}

void
ChromatogramWidget::setData( const adcontrols::Chromatogram& c )
{
    annotations_.clear();
    peaks_.clear();
    baselines_.clear();
    traces_.clear();

    using chromatogram_internal::TraceData;

    traces_.push_back( TraceData( *this ) );
    TraceData& trace = traces_.back();

    trace.setData( c );

    const double * intens = c.getIntensityArray();

#if 0
    const adcontrols::Baselines& baselines = c.baselines();
    for ( adcontrols::Baselines::vector_type::const_iterator it = baselines.begin(); it != baselines.end(); ++it )
        setBaseline( *it );
#endif
    const adcontrols::Peaks& peaks = c.peaks();

    for ( adcontrols::Peaks::vector_type::const_iterator it = peaks.begin(); it != peaks.end(); ++it )
        setPeak( *it );

    std::pair< double, double > time_range = adcontrols::timeutil::toMinutes( c.timeRange() );
    setAxisScale( QwtPlot::xBottom, time_range.first, time_range.second );
    setAxisScale( QwtPlot::yLeft, intens[ c.min_element() ], intens[ c.max_element() ] );
    zoomer1_->setZoomBase();
    // replot();
}
    

void
ChromatogramWidget::setPeak( const adcontrols::Peak& peak )
{
    double tR = adcontrols::timeutil::toMinutes( peak.peakTime() );

    std::wstring label = peak.name();
    if ( label.empty() )
        label = ( boost::wformat( L"%.3lf" ) % tR ).str();

    Annotations annots( *this, annotations_ );

    Annotation anno = annots.add( tR, peak.peakHeight(), label );
    anno.setLabelAlighment( Qt::AlignTop | Qt::AlignCenter );

    peaks_.push_back( adwplot::Peak( *this, peak ) );
}

void
ChromatogramWidget::setBaseline( const adcontrols::Baseline& bs )
{
    baselines_.push_back( adwplot::Baseline( *this, bs ) );
}

using namespace adwplot::chromatogram_internal;

void
TraceData::setData( const adcontrols::Chromatogram& c )
{
    const double * intens = c.getIntensityArray();
    const double * times = c.getTimeArray();
    const size_t size = c.size();

    data_.d_.resize( size );

    if ( times ) {
        for ( size_t i = 0; i < size; ++i )
            data_.d_[i] = QPointF( adcontrols::Chromatogram::toMinutes( times[i] ), intens[i] );
    } else {
        for ( size_t i = 0; i < size; ++i )
            data_.d_[i] = QPointF( adcontrols::Chromatogram::toMinutes( c.timeFromDataIndex( i ) ), intens[i] );
    }

    QRectF rect;
    std::pair< double, double > time_range = adcontrols::timeutil::toMinutes( c.timeRange() );

    rect.setCoords( time_range.first, intens[ c.min_element() ], time_range.second, intens[ c.max_element() ] );

    curve_.p()->setData( new SeriesData( data_.d_, rect ) );

}

void
TraceData::setData( const adcontrols::Trace& trace )
{
    if ( trace.size() <= 2 )
        return;

    const double *x = trace.getTimeArray();
    const double *y = trace.getIntensityArray();
    size_t current_size = data_.d_.size();

    for ( size_t i = current_size; i < trace.size(); ++i )
        data_.d_.push_back( QPointF( adcontrols::timeutil::toMinutes( x[i] ), y[i] ) );

    QRectF rect;
    rect.setCoords( data_.d_[0].x(), trace.range_y().first, data_.d_[ data_.d_.size() - 1 ].x(), trace.range_y().second );
    if ( ! data_.series_ ) {
        data_.series_ = new SeriesData( data_.d_, rect );
        curve_.p()->setData( data_.series_ );
    }
    data_.series_->boundingRect( rect );
}