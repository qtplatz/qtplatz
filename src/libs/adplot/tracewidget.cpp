// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include "annotation.hpp"
#include "annotations.hpp"
#include "picker.hpp"
#include "plotcurve.hpp"
#include "tracewidget.hpp"
#include "zoomer.hpp"
#include <adportable/array_wrapper.hpp>
#include <qtwrapper/font.hpp>
#include <qwt_plot_picker.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_picker_machine.h>
#include <qwt_text.h>
#include <QPen>
#include <boost/format.hpp>
#include <algorithm>
#include <functional>

using namespace adplot;

namespace adplot {

    namespace tracewidget {

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
            SeriesDataImpl() {}

            std::vector< double > x_;
            std::vector< double > y_;

            typedef std::vector<double> vector_type;

            SeriesDataImpl( const SeriesDataImpl& t )
                : x_( t.x_ )
                , y_( t.y_ ) {
            }

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
            virtual ~SeriesData() {   }
            SeriesData( const SeriesDataImpl& impl, const QRectF& rc ) : rect_( rc ), impl_( impl ) {    }
            SeriesData( const SeriesData& t ) : rect_( t.rect_ ), impl_( t.impl_ ) { }
            // implements QwtSeriesData<>
            virtual size_t size() const                { return impl_.x_.size(); }
            virtual QPointF sample( size_t idx ) const { return QPointF( impl_.x_[ idx ], impl_.y_[ idx ] );  }
            virtual QRectF boundingRect() const        { return rect_; }
        private:
            QRectF rect_;
            SeriesDataImpl impl_;
        };


        class TraceData {
        public:
            TraceData() {  }
            TraceData( const TraceData& t ) : curves_( t.curves_ ), data_( t.data_ ) {   }
            ~TraceData();
            void setData( plot& plot, std::size_t size, const double * x, const double * y, bool y2, int idx );
            std::pair<double, double> y_range( double left, double right ) const;

            typedef std::map< int, SeriesDataImpl > map_type;
        private:
            std::vector< PlotCurve > curves_;
            map_type data_;
        };
    } // namespace spectrumwidget

    struct TraceWidgetImpl {
        // adcontrols::MassSpectrum centroid_;  // for annotation
        std::vector< Annotation > annotations_;
        std::vector< tracewidget::TraceData > traces_;

        void clear();
		void clear_annotations();
        QwtText tracker1( const QPointF& );
        QwtText tracker2( const QPointF&, const QPointF& );
    };

} // namespace adplot

TraceWidget::~TraceWidget()
{
    delete impl_;
}

TraceWidget::TraceWidget(QWidget *parent) : plot(parent)
                                                , impl_( new TraceWidgetImpl )
                                                , autoYZoom_( true )
{

    setAxisTitle(QwtPlot::xBottom, "Time(microsecond)");
    setAxisTitle(QwtPlot::yLeft, "Intensity");

    // handle zoom rect by this
    if ( zoomer() ) {
        zoomer()->autoYScale( autoYZoom_ );
		using namespace std::placeholders;
        zoomer()->tracker1( std::bind( &TraceWidgetImpl::tracker1, impl_, _1 ) );
        zoomer()->tracker2( std::bind( &TraceWidgetImpl::tracker2, impl_, _1, _2 ) );
        //connect( zoomer(), SIGNAL( zoom_override( QRectF& ) ), this, SLOT( override_zoom_rect( QRectF& ) ) );
    }

	if ( picker() ) {
        connect( picker(), SIGNAL( selected( const QRectF& ) ), this, SLOT( selected( const QRectF& ) ) );
        picker()->setEnabled( true );
	}

}

void
TraceWidget::xBottomTitle( const std::string& title )
{
    setAxisTitle( QwtPlot::xBottom, QwtText( title.c_str(), QwtText::RichText ) );
}

void
TraceWidget::yLeftTitle( const std::string& title )
{
    setAxisTitle(QwtPlot::yLeft, QwtText( title.c_str(), QwtText::RichText ) );
}

void
TraceWidget::override_zoom_rect( QRectF& rc )
{
    if ( autoYZoom_ ) {
        using tracewidget::TraceData;
        double bottom = rc.bottom();
        double top = rc.top();
        for ( const TraceData& trace: impl_->traces_ ) {
            std::pair<double, double> y = trace.y_range( rc.left(), rc.right() );
            if ( bottom > y.first )
                bottom = y.first;
            if ( top < y.second )
                top = y.second; // rc.setTop( y.second );
        }
        rc.setBottom( bottom );
        rc.setTop( top + ( top - bottom ) * 0.12 );  // increase 12% for annotation
    }
}

void
TraceWidget::zoom( const QRectF& rect )
{
	plot::zoom( rect );
}

void
TraceWidget::moved( const QPointF& pos )
{
	(void)pos;
	//std::cout << "TraceWidget::moved( " << pos.x() << ", " << pos.y() << ")" << std::endl;
}

void
TraceWidget::selected( const QPointF& pos )
{
	//std::cout << "TraceWidget::selected( " << pos.x() << ", " << pos.y() << ")" << std::endl;
	emit onSelected( pos );
}

void
TraceWidget::selected( const QRectF& rect )
{
	emit onSelected( rect );
}

void
TraceWidget::clear()
{
    impl_->clear();
    zoomer()->setZoomBase();
}

void
TraceWidget::setData( std::size_t n, const double * px, const double * py, int idx, bool axisRight )
{
    using tracewidget::TraceData;

    bool addedTrace = impl_->traces_.size() <= size_t( idx );

    while ( int( impl_->traces_.size() ) <= idx )
        impl_->traces_.push_back( TraceData() );

    TraceData& trace = impl_->traces_[ idx ];
    trace.setData( *this, n, px, py, axisRight, idx );

    auto minmax = std::minmax_element( py, py + n );
    double minimum = *minmax.first;
    double maximum = *minmax.second;

    setAxisScale( QwtPlot::xBottom, px[ 0 ], px[ n - 1 ] );
    if ( axisRight )
        setAxisScale( QwtPlot::yRight, minimum, maximum );
    else
        setAxisScale( QwtPlot::yLeft, minimum, maximum );

    QRectF z = zoomer()->zoomRect();

    zoomer()->setZoomBase( true );
    if ( ! addedTrace )
        zoomer()->zoom( z );
    replot();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

using namespace adplot::tracewidget;

TraceData::~TraceData()
{
    curves_.clear();
    data_.clear();
}

void
TraceData::setData( plot& plot, std::size_t size, const double * x, const double * y, bool y2, int idx )
{
    curves_.clear();
    data_.clear();

    QRectF rect;

    curves_.push_back( PlotCurve( plot ) );
    PlotCurve &curve = curves_[0];
    data_[ 0 ].setData( size, x, y );
    curve.p()->setData( new SeriesData( data_[ 0 ], rect ) );
    if ( y2 )
        curve.p()->setYAxis( QwtPlot::yRight );
    if ( idx )
        curve.p()->setPen( QPen( color_table[ idx ] ) );
}

std::pair< double, double >
TraceData::y_range( double left, double right ) const
{
    for ( const map_type::value_type& pair: data_ ) {

        size_t idx0 = pair.second.index( left );
        size_t idx1 = pair.second.index( right );
        if ( idx0 < idx1 ) {
            double min = pair.second.minimum_value( idx0, idx1 );
            double max = pair.second.maximum_value( idx0, idx1 );
			return std::make_pair( min, max );
        }
    }
    return std::make_pair<>( -10, 100);
}

namespace adplot {
    namespace tracewidget {

        struct compare_priority {
            const unsigned char * colors_;
            const double * intensities_;
            compare_priority( const double * intensities, const unsigned char * colors)
                : colors_( colors )
                , intensities_( intensities ) {
            }
            bool operator()( size_t idx0, size_t idx1 ) const {
                if ( colors_ && colors_[ idx0 ] != colors_[ idx1 ] )
                    return colors_[ idx0 ] > colors_[ idx1 ];
                return intensities_[ idx0 ] > intensities_[ idx1 ];
            }
        };

    }
}

void
TraceWidgetImpl::clear_annotations()
{
	annotations_.clear();
}

void
TraceWidgetImpl::clear()
{
    annotations_.clear();
    traces_.clear();
}

QwtText
TraceWidgetImpl::tracker1( const QPointF& pos )
{
    return QwtText( (boost::format("%.4g %.4g") % pos.x() % pos.y()).str().c_str(), QwtText::RichText );
}

QwtText
TraceWidgetImpl::tracker2( const QPointF& p1, const QPointF& pos )
{
    double dx = ( pos.x() - p1.x() );
    double dy = ( pos.y() - p1.y() );
    return QwtText( (boost::format("%.4g %.4g (&delta;=%g,%g)") % pos.x() % pos.y() % dx % dy).str().c_str(), QwtText::RichText );
}
