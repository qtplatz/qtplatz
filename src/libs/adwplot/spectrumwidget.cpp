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

#include "spectrumwidget.hpp"
#include "zoomer.hpp"
#include "plotcurve.hpp"
#include "plotpicker.hpp"
#include "plotpanner.hpp"
#include "annotation.hpp"
#include <adcontrols/massspectrum.hpp>
#include <boost/foreach.hpp>
#include <qwt_plot_curve.h>

using namespace adwplot;

namespace adwplot {
    namespace internal {

	static Qt::GlobalColor color_table[] = {
	    Qt::blue,
	    Qt::red,
	    Qt::green,
	    Qt::cyan,
	    Qt::magenta,
	    Qt::yellow,
	    Qt::darkRed,
	    Qt::darkGreen,
	    Qt::darkBlue,
	    Qt::darkCyan,
	    Qt::darkMagenta,
	    Qt::darkYellow,
	    Qt::darkGray,
	    Qt::gray,
	    Qt::lightGray,
	};
	
	class SeriesData : public QwtSeriesData<QPointF> {
	public:
	    virtual ~SeriesData() {
	    }
	    SeriesData( const QVector< QPointF >& v, const QRectF& rc ) : rect_( rc ), v_( v ) {
	    }
	    SeriesData( const SeriesData& t ) : v_( t.v_ ) {
	    }
	    // implements QwtSeriesData<>
	    virtual size_t size() const { return v_.size(); }
	    virtual QPointF sample( size_t idx ) const { return v_[ idx ]; }
	    virtual QRectF boundingRect() const { return rect_; }
	private:
	    QRectF rect_;
	    const QVector< QPointF >& v_;
	};
	
	struct SeriesDataImpl {
	    QVector< QPointF > d_;
	    void setData( size_t size, const double * x, const double * y ) {
		d_.resize( size );
		for ( size_t i = 0; i < size; ++i )
		    d_[ i ] = QPointF( x[i], y[i] );
	    }
	};
	
	class TraceData {
	public:
	    TraceData() {}
	    TraceData( const TraceData& t ) : curves_( t.curves_ ), dataMap_( t.dataMap_ ) {
	    }
	    void setData( Dataplot& plot, const adcontrols::MassSpectrum& ms );
	    typedef std::map< int, SeriesDataImpl > map_type;
	private:
	    std::vector< PlotCurve > curves_;
	    map_type dataMap_;
	};
    } // namespace internal

    struct SpectrumWidgetImpl {
	std::vector< internal::TraceData > traces_;
    };

} // namespace adwplot

SpectrumWidget::~SpectrumWidget()
{
    delete impl_;
}

SpectrumWidget::SpectrumWidget(QWidget *parent) : Dataplot(parent)
						, impl_( new SpectrumWidgetImpl )
{
    setAxisTitle(QwtPlot::xBottom, "m/z");
    setAxisTitle(QwtPlot::yLeft, "Intensity[uV]");
    
    // picker_->setRubberBand( QwtPicker::CrossRubberBand );
    // zoomer1_->setRubberBandPen( QColor(Qt::green) );
    zoomer1_->setRubberBand( QwtPicker::CrossRubberBand );
    zoomer1_->autoYScale( true );

    zoomer2_.reset(); //  new Zoomer( QwtPlot::xTop, QwtPlot::yRight, canvas() ) );
}

void
SpectrumWidget::zoom( const QRectF& rect )
{
    QRectF rc = zoomer1_->zoomRect();
    rc.setLeft( rect.left() );
    rc.setRight( rect.right() );
    zoomer1_->zoom( rc );
}

void
SpectrumWidget::setData( const adcontrols::MassSpectrum& ms )
{
    setData( ms, 0, false );
}

void
SpectrumWidget::setData( const adcontrols::MassSpectrum& ms1, const adcontrols::MassSpectrum& ms2 )
{
    setData( ms1, 0, false );
    setData( ms2, 1, true );
}

void
SpectrumWidget::setData( const adcontrols::MassSpectrum& ms, int idx, bool yaxis2 )
{
    using internal::TraceData;

    while ( int( impl_->traces_.size() ) <= idx ) 
        impl_->traces_.push_back( TraceData() );

    TraceData& trace = impl_->traces_[ idx ];
    trace.setData( *this, ms );

    setAxisScale( QwtPlot::xBottom, ms.getAcquisitionMassRange().first, ms.getAcquisitionMassRange().second );
    setAxisScale( yaxis2 ? QwtPlot::yRight : QwtPlot::yLeft, ms.getMinIntensity(), ms.getMaxIntensity() );

    zoomer1_->setZoomBase();
    // todo: annotations

    // replot();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

using namespace adwplot::internal;

void
TraceData::setData( Dataplot& plot, const adcontrols::MassSpectrum& ms )
{
    curves_.clear();
 
    const double * intens = ms.getIntensityArray();
    const double * masses = ms.getMassArray();
    const size_t size = ms.size();

    QRectF rect;
    rect.setCoords( ms.getAcquisitionMassRange().first, ms.getMinIntensity(), ms.getAcquisitionMassRange().second, ms.getMaxIntensity() );

    if ( ms.isCentroid() ) {
        const unsigned char * colors = ms.getColorArray();
        if ( colors ) {
            for ( size_t i = 0; i < size; ++i )
                dataMap_[ colors[i] ].d_.push_back( QPointF( masses[i], intens[i] ) );
        } else {
            dataMap_[ 0 ].setData( size, masses, intens );
        }
        BOOST_FOREACH( const map_type::value_type& pair, dataMap_ ) {
            curves_.push_back( PlotCurve( plot ) );
            PlotCurve& curve = curves_.back();
            if ( pair.first != 0 && pair.first < sizeof( color_table ) / sizeof( color_table[0] ) )
                curve.p()->setPen( QPen( color_table[ pair.first ] ) );
            curve.p()->setData( new SeriesData( pair.second.d_, rect ) );
            curve.p()->setStyle( QwtPlotCurve::Sticks );
        }

    } else {
        curves_.push_back( PlotCurve( plot ) );
        PlotCurve &curve = curves_[0];
        dataMap_[ 0 ].setData( size, masses, intens );
        curve.p()->setData( new SeriesData( dataMap_[ 0 ].d_, rect ) );
    }
}
