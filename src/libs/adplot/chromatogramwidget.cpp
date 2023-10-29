// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2021 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2021 MS-Cheminformatics LLC
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

#include "chromatogramwidget.hpp"
#include "annotation.hpp"
#include "boundingrect.hpp"
#include "color_table.hpp"
#include "zoomer.hpp"
#include "picker.hpp"
#include "peak.hpp"
#include "baseline.hpp"
#include "plotcurve.hpp"
#include <QtCore/qnamespace.h>
#include <qwt_plot.h>
#include <qwt_plot_picker.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_legenditem.h>
#include <qwt_picker_machine.h>
#include <qwt_plot_legenditem.h>
#include <qwt_legend.h>
#include <qwt_symbol.h>
#include <qwt_text.h>
#include <adcontrols/trace.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/peakresult.hpp>
#include <adcontrols/peaks.hpp>
#include <adcontrols/peak.hpp>
#include <adcontrols/retentiontime.hpp>
#include <adcontrols/baselines.hpp>
#include <adcontrols/baseline.hpp>
#include <adcontrols/descriptions.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/annotation.hpp>
#include <adportable/debug.hpp>
#include <adportable/float.hpp>
#include <qtwrapper/font.hpp>
#include <boost/format.hpp>
#include <boost/variant.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/optional.hpp>
#include <array>
#include <memory>
#include <queue>
#include <set>

namespace adplot {
    namespace ns_c     {  class ChromatogramData;                 }
    namespace ns_trace {  template<typename T> class TraceData;   }
}

namespace {

    template< typename T >
    std::string __name( T* obj ) { return " " + obj->objectName().toStdString() + ":"; };

    std::tuple< double, double, double > rc_tuple( const QRectF& rc ) {
        return std::make_tuple( rc.top(), rc.bottom(), rc.height() );
    }

    //---------------------------------------

    class zoomer : public QwtPlotZoomer {
        zoomer( QWidget * canvas ) : QwtPlotZoomer( canvas ) {
            QPen pen( QColor( 0xff, 0, 0, 0x80 ) ); // transparent darkRed
            setRubberBandPen( pen );
            setRubberBand(QwtPlotPicker::CrossRubberBand);
            setTrackerMode( AlwaysOn );
        }
        QSizeF minZoomSize() const override {
            QRectF rc = zoomBase();
            return QSizeF( rc.width() * 1.0e-9, rc.height() * 1.0e-9 );
        }
        QwtText trackerTextF( const QPointF &pos ) const override {
            QColor bg( Qt::white );
            bg.setAlpha( 128 );
            QwtText text;
            text = QwtText( (boost::format("<i>m/z</i> %.4f @ %.4fs") % pos.y() % pos.x() ).str().c_str(), QwtText::RichText );
            text.setBackgroundBrush( QBrush( bg ) );
            return text;
        }

    };

    struct clipping {
        std::vector< adcontrols::annotation >
        operator()( const QRectF& rect, const std::vector< adcontrols::annotation >& list ) const {
            std::vector< adcontrols::annotation > clipped;
            std::for_each( list.begin(), list.end(), [&]( const auto& a ){
                if ( rect.contains( QPointF(a.x(), a.y()) ) )
                    clipped.emplace_back( a );
            });
            std::sort( clipped.begin(), clipped.end(), [](const auto& a, const auto& b){ return a.priority() > b.priority(); } );
            return clipped;
        }
    };

}

namespace {

    typedef boost::variant<
        std::unique_ptr< adplot::ns_c::ChromatogramData >
        , std::unique_ptr< adplot::ns_trace::TraceData<adcontrols::Trace> >
        > trace_variant;

    //-------------
    struct transformY : public boost::static_visitor< double > {
        double y_;
        transformY(double y) : y_( y ) {};
        template<typename T> double operator()( const T& t ) const {
            return t ? t->transformY( y_ ) : y_;
        }
    };

    struct boundingRect_visitor : public boost::static_visitor< QRectF > {
        template<typename T> QRectF operator()( const T& t ) const {  return t ? t->boundingRect() : QRectF{};  }
    };

    struct yAxis_visitor : public boost::static_visitor< QwtPlot::Axis > {
        template< typename T > QwtPlot::Axis operator ()( const T& t ) const { return t ? QwtPlot::Axis( t->plot_curve().yAxis() ) : QwtPlot::yLeft; }
    };

    struct updateMarkers_visitor : public boost::static_visitor< void > {
        QwtPlot * plot_;
        std::pair< double, double > range_;
        updateMarkers_visitor( QwtPlot * plot, const std::pair< double, double >& range ) : plot_( plot ), range_( range ) {}
        template<typename T> void operator()( T& t ) const { if ( t ) t->drawMarkers( plot_, range_ ); }
    };

    template< typename trace_type >
        struct isValid : public boost::static_visitor< bool > {
        template< typename T > bool operator()( T& t ) const {  return bool( t ) && typeid(T) == typeid(trace_type);  };
    };

    struct isNull : public boost::static_visitor< bool > {
        template< typename T > bool operator()( T& t ) const {  return !( t );  };
    };

    struct GetPlotItem : public boost::static_visitor< QwtPlotItem * > {
        template< typename T > QwtPlotItem * operator()( T& t ) const { return t ? &(t->plot_curve()) : nullptr;  };
    };

    struct RemoveData : public boost::static_visitor< void > {
        template< typename T > void operator()( T& t ) const {  t = nullptr;  };
    };

} // namespaece


namespace adplot {
    namespace ns_c {
        //---------------------------------------------------------
        //---------------------------------------------------------
        // for ChromatogramData
        class xSeriesData : public QwtSeriesData< QPointF > {
            xSeriesData( const xSeriesData& ) = delete;
            xSeriesData& operator = ( const xSeriesData ) = delete;
            std::weak_ptr< const adcontrols::Chromatogram > cptr_;
            QRectF rect_;
            bool yNormalize_;
        public:
            virtual ~xSeriesData() {}
            xSeriesData( std::shared_ptr< const adcontrols::Chromatogram >& chro, const QRectF& rc, bool yNormalize )
                : cptr_( chro )
                , rect_( rc )
                , yNormalize_( yNormalize ) {
            }

            size_t size() const override {
                if ( auto ptr = cptr_.lock() )
                    return ptr->size();
                return 0;
            }

            QPointF sample( size_t idx ) const override {
                if ( auto ptr = cptr_.lock() ) {
                    if ( ptr->size() > idx ) {
                        using adcontrols::Chromatogram;
                        if ( yNormalize_ ) {
                            double yMax = ptr->getMaxIntensity() - ptr->getMinIntensity();
                            double y = 100. * ( ptr->intensity( idx ) - ptr->getMinIntensity() ) / yMax;
                            return QPointF( ptr->time( idx ), y );
                        } else {
                            return QPointF( ptr->time( idx ), ptr->intensity( idx ) );
                        }
                    }
                }
                return {};
            }

            QRectF boundingRect() const override { return rect_; }
        };
    }

    namespace ns_trace {
        //---------------------------------------------------------
        //---------------------------------------------------------
        // for TraceData
        template< typename T >
        class tSeriesData : public QwtSeriesData< QPointF > {

            tSeriesData( const tSeriesData& ) = delete;
            tSeriesData& operator = ( const tSeriesData& ) = delete;
            std::shared_ptr<const T> t_;

        public:
            tSeriesData() { }
			tSeriesData( std::shared_ptr< const T> t ) : t_( t ) { }
			~tSeriesData() { }

            // implements QwtSeriesData<>
            size_t size() const override { return t_->size(); }

            QPointF sample( size_t idx ) const override {
                return QPointF( t_->x(idx) - t_->injectTime(), (t_->y( idx ) - t_->yOffset()) );
            }

            virtual QRectF boundingRect() const override {
                return rect_;
            }
            void setBoundingRect( const QRectF& rc ) {
                rect_ = rc;
            }
        private:
			QRectF rect_;
        };

        //---------------------------------------------------------
        //---------------------------------------------------------
        template<class T> class TraceData {
        public:
            ~TraceData() { }
			TraceData( plot& plot ) : curve_( plot ) { }
            TraceData( const TraceData& t ) : curve_( t.curve_ ), rect_( t.rect_ ) { }
            void setData( std::shared_ptr<const T> ) {}
			const QRectF& boundingRect() const { return rect_; };
			QwtPlotCurve& plot_curve() { return *curve_.p(); }
			const QwtPlotCurve& plot_curve() const { return *curve_.p(); }
            void drawMarkers( QwtPlot *, const std::pair< double, double >& ) {}
            void setAlpha( int ) {};
            void setColor( const QColor& ) {};
            double transformY( double y ) const { return y; }
        private:
            PlotCurve curve_;
			QRectF rect_;
        };

        template<> void TraceData<adcontrols::Trace>::setData( std::shared_ptr< const adcontrols::Trace > trace )
        {
            if ( trace->size() > 2 ) {
                auto * d_trace = new tSeriesData< adcontrols::Trace >( trace );

                double x0 = trace->x( 0 ) - trace->injectTime();
                double x1 = trace->x( trace->size() - 1 ) - trace->injectTime();
                auto range_y = trace->range_y();
                rect_ = QRectF( QPointF( x0, range_y.first ), QPointF( x1, range_y.second ) );

                d_trace->setBoundingRect( rect_ );

                curve_.p()->setData( d_trace );
            }
        }
    } // ns_trace

    namespace ns_c {

        class ChromatogramData {
        public:
            ~ChromatogramData() { }
			ChromatogramData( plot& plot ) : curve_( plot )
                                           , yAxis_( QwtPlot::yLeft )
                                           , yNormalize_( false ) {}
            ChromatogramData( const ChromatogramData& t ) : curve_( t.curve_ ), rect_( t.rect_ ), grab_( t.grab_ ), yAxis_( t.yAxis_ ) { }

            inline bool y2() const { return yAxis_ == QwtPlot::yRight; }
            inline QwtPlot::Axis yAxis() const { return yAxis_; }

            void setNormalizedY( bool normalize ) {
                yNormalize_ = normalize;
            };

            void setData( std::shared_ptr< const adcontrols::Chromatogram>& cp, QwtPlot::Axis yAxis ) {
                grab_ = cp;
                yAxis_ = yAxis;
                auto range_x = cp->timeRange();
                auto range_y = std::pair<double, double>( cp->getMinIntensity(), cp->getMaxIntensity() );

                // // workaround for 'counting chromatogram', which can be complete flat signals
                if ( std::abs( range_y.first - range_y.second ) <= std::numeric_limits<double>::epsilon() )
                    range_y.first = 0;
                if ( yNormalize_ )
                    rect_.setCoords( range_x.first, 105.0, range_x.second, -5.0 );
                else
                    rect_.setCoords( range_x.first, range_y.second, range_x.second, range_y.first ); // left,top, right,bottom

                curve_.p()->setYAxis( yAxis_ );
                curve_.p()->setData( new xSeriesData( cp, rect_, yNormalize_ ) );
            }

            double transformY( double y ) const {
                if ( yNormalize_ )
                    return 100. * ( y - grab_->getMinIntensity() ) / ( grab_->getMaxIntensity() - grab_->getMinIntensity() );
                return y;
            }

			const QRectF& boundingRect() const { return rect_; };
			QwtPlotCurve& plot_curve() { return *curve_.p(); }
			const QwtPlotCurve& plot_curve() const { return *curve_.p(); }

            void drawMarkers( QwtPlot * plot, const std::pair< double, double >& range ) {
                if ( grab_ ) {
                    const double * times = grab_->getTimeArray();
                    auto beg = std::lower_bound( times, times + grab_->size(), range.first );
                    auto end = std::lower_bound( beg, times + grab_->size(), range.second );
                    size_t d = std::distance( beg, end );
                    if ( d < 80 ) {
                        QPen pen( Qt::red );
                        curve_.p()->setSymbol( new QwtSymbol( QwtSymbol::Style( QwtSymbol::XCross ), Qt::NoBrush, pen, QSize( 5, 5 ) ) );
                        plot->replot();
                    }
                    else {
                        curve_.p()->setSymbol( 0 );
                    }
                }
            }

            void setAlpha( int alpha ) {
                QColor color( curve_.p()->pen().color() );
                color.setAlpha( alpha );
                curve_.p()->setPen( QPen( color ) );
            }

            void setColor( const QColor& color ) {
                curve_.p()->setPen( QPen( color ) );
            }

            std::shared_ptr< const adcontrols::Chromatogram > get() const { return grab_; }

        private:
            PlotCurve curve_;
            QRectF rect_;
            std::shared_ptr< const adcontrols::Chromatogram > grab_;
            QwtPlot::Axis yAxis_;
            bool yNormalize_;
        };

    }  // chromatogram
} // adplot

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

namespace adplot {

    class ChromatogramWidget::impl {
        impl( const impl& ) = delete;
        impl& operator = ( const impl& ) = delete;
    public:
        impl() : axis_( HorizontalAxisSeconds )
               , normalizedY_{ false }
               , yScale_{ true, 0, 0 }
               , xScale_{ true, 0, 0 }
            {}

        std::vector< adcontrols::annotation > peak_annotations_;
        std::vector< Annotation > annotation_markers_;

        std::vector< /* anonymous */ trace_variant > traces_;
        std::vector< adplot::Peak > plot_peaks_;
        std::vector< adplot::Baseline > plot_baselines_;
        std::function< bool( const QPointF&, QwtText& ) > tracker_hook_;
        std::vector< std::shared_ptr< QwtPlotCurve > > peak_params_curves_; // peak parameter curves
        ChromatogramWidget::HorizontalAxis axis_;
        std::unique_ptr< QwtPlotLegendItem > legendItem_;
        std::unique_ptr< QwtLegend > externalLegend_;
        std::array< bool, QwtPlot::axisCnt > normalizedY_;

        std::tuple< bool, double, double > yScale_;
        std::tuple< bool, double, double > xScale_;

        void clear();
        void removeData( int );
		void clear_annotations();
        void update_markers( const std::pair<double, double>& ) const;

        QwtText tracker1( const QPointF& );
        QwtText tracker2( const QPointF&, const QPointF& );

        void redraw();

        QRectF unitedRect( QwtPlot::Axis yAxis ) const {
            QRectF rect = {};
            for ( const auto& v: traces_ ) {
                if ( boost::apply_visitor( isValid< std::unique_ptr< ns_c::ChromatogramData > >(), v ) ) {
                    auto rc = boost::get< std::unique_ptr< ns_c::ChromatogramData > >( v )->boundingRect();
                    auto leftTop = QPointF( std::min( rc.left(), rect.left() ), std::max( rc.top(), rect.top() ) );
                    auto rightBottom = QPointF( std::max( rc.right(), rect.right() ), std::min( rc.bottom(), rect.bottom() ) );
                    rect = QRectF( leftTop, rightBottom );
                }
            }
            return rect;
        }
        void setPeak( adplot::plot&, const adcontrols::Peak& peak );
        void setAnnotation( adplot::plot&, const std::vector< adcontrols::annotation >& vec );
        void setAnnotation( adplot::plot&, std::vector< adcontrols::annotation >&& );
    };
}

using namespace adplot;
// using namespace adplot::chromatogram;

ChromatogramWidget::~ChromatogramWidget()
{
    delete impl_;
}

ChromatogramWidget::ChromatogramWidget(QWidget *parent) : plot(parent)
                                                        , impl_( new impl() )
{
    setAxisTitle(QwtPlot::xBottom, QwtText( "Time (s)", QwtText::RichText ) );
    setAxisTitle(QwtPlot::yLeft, QwtText( "Intensity" ) );

    // -----------
    auto font = qtwrapper::font()( QFont(), qtwrapper::fontSizeSmall, qtwrapper::fontAxisLabel );
    setAxisFont( QwtPlot::xBottom, font );
    setAxisFont( QwtPlot::yLeft, font );

    if ( auto zoomer = plot::zoomer() ) {
        zoomer->tracker1( std::bind( &impl::tracker1, impl_, std::placeholders::_1 ) );
        zoomer->tracker2( std::bind( &impl::tracker2, impl_, std::placeholders::_1, std::placeholders::_2 ) );

        connect( zoomer, &Zoomer::zoomed, this, &ChromatogramWidget::zoomed );
	}

    if ( auto picker = plot::picker() ) {
        // picker_->setStateMachine( new QwtPickerClickPointMachine() );
		connect( picker, SIGNAL( moved( const QPointF& ) ), this, SLOT( moved( const QPointF& ) ) );
		connect( picker, SIGNAL( selected( const QPointF& ) ), this, SLOT( selected( const QPointF& ) ) );
		connect( picker, SIGNAL( selected( const QRectF& ) ), this, SLOT( selected( const QRectF& ) ) );

        picker->setEnabled( true );
	}
}

QColor
ChromatogramWidget::color( int idx ) const
{
    using constants::chromatogram::color_table;

    if ( idx >= 0 ) // && idx < sizeof( color_table )/sizeof( color_table[0] ) )
        return QColor( color_table[ idx % sizeof(color_table)/sizeof(color_table[0]) ] );
    return QColor( Qt::black );
}

void
ChromatogramWidget::register_tracker( std::function< bool( const QPointF&, QwtText& ) > f )
{
    impl_->tracker_hook_ = f;
}

void
ChromatogramWidget::clear()
{
    impl_->clear();
    replot();
}

void
ChromatogramWidget::removeData( int idx, bool bReplot )
{
    if ( impl_->traces_.size() > size_t(idx) ) {
        boost::apply_visitor( RemoveData(), impl_->traces_[ idx ] );
    }
    if ( bReplot )
        replot();
}
void
ChromatogramWidget::setAxis( HorizontalAxis axis, bool replot ) // minutes|seconds
{
    impl_->axis_ = axis;
    if ( replot )
        impl_->redraw();
}

void
ChromatogramWidget::setNormalizedY( QwtPlot::Axis axis, bool normalized )
{
    if ( QwtPlot::isAxisValid( axis ) )
        impl_->normalizedY_[ axis ] = normalized;
}


void
ChromatogramWidget::setTrace( std::shared_ptr< const adcontrols::Trace> c, int idx, QwtPlot::Axis yAxis )
{
    using constants::chromatogram::color_table;

    if ( !c || c->size() < 2 )
        return;

    if ( impl_->traces_.size() <= size_t( idx ) )
        impl_->traces_.resize( idx + 1 );

    if ( ! boost::apply_visitor( isValid< ns_trace::TraceData< adcontrols::Trace > >(), impl_->traces_[ idx ] ) )
        impl_->traces_[ idx ] = std::make_unique< ns_trace::TraceData< adcontrols::Trace > >( *this );

    if ( auto& trace = boost::get< std::unique_ptr< ns_trace::TraceData< adcontrols::Trace > > >( impl_->traces_[ idx ] ) ) {

        trace->plot_curve().setPen( QPen( color_table [ idx ] ) );
        trace->plot_curve().setYAxis( yAxis );
        trace->setData( c );

        trace->plot_curve().setTitle( QString::fromStdString( c->legend() ) );

        QRectF rc;
        for ( auto& trace: impl_->traces_ ) {
            if ( ( ! boost::apply_visitor( isNull(), trace ) ) && ( boost::apply_visitor( yAxis_visitor(), trace ) == yAxis ) ) {
                QRectF rect( boost::apply_visitor( boundingRect_visitor(), trace ) );
                if ( rc.isEmpty() )
                    rc = rect;
                else
                    rc |= rect;
            }
        }

        for ( auto& trace: impl_->traces_ ) {
            if ( ! boost::apply_visitor( isNull(), trace ) && ( boost::apply_visitor( yAxis_visitor(), trace ) != yAxis ) ) {
                QRectF rect( boost::apply_visitor( boundingRect_visitor(), trace ) );
                rc = QRectF( QPointF( std::min( rc.left(), rect.left() ), rc.top() )
                             , QPointF( std::max( rc.right(), rect.right() ), rc.bottom() ) );
            }
        }

        if ( adportable::compare<double>::essentiallyEqual( rc.height(), 0.0 ) )
            rc.setHeight( 1.0 );

        setAxisScale( QwtPlot::xBottom, rc.left(), rc.right() + rc.width() / 20.0 );

        setAxisScale( yAxis, rc.top(), rc.bottom() ); // flipped y-scale

        zoomer()->setZoomBase();
    }
}

std::shared_ptr< const adcontrols::Chromatogram >
ChromatogramWidget::getData( int idx ) const
{
    using ns_c::ChromatogramData;

    if ( impl_->traces_.size() > size_t( idx ) ) {
        if ( ! boost::apply_visitor( isValid< ChromatogramData >(), impl_->traces_[ idx ] ) ) {
            auto& data = boost::get< std::unique_ptr< ChromatogramData > >( impl_->traces_[ idx ] );
            return data->get();
        }
    }
    return {};
}

void
ChromatogramWidget::setData( std::shared_ptr< const adcontrols::Chromatogram > cp, int idx, QwtPlot::Axis yAxis )
{
    using constants::chromatogram::color_table;

    if ( cp->size() < 2 )
        return;

    using ns_c::ChromatogramData;

    if ( impl_->traces_.size() <= size_t( idx ) )
        impl_->traces_.resize( idx + 1 );

    if ( ! boost::apply_visitor( isValid< ChromatogramData >(), impl_->traces_[ idx ] ) )
        impl_->traces_[ idx ] = std::make_unique< ChromatogramData >( *this );

    auto& trace = boost::get< std::unique_ptr< ChromatogramData > >( impl_->traces_ [ idx ] );
    if ( QwtPlot::isAxisValid( yAxis ) ) {
        trace->setNormalizedY( impl_->normalizedY_[ yAxis ] );
    }

	trace->plot_curve().setPen( QPen( color_table[idx] ) );
    trace->setData( cp, yAxis );
    // impl_->peak_annotations_.clear();

    QRectF rect = trace->boundingRect();
    for ( const auto& v: impl_->traces_ ) {
        if ( boost::apply_visitor( isValid< std::unique_ptr< ChromatogramData > >(), v ) ) {
            auto& trace = boost::get< std::unique_ptr< ChromatogramData > >( v );
            if ( trace->yAxis() == yAxis ) {
                rect |= trace->boundingRect();
            }
        }
    }
    // ADDEBUG() << "rect.height: " << std::make_tuple( rect.top(), rect.bottom(), rect.height() );

    if ( std::get< 0 >( impl_->xScale_ ) ) { // x auto scale
        setAxisScale( QwtPlot::xBottom, rect.left(), rect.right() );
    } else {
        setAxisScale( QwtPlot::xBottom, std::get< 1 >( impl_->xScale_ ), std::get< 2 >( impl_->xScale_ ) );
    }

    if ( std::get< 0 >( impl_->yScale_ ) ) { // y is not auto scale
        setAxisScale( yAxis, rect.top() - rect.height() * 0.05, rect.bottom() + rect.height() * 0.05 );
    } else {
        setAxisScale( yAxis, std::get< 1 >( impl_->yScale_ ), std::get< 2 >( impl_->yScale_ ) );
    }
    zoomer()->setZoomBase();
}

void
ChromatogramWidget::setChromatogram( std::tuple< int
                                     , std::shared_ptr< const adcontrols::Chromatogram >
                                     , std::shared_ptr< const adcontrols::PeakResult > > data
                                     , QwtPlot::Axis yAxis )
{
    auto [idx, chr, pkres] = data;

    setData( chr, idx, yAxis );

    auto& vec = impl_->peak_annotations_;
    vec.erase( std::remove_if( vec.begin(), vec.end(), [idx=idx](const auto& a){ return a.index() < 0 || a.index() == idx; } ), vec.end() );

    if ( pkres ) {
        for ( const auto& pk: pkres->peaks() ) {
            if ( !pk.name().empty() )
                vec.emplace_back( pk.name(), pk.peakTime(), pk.topHeight(), idx, int( pk.topHeight() * 100 ) );
        }
    }
    auto dup( vec );
    std::sort( dup.begin(), dup.end(), [](const auto& a, const auto& b){ return a.priority() > b.priority(); } );
    impl_->setAnnotation( *this, dup );
}

void
ChromatogramWidget::setAlpha( int idx, int alpha )
{
    using ns_c::ChromatogramData;
    if ( impl_->traces_.size() > idx ) {
        if ( const auto& p = boost::get< std::unique_ptr< ChromatogramData > >( impl_->traces_.at( idx ) ) )
            p->setAlpha( alpha );
    }
}

void
ChromatogramWidget::setColor( int idx, const QColor& color )
{
    using ns_c::ChromatogramData;
    if ( impl_->traces_.size() > idx ) {
        if ( const auto& p = boost::get< std::unique_ptr< ChromatogramData > >( impl_->traces_.at( idx ) ) )
            p->setColor( color );
    }
}

void
ChromatogramWidget::setZoomed( const QRectF& rect, bool keepY )
{
    auto rc( rect );

    if ( keepY ) {
        QRectF z = zoomer()->zoomRect(); // current (:= previous) zoom
        rc.setBottom( z.bottom() );
        rc.setTop( z.top() );
    }
    plot::zoom( rc );
}

void
ChromatogramWidget::setPeakResult( const adcontrols::PeakResult& r, int idx, QwtPlot::Axis axis )
{
    impl_->plot_peaks_.clear();
    impl_->plot_baselines_.clear();
    impl_->peak_params_curves_.clear();

    for ( const auto& bs: r.baselines() )
		impl_->plot_baselines_.emplace_back( *this, bs );

    impl_->peak_annotations_.clear();

    for ( const auto& pk:  r.peaks() ) {
        // peak marker
		impl_->setPeak( *this, pk );

        // annotation
		impl_->peak_annotations_.emplace_back( pk.name().empty() ? (boost::format( "%.2f" ) % pk.peakTime()).str() : pk.name()
                                               , pk.peakTime()
                                               , pk.topHeight()
                                               , idx
                                               , pk.topHeight() * 100 );
    }

    auto dup( impl_->peak_annotations_ );
    std::sort( dup.begin(), dup.end(), [](const auto& a, const auto& b){ return a.priority() > b.priority(); } );
    impl_->setAnnotation( *this, std::move( dup ) );
}

void
ChromatogramWidget::impl::setPeak( adplot::plot& plot, const adcontrols::Peak& peak )
{
    double tR = peak.peakTime();
    plot_peaks_.emplace_back( plot, peak );
}

void
ChromatogramWidget::impl::setAnnotation( adplot::plot& plot, const std::vector< adcontrols::annotation >& vec )
{
    auto dup( vec );
    setAnnotation( plot, std::move( dup ) );
}

void
ChromatogramWidget::impl::setAnnotation( adplot::plot& plot, std::vector< adcontrols::annotation >&& vec )
{
    using constants::chromatogram::color_table;

    annotation_markers_.clear();
    std::vector< QRectF > rectVec;
    std::set< int > indices;

    for ( auto& a: vec ) {
		QwtText text( QString::fromStdString( a.text() ), QwtText::RichText );
        text.setColor( a.index() < 0 ? Qt::darkGreen : color_table[ a.index() ] );
        text.setFont( Annotation::font() );
        double y = a.y();
        if ( normalizedY_[ QwtPlot::yLeft ] &&
             ( a.index() >= 0 && a.index() < traces_.size() ) ) {
            y = boost::apply_visitor( transformY{ a.y() }, traces_[ a.index() ] );
        }

        auto rc = boundingRect()( plot, QPointF{ a.x(), y }, text, Qt::AlignTop | Qt::AlignCenter );
        auto it = std::find_if( rectVec.begin(), rectVec.end(), [&](const auto& rect){ return rc.intersects( rect ); });
        if ( it == rectVec.end() || indices.find( a.index() ) == indices.end() ) {
            indices.emplace( a.index() );
            rectVec.emplace_back( rc );
            annotation_markers_.emplace_back( plot, text, QPointF{ a.x(), y }, QwtPlot::yLeft, Qt::AlignTop | Qt::AlignHCenter );
        }
    }
}


void
ChromatogramWidget::drawPeakParameter( const adcontrols::Peak& pk )
{
    const auto& tr = pk.retentionTime();

    if ( tr.algorithm() == adcontrols::RetentionTime::ParaboraFitting ) {

        auto curve = std::make_shared< QwtPlotCurve >();
        curve->setItemAttribute( QwtPlotItem::Legend, false );

        QPolygonF points;

        double a, b, c;
        tr.eq( a, b, c );
        constexpr const double width = 10;
        for ( int i = 0; i <= width; ++i ) {
            double x = tr.boundary(0) + ( ( tr.boundary(1) - tr.boundary(0) ) * i ) / width;
            double y = a + ( b * x ) + ( c * x * x );
            points << QPointF( x, y );
            curve->setSamples( points );
        }
        curve->setPen( QPen( QColor( 240, 0, 0, 0x80 ) ) );
        curve->attach( this );
        impl_->peak_params_curves_.emplace_back( curve );
    }

}

void
ChromatogramWidget::zoomed( const QRectF& rect )
{
    auto range = std::make_pair( rect.left(), rect.right() );
    updateMarkers_visitor drawMarker( this, range );
    for ( auto& trace : impl_->traces_ )
        boost::apply_visitor( drawMarker, trace );

    auto vec = clipping()( rect, impl_->peak_annotations_ );
    impl_->setAnnotation( *this, vec );
}

void
ChromatogramWidget::moved( const QPointF& pos )
{
	emit onMoved( pos );
}

void
ChromatogramWidget::selected( const QPointF& pos )
{
	emit onSelected( pos );
}

void
ChromatogramWidget::selected( const QRectF& rect )
{
	emit onSelected( rect );
}

QwtPlotItem *
ChromatogramWidget::getPlotItem( int idx )
{
    if ( idx < impl_->traces_.size() )
        return boost::apply_visitor( GetPlotItem(), impl_->traces_[ idx ] );
    return nullptr;
}

//--------------
void
ChromatogramWidget::impl::clear()
{
    plot_peaks_.clear();
    plot_baselines_.clear();
	annotation_markers_.clear();
    traces_.clear();
    peak_annotations_.clear();
}

void
ChromatogramWidget::impl::removeData( int idx )
{
    if ( traces_.size() > size_t(idx) ) {
        plot_peaks_.clear();
        plot_baselines_.clear();
        annotation_markers_.clear();
    }
}

void
ChromatogramWidget::impl::clear_annotations()
{
	annotation_markers_.clear();
}

void
ChromatogramWidget::impl::update_markers( const std::pair<double, double>& ) const
{
}

// std::vector< adcontrols::annotation >
// ChromatogramWidget::impl::clipping( const QRectF& rect
//                                     , const std::vector< adcontrols::annotation >& annots ) const
// {
//     std::vector< adcontrols::annotation > vec;

//     std::for_each( annots.begin(), annots.end(), [&]( const auto& a ){
//         if ( rect.contains( QPointF(a.x(), a.y()) ) )
//             vec.emplace_back( a );
//     });
//     std::sort( vec.begin(), vec.end(), [](const auto& a, const auto& b){ return a.priority() > b.priority(); } );
//     return vec;
// }

QwtText
ChromatogramWidget::impl::tracker1( const QPointF& pos )
{
    QwtText text = QwtText( (boost::format("%.3fs, %.3f") % pos.x() % pos.y() ).str().c_str(), QwtText::RichText );

    if ( tracker_hook_ && tracker_hook_( pos, text ) )
        return text;

    return text;
}

QwtText
ChromatogramWidget::impl::tracker2( const QPointF& p1, const QPointF& pos )
{
    QwtText text;

    double dx = ( pos.x() - p1.x() );
    double dy = ( pos.y() - p1.y() );

    text = QwtText( (boost::format("%.3fs(&delta;=%.3f<i>s</i>,%.3f),%.3f") % pos.x() % dx % dy % pos.y() ).str().c_str(), QwtText::RichText );

    if ( tracker_hook_ && tracker_hook_( pos, text ) )
        return text;

    return text;
}

void
ChromatogramWidget::impl::redraw()
{
}

void
ChromatogramWidget::setItemLegendEnabled( bool enable )
{
    if ( enable ) {

        if ( ! impl_->legendItem_ )
            impl_->legendItem_ = std::make_unique< QwtPlotLegendItem >();

        impl_->legendItem_->setRenderHint( QwtPlotItem::RenderAntialiased );
        QColor color( Qt::white );
        impl_->legendItem_->setTextPen( color );
        impl_->legendItem_->setBorderPen( color );
        QColor bc( Qt::gray );
        bc.setAlpha( 200 );
        impl_->legendItem_->setBackgroundBrush( bc );

        impl_->legendItem_->attach( this );
        impl_->legendItem_->setMaxColumns( 2 );

        QFont font = impl_->legendItem_->font();
        font.setPointSize( 10 );
        impl_->legendItem_->setFont( font );

        impl_->legendItem_->setAlignmentInCanvas( Qt::AlignTop | Qt::AlignRight );

    } else {
        impl_->legendItem_.reset();
    }
}

bool
ChromatogramWidget::itemLegendEnabled() const
{
    return impl_->legendItem_ != nullptr;
}

void
ChromatogramWidget::setLegendEnabled( bool enable )
{
    if ( enable ) {
        if ( ! impl_->externalLegend_ )
            impl_->externalLegend_ = std::make_unique< QwtLegend >();
        impl_->externalLegend_->setWindowTitle("Legend");
    } else {
        impl_->externalLegend_.reset();
    }
}

bool
ChromatogramWidget::legendEnabled() const
{
    return impl_->externalLegend_ != nullptr;
}


void
ChromatogramWidget::setYScale( std::tuple< bool, double, double >&& yScale, bool update )
{
    if ( !std::get< 0 >( yScale ) ) { // not an auto scaled
        std::get< 0 >( yScale ) = adportable::compare< double >::essentiallyEqual( std::get< 1 >( yScale ), std::get< 2 >( yScale ) );
    }
    impl_->yScale_ = std::move( yScale );

    if ( auto zoomer = plot::zoomer() )
        zoomer->autoYScale( std::get< 0 >( impl_->yScale_ ) );

    if ( std::get< 0 >( impl_->yScale_ ) ) { // auto scale
        auto rect = impl_->unitedRect( QwtPlot::yLeft );
        if ( rect != QRectF{} ) {
            double margin = rect.height() * 0.05;  // should be negative height
            setAxisScale( QwtPlot::yLeft, rect.bottom() + margin, rect.top() - margin ); // min, max
        }
    } else {
        setAxisScale( QwtPlot::yLeft, std::get< 1 >( impl_->yScale_ ), std::get< 2 >( impl_->yScale_ ) );
    }
    if ( update )
        replot();
}

void
ChromatogramWidget::setXScale( std::tuple< bool, double, double >&& xScale, bool update )
{
    if ( !std::get< 0 >( xScale ) ) { // not an auto scaled
        std::get< 0 >( xScale ) = adportable::compare< double >::essentiallyEqual( std::get< 1 >( xScale ), std::get< 2 >( xScale ) );
    }
    impl_->xScale_ = std::move( xScale );
    if ( !std::get< 0 >( impl_->xScale_ ) ) { // not auto scale
        setAxisScale( QwtPlot::xBottom, std::get< 1 >( impl_->xScale_ ), std::get< 2 >( impl_->xScale_ ) );
    } else {
        auto rect = impl_->unitedRect( QwtPlot::yLeft );
        if ( rect != QRectF{} ) {
            setAxisScale( QwtPlot::xBottom, rect.left(), rect.right() );
        }
    }
    if ( update )
        replot();
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
