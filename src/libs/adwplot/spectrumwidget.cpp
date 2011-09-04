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
    namespace spectrumwidget {

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

	struct SeriesDataImpl {
        std::vector< double > x_;
        std::vector< double > y_;
        typedef std::vector<double> vector_type;

	    void setData( size_t size, const double * x, const double * y ) {
            x_.resize( size );
            y_.resize( size );
            double * xd = &x_[0];
            double * yd = &y_[0];
            for ( size_t i = 0; i < size; ++i ) {
               *xd++ = *x++;
               *yd++ = *y++;
            }
        }
        void push_back( double x, double y ) {
            x_.push_back( x );
            y_.push_back( y );
        }
        size_t index( double x ) const   {  return std::distance( x_.begin(), std::lower_bound( x_.begin(), x_.end(), x ) ); }
        double maximum_value( size_t left, size_t right ) const { return *std::max_element( y_.begin() + left, y_.begin() + right ); }
        double minimum_value( size_t left, size_t right ) const { return *std::min_element( y_.begin() + left, y_.begin() + right ); }
	};
		
	class SeriesData : public QwtSeriesData<QPointF> {
	public:
	    virtual ~SeriesData() {
	    }

        SeriesData( const SeriesDataImpl& impl, const QRectF& rc ) : rect_( rc ), impl_( impl ) {
        }

	    // implements QwtSeriesData<>
        virtual size_t size() const                { return impl_.x_.size(); }
	    virtual QPointF sample( size_t idx ) const { return QPointF( impl_.x_[ idx ], impl_.y_[ idx ] ); }
	    virtual QRectF boundingRect() const        { return rect_; }
	private:
	    QRectF rect_;
        const SeriesDataImpl& impl_;
	};
	

	class TraceData {
	public:
        TraceData() : ms_( 0 ) {}
        TraceData( const TraceData& t ) : curves_( t.curves_ ), dataMap_( t.dataMap_ ), ms_( t.ms_ ) {
	    }
	    void setData( Dataplot& plot, const adcontrols::MassSpectrum& ms );
        std::pair<double, double> y_range( double left, double right ) const;

	    typedef std::map< int, SeriesDataImpl > map_type;
	private:
	    std::vector< PlotCurve > curves_;
	    map_type dataMap_;
        const adcontrols::MassSpectrum * ms_;
	};
    } // namespace spectrumwidget

    struct SpectrumWidgetImpl {
        std::vector< Annotation > annotations_;
        std::vector< spectrumwidget::TraceData > traces_;
    };

} // namespace adwplot

SpectrumWidget::~SpectrumWidget()
{
    delete impl_;
}

SpectrumWidget::SpectrumWidget(QWidget *parent) : Dataplot(parent)
                                                , impl_( new SpectrumWidgetImpl )
                                                , autoYZoom_( true ) 
{
    setAxisTitle(QwtPlot::xBottom, "m/z");
    setAxisTitle(QwtPlot::yLeft, "Intensity[uV]");
    
    // picker_->setRubberBand( QwtPicker::CrossRubberBand );
    //zoomer1_->setRubberBandPen( QColor(Qt::green) );
    //zoomer1_->setRubberBand( QwtPicker::CrossRubberBand );

    // handle zoom rect by this
    if ( zoomer1_ )
        connect( zoomer1_.get(), SIGNAL( zoom_override( QRectF& ) ), this, SLOT( override_zoom_rect( QRectF& ) ) );

    zoomer2_.reset(); //  new Zoomer( QwtPlot::xTop, QwtPlot::yRight, canvas() ) );
}

void
SpectrumWidget::override_zoom_rect( QRectF& rc )
{
    if ( autoYZoom_ ) {
        using spectrumwidget::TraceData;
        BOOST_FOREACH( const TraceData& trace, impl_->traces_ ) {
            std::pair<double, double> y = trace.y_range( rc.left(), rc.right() );
            rc.setBottom( y.first );
            rc.setTop( y.second );
        }
    }
}

void
SpectrumWidget::zoom( const QRectF& rect )
{
    /*
    QRectF rc;
    rc.setLeft( rect.left() );
    rc.setRight( rect.right() );
    if ( autoYZoom_ ) {
        using spectrumwidget::TraceData;
        BOOST_FOREACH( const TraceData& trace, impl_->traces_ ) {
            std::pair<double, double> y = trace.y_range( rect.left(), rect.right() );
            rc.setBottom( y.first );
            rc.setTop( y.second );
        }
    }
    */
    zoomer1_->zoom( rect );
    // static_cast< QwtPlotZoomer *>( zoomer1_.get() )->zoom( rc );
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
    using spectrumwidget::TraceData;

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

using namespace adwplot::spectrumwidget;

void
TraceData::setData( Dataplot& plot, const adcontrols::MassSpectrum& ms )
{
    curves_.clear();
 
    ms_ = &ms;
    const double * intens = ms.getIntensityArray();
    const double * masses = ms.getMassArray();
    const size_t size = ms.size();

    QRectF rect;
    rect.setCoords( ms.getAcquisitionMassRange().first, ms.getMinIntensity(), ms.getAcquisitionMassRange().second, ms.getMaxIntensity() );

    if ( ms.isCentroid() ) {
        const unsigned char * colors = ms.getColorArray();
        if ( colors ) {
            for ( size_t i = 0; i < size; ++i )
                dataMap_[ colors[i] ].push_back( masses[i], intens[i] );
        } else {
            dataMap_[ 0 ].setData( size, masses, intens );
        }
        BOOST_FOREACH( const map_type::value_type& pair, dataMap_ ) {
            curves_.push_back( PlotCurve( plot ) );
            PlotCurve& curve = curves_.back();
            if ( pair.first != 0 && pair.first < sizeof( color_table ) / sizeof( color_table[0] ) )
                curve.p()->setPen( QPen( color_table[ pair.first ] ) );
            curve.p()->setData( new SeriesData( pair.second, rect ) );
            curve.p()->setStyle( QwtPlotCurve::Sticks );
        }

    } else {
        curves_.push_back( PlotCurve( plot ) );
        PlotCurve &curve = curves_[0];
        dataMap_[ 0 ].setData( size, masses, intens );
        curve.p()->setData( new SeriesData( dataMap_[ 0 ], rect ) );
    }
}

std::pair< double, double >
TraceData::y_range( double left, double right ) const
{
    double top = 100; 
    double bottom = 0;
    BOOST_FOREACH( const map_type::value_type& pair, dataMap_ ) {

        size_t idx0 = pair.second.index( left );
        size_t idx1 = pair.second.index( right );
        if ( idx0 < idx1 ) {
            double min = pair.second.minimum_value( idx0, idx1 );
            double max = pair.second.maximum_value( idx0, idx1 );
            if ( min < bottom )
               bottom = min;
            if ( max > top )
                top = max;
        }
    }
    return std::make_pair<>(bottom, top);
}