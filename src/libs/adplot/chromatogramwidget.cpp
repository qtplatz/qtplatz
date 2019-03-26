// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC
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
#include "annotations.hpp"
#include "zoomer.hpp"
#include "picker.hpp"
#include "peak.hpp"
#include "baseline.hpp"
#include "plotcurve.hpp"
#include "seriesdata.hpp"
#include <qwt_plot_picker.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_marker.h>
#include <qwt_picker_machine.h>
#include <qwt_plot_legenditem.h>
#include <qwt_legend.h>
#include <qwt_symbol.h>
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
#include <adcontrols/annotations.hpp>
#include <adportable/debug.hpp>
#include <adportable/float.hpp>
#include <qtwrapper/font.hpp>
#include <boost/format.hpp>
#include <boost/variant.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/optional.hpp>
#include <QDebug>
#include <queue>
#include <memory>

namespace adplot {

    namespace chromatogram_widget {

        static QColor color_table [] = {
            QColor( 0x00, 0x00, 0xff )    // 0  blue
            , QColor( 0xff, 0x00, 0x00 )  // 1  red
            , QColor( 0x00, 0x80, 0x00 )  // 2  green
            , QColor( 0x4b, 0x00, 0x82 )  // 3  indigo
            , QColor( 0xff, 0x14, 0x93 )  // 4  deep pink
            , QColor( 0x94, 0x00, 0xd3 )  // 5  dark violet
            , QColor( 0x80, 0x00, 0x80 )  // 6  purple
            , QColor( 0xdc, 0x13, 0x4c )  // 7  crimson
            , QColor( 0x69, 0x69, 0x69 )  // 8  dim gray
            , QColor( 0x80, 0x80, 0x80 )  // 9  gray
            , QColor( 0xa9, 0xa9, 0xa9 )  //10  dark gray
            , QColor( 0xc0, 0xc0, 0xc0 )  //11  silver
            , QColor( 0xd3, 0xd3, 0xd3 )  //12  light gray
            , QColor( 0xd2, 0x69, 0x1e )  //13  chocolate
            , QColor( 0x00, 0x00, 0x8b )  //14  dark blue
            , QColor( 0xff, 0xff, 0xff )  //15  white
            , QColor( 0xff, 0x8c, 0x00 )  //16  dark orange
            , QColor( 0x00, 0x00, 0x00, 0x00 )  //17
        };

        // for ChromatogramData
        class xSeriesData : public QwtSeriesData< QPointF > {
            xSeriesData( const xSeriesData& ) = delete;
            xSeriesData& operator = ( const xSeriesData ) = delete;
            std::weak_ptr< const adcontrols::Chromatogram > cptr_;
            QRectF rect_;
        public:
            virtual ~xSeriesData() {}
            xSeriesData( std::shared_ptr< const adcontrols::Chromatogram >& chro, const QRectF& rc )
                : cptr_( chro )
                , rect_( rc ) {
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
                        const double * times = ptr->getTimeArray();
                        const double * intens = ptr->getIntensityArray();
                        return QPointF( times[ idx ], intens[ idx ] );
                    }
                }
                return QPointF();
            }

            QRectF boundingRect() const override { return rect_; }
        };

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

        class ChromatogramData {
        public:
            ~ChromatogramData() { }
			ChromatogramData( plot& plot ) : curve_( plot ), y2_(false) { }
            ChromatogramData( const ChromatogramData& t ) : curve_( t.curve_ ), rect_( t.rect_ ), grab_( t.grab_ ), y2_( t.y2_ ) { }

            inline bool y2() const { return y2_; }

            void setData( std::shared_ptr< const adcontrols::Chromatogram>& cp, bool y2 ) {
                grab_ = cp;
                auto range_x = cp->timeRange(); // adcontrols::Chromatogram::toMinutes( cp->timeRange() );
                auto range_y = std::pair<double, double>( cp->getMinIntensity(), cp->getMaxIntensity() );

                // // workaround for 'counting chromatogram', which can be complete flat signals
                if ( std::abs( range_y.first - range_y.second ) <= std::numeric_limits<double>::epsilon() )
                     range_y.first = 0;

                rect_.setCoords( range_x.first, range_y.second, range_x.second, range_y.first );
                if ( y2 )
                    curve_.p()->setYAxis( QwtPlot::yRight );
                curve_.p()->setData( new xSeriesData( cp, rect_ ) );
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

        private:
            PlotCurve curve_;
            QRectF rect_;
            std::shared_ptr< const adcontrols::Chromatogram > grab_;
            bool y2_;
        };

        // typedef boost::variant< ChromatogramData, TraceData<adcontrols::Trace> > trace_variant;
        typedef boost::variant< std::unique_ptr< ChromatogramData >
                                , std::unique_ptr< TraceData<adcontrols::Trace> > > trace_variant;

        struct boundingRect_visitor : public boost::static_visitor< QRectF > {
            template<typename T> QRectF operator()( const T& t ) const {
                return t ? t->boundingRect() : QRectF();
            }
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
            template< typename T > bool operator()( T& t ) const {
                return bool( t ) && typeid(T) == typeid(trace_type);
            };
        };

        struct isNull : public boost::static_visitor< bool > {
            template< typename T > bool operator()( T& t ) const {
                return !( t );
            };
        };

        struct GetPlotItem : public boost::static_visitor< QwtPlotItem * > {
            template< typename T > QwtPlotItem * operator()( T& t ) const {
                return t ? &(t->plot_curve()) : nullptr;
            };
        };

        struct RemoveData : public boost::static_visitor< void > {
            template< typename T > void operator()( T& t ) const {
                t = nullptr;
            };
        };

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

    }

    class ChromatogramWidget::impl {
        impl( const impl& ) = delete;
        impl& operator = ( const impl& ) = delete;
    public:
        impl() : axis_( HorizontalAxisSeconds ) {}

        adcontrols::annotations peak_annotations_;
        std::vector< Annotation > annotation_markers_;

        std::vector< chromatogram_widget::trace_variant > traces_;
        std::vector< Peak > peaks_;
        std::vector< Baseline > baselines_;
        std::function< bool( const QPointF&, QwtText& ) > tracker_hook_;
        std::vector< std::shared_ptr< QwtPlotCurve > > peak_params_curves_; // peak parameter curves
        ChromatogramWidget::HorizontalAxis axis_;
        std::unique_ptr< QwtPlotLegendItem > legendItem_;
        std::unique_ptr< QwtLegend > externalLegend_;

        void clear();
        void removeData( int );
        void update_annotations( const std::pair<double, double>&, adcontrols::annotations& ) const;
		void clear_annotations();
        void update_markers( const std::pair<double, double>& ) const;

        QwtText tracker1( const QPointF& );
        QwtText tracker2( const QPointF&, const QPointF& );

        void redraw();
    };
}

using namespace adplot;
using namespace adplot::chromatogram_widget;

ChromatogramWidget::~ChromatogramWidget()
{
    delete impl_;
}

ChromatogramWidget::ChromatogramWidget(QWidget *parent) : plot(parent)
                                                        , impl_( new impl() )
{
    setAxisTitle(QwtPlot::xBottom, QwtText( "Time[seconds]", QwtText::RichText ) );
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
    if ( idx >= 0 && idx < sizeof( color_table )/sizeof( color_table[0] ) )
        return QColor( color_table[ idx ] );
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
        // if ( impl_->traces_[ idx ].which() == 0 )
        //     boost::get< std::unique_ptr< ChromatogramData > >( impl_->traces_[ idx ] ) = 0;
        // impl_->traces_[ idx ] = std::move( std::unique_ptr< ChromatogramData >( 0 ) ); //ChromatogramData( *this );
    }
    if ( bReplot )
        replot();
}
void
ChromatogramWidget::setAxis( HorizontalAxis axis, bool replot )
{
    impl_->axis_ = axis;
    if ( replot )
        impl_->redraw();
}

void
ChromatogramWidget::setData( std::shared_ptr< const adcontrols::Trace> c, int idx, bool yRight )
{
    if ( c->size() < 2 )
        return;

    if ( impl_->traces_.size() <= size_t( idx ) )
        impl_->traces_.resize( idx + 1 );

    if ( ! boost::apply_visitor( isValid< TraceData< adcontrols::Trace > >(), impl_->traces_[ idx ] ) )
        impl_->traces_[ idx ] = std::make_unique< TraceData< adcontrols::Trace > >( *this );

    if ( auto& trace = boost::get< std::unique_ptr< TraceData< adcontrols::Trace > > >( impl_->traces_[ idx ] ) ) {

        trace->plot_curve().setPen( QPen( color_table [ idx ] ) );
        trace->plot_curve().setYAxis( yRight ? QwtPlot::yRight : QwtPlot::yLeft );
        trace->setData( c );

        trace->plot_curve().setTitle( QString::fromStdString( c->legend() ) );

        auto yAxis = yRight ? QwtPlot::yRight : QwtPlot::yLeft;

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

void
ChromatogramWidget::setData( std::shared_ptr< const adcontrols::Chromatogram >&& cp, int idx, bool yRight )
{
    std::shared_ptr< const adcontrols::Chromatogram > xcp = std::move( cp );
    setData( xcp, idx, yRight );
}

void
ChromatogramWidget::setData( std::shared_ptr< const adcontrols::Chromatogram >& cp, int idx, bool yRight )
{
    if ( cp->size() < 2 )
        return;

    using adcontrols::Chromatogram;

    if ( impl_->traces_.size() <= size_t( idx ) )
        impl_->traces_.resize( idx + 1 );

    if ( ! boost::apply_visitor( isValid< ChromatogramData >(), impl_->traces_[ idx ] ) )
        impl_->traces_[ idx ] = std::make_unique< ChromatogramData >( *this );

    auto& trace = boost::get< std::unique_ptr< ChromatogramData > >( impl_->traces_ [ idx ] );

	trace->plot_curve().setPen( QPen( color_table[idx] ) );
    trace->setData( cp, yRight );
    impl_->peak_annotations_.clear();

    for ( auto& pk: cp->peaks() )
        setPeak( pk, impl_->peak_annotations_ );

    impl_->peak_annotations_.sort();

    plotAnnotations( impl_->peak_annotations_ );

    if ( auto value = cp->ptree().get_optional<std::string>( "trace.legend" ) )
        trace->plot_curve().setTitle( QString::fromStdString( value.get() ) );

    QRectF rect = trace->boundingRect();
    for ( const auto& v: impl_->traces_ ) {
        if ( boost::apply_visitor( isValid< std::unique_ptr< ChromatogramData > >(), v ) ) {
            auto& trace = boost::get< std::unique_ptr< ChromatogramData > >( v );
            if ( trace->y2() == yRight ) {
                rect |= trace->boundingRect();
            }
        }
    }

    setAxisScale( QwtPlot::xBottom, rect.left(), rect.right() );
    setAxisScale( yRight ? QwtPlot::yRight : QwtPlot::yLeft
                  , rect.top() - rect.height() * 0.05, rect.bottom() + rect.height() * 0.05 );

    zoomer()->setZoomBase(); // zoom base set to data range

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
ChromatogramWidget::setData( const adcontrols::PeakResult& r )
{
	using adcontrols::Peaks;
	using adcontrols::Baselines;

    impl_->peaks_.clear();
    impl_->baselines_.clear();
    impl_->peak_params_curves_.clear();

	for ( Baselines::vector_type::const_iterator it = r.baselines().begin(); it != r.baselines().end(); ++it )
		setBaseline( *it );

    impl_->peak_annotations_.clear();
	for ( Peaks::vector_type::const_iterator it = r.peaks().begin(); it != r.peaks().end(); ++it )
		setPeak( *it, impl_->peak_annotations_ );

    plotAnnotations( impl_->peak_annotations_ );
}

void
ChromatogramWidget::setPeak( const adcontrols::Peak& peak, adcontrols::annotations& vec )
{
    //double tR = adcontrols::timeutil::toMinutes( peak.peakTime() );
    double tR = peak.peakTime();

    int pri = 0;
    std::string label = peak.name();
    if ( label.empty() )
        label = ( boost::format( "%.4lf" ) % tR ).str();

    pri = label.empty() ? int( peak.topHeight() ) : int( peak.topHeight() ) + 0x3fffffff;

    adcontrols::annotation annot( label, tR, peak.topHeight(), pri );
    vec << annot;

    impl_->peaks_.emplace_back( *this, peak );
}

void
ChromatogramWidget::setBaseline( const adcontrols::Baseline& bs )
{
    impl_->baselines_.emplace_back( *this, bs );
}

void
ChromatogramWidget::plotAnnotations( const adcontrols::annotations& vec )
{
    impl_->clear_annotations();

    adplot::Annotations w( *this, impl_->annotation_markers_ );

    for ( auto& a: vec ) {
		QwtText text( QString::fromStdString( a.text() ), QwtText::RichText );
        text.setColor( Qt::darkGreen );
        text.setFont( Annotation::font() );
        w.insert( a.x(), a.y(), text, Qt::AlignTop | Qt::AlignHCenter );
    }
}

void
ChromatogramWidget::drawPeakParameter( const adcontrols::Peak& pk )
{
    auto tr = pk.retentionTime();

    if ( tr.algorithm() == adcontrols::RetentionTime::ParaboraFitting ) {

        auto curve = std::make_shared< QwtPlotCurve >();
        curve->setItemAttribute( QwtPlotItem::Legend, false );

        QPolygonF points;

        double a, b, c;
        tr.eq( a, b, c );
        enum { width = 10 };
        for ( int i = 0; i <= width; ++i ) {
            double x = tr.boundary( 0 ) + ( ( tr.boundary( 1 ) - tr.boundary( 0 ) ) * i ) / width;
            double y = a + ( b * x ) + ( c * x * x );
            //points << QPointF( adcontrols::Chromatogram::toMinutes( x ), y );
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

    adcontrols::annotations vec;
    impl_->update_annotations( range, vec );
    vec.sort();
    plotAnnotations( vec );
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
    peaks_.clear();
    baselines_.clear();
	annotation_markers_.clear();
    traces_.clear();
}

void
ChromatogramWidget::impl::removeData( int idx )
{
    if ( traces_.size() > size_t(idx) ) {
        peaks_.clear();
        baselines_.clear();
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

void
ChromatogramWidget::impl::update_annotations( const std::pair<double, double>& range
                                            , adcontrols::annotations& vec ) const
{
	auto& peaks = peak_annotations_;
    auto beg = std::lower_bound( peaks.begin(), peaks.end(), range.first, [=]( const adcontrols::annotation& a, double rhs ){
            return a.x() < rhs;
        });
    auto end = std::lower_bound( peaks.begin(), peaks.end(), range.second, [=]( const adcontrols::annotation& a, double rhs ){
            return a.x() < rhs;
        });

    std::for_each( beg, end, [&]( const adcontrols::annotation& a ){
            vec << a;
        });

    vec.sort();
}

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

        impl_->legendItem_->setAlignment( Qt::AlignTop | Qt::AlignRight );

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
        //connect( this, SIGNAL( legendDataChanged( const QVariant &, const QList<QwtLegendData> & ) ),
        //         impl_->externalLegend_, SLOT( updateLegend( const QVariant &, const QList<QwtLegendData> & ) ) );
    } else {
        impl_->externalLegend_.reset();
    }
}

bool
ChromatogramWidget::legendEnabled() const
{
    return impl_->externalLegend_ != nullptr;
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
