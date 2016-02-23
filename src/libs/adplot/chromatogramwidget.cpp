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

#include "chromatogramwidget.hpp"
#include "annotation.hpp"
#include "annotations.hpp"
#include "zoomer.hpp"
#include "picker.hpp"
#include "peak.hpp"
#include "baseline.hpp"
#include "plotcurve.hpp"
#include <qwt_plot_picker.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_marker.h>
#include <qwt_picker_machine.h>
#include <qwt_symbol.h>
#include "seriesdata.hpp"
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
#include <qtwrapper/font.hpp>
#include <boost/format.hpp>
#include <boost/variant.hpp>
#include <queue>
#include <memory>

namespace adplot {

    namespace chromatogram_widget {

        static Qt::GlobalColor color_table[] = {
            Qt::blue,
            Qt::red,
            Qt::darkGreen,
            Qt::darkCyan,
            Qt::magenta,
            Qt::yellow,
            Qt::darkRed,
            Qt::green,
            Qt::darkBlue,
            Qt::cyan,
            Qt::darkMagenta,
            Qt::darkYellow,
            Qt::darkGray,
            Qt::gray,
            Qt::lightGray,
        };

        class xSeriesData : public QwtSeriesData< QPointF > {
            xSeriesData( const xSeriesData& ) = delete;
            xSeriesData& operator = ( const xSeriesData ) = delete;
            std::weak_ptr< adcontrols::Chromatogram > cptr_;
            QRectF rect_;
        public:
            virtual ~xSeriesData() {}
            xSeriesData( const std::shared_ptr< adcontrols::Chromatogram >& chro, const QRectF& rc )
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
                        return QPointF( Chromatogram::toMinutes( times[ idx ] ), intens[ idx ] );
                    }
                }
                return QPointF();
            }

            QRectF boundingRect() const override { return rect_; }
        };

        template<class T> class series_data : public QwtSeriesData< QPointF > {

            series_data( const series_data& ) = delete;
            series_data& operator = ( const series_data& ) = delete;
            T t_;

        public:
            series_data() { }
			series_data( const T& t ) : t_( t ) { }
			~series_data() { }
			inline operator T& () {	return t_;	};

            // implements QwtSeriesData<>
            size_t size() const override { return t_.size(); }

            QPointF sample( size_t idx ) const override { 
                return QPointF( adcontrols::Chromatogram::toMinutes( t_.x(idx) ), t_.y(idx) );
            }
            
            virtual QRectF boundingRect() const { return rect_; }
            void boundingRect( const QRectF& rc ) { rect_ = rc; }
        private:
			QRectF rect_;
        };
        
        template<class T> class TraceData {
        public:
            ~TraceData() { }
			TraceData( plot& plot ) : curve_( plot ) { }
			TraceData( const TraceData& t ) : curve_( t.curve_ ), rect_( t.rect_ ) { }
            void setData( const T& ) {  }
            void setData( const std::shared_ptr<T>& ) {}
			const QRectF& boundingRect() const { return rect_; };
			QwtPlotCurve& plot_curve() { return *curve_.p(); }
			const QwtPlotCurve& plot_curve() const { return *curve_.p(); }
            void drawMarkers( QwtPlot *, const std::pair< double, double >& ) {}

        private:
            PlotCurve curve_;
			QRectF rect_;
        };

        template<> void TraceData<adcontrols::Trace>::setData( const adcontrols::Trace& trace )
        {
            if ( trace.size() <= 2 )
                return;

            // TODO:  refactor code in order to avoid full data copy
			series_data< adcontrols::Trace > * d_trace = new series_data< adcontrols::Trace >( trace );
            // make rect upside down due to QRectF 'or' operator flips y-coord for negative height
            rect_ = QRectF( QPointF( d_trace->sample( 0 ).x(), trace.range_y().first ), QPointF(d_trace->sample( trace.size() - 1 ).x(), trace.range_y().second ));
            d_trace->boundingRect( rect_ );
			curve_.p()->setData( d_trace );
        }

        class ChromatogramData {
        public:
            ~ChromatogramData() { }
			ChromatogramData( plot& plot ) : curve_( plot ), y2_(false) { }
            ChromatogramData( const ChromatogramData& t ) : curve_( t.curve_ ), rect_( t.rect_ ), grab_( t.grab_ ), y2_( t.y2_ ) { }

            inline bool y2() const { return y2_; }

            void setData( const std::shared_ptr< adcontrols::Chromatogram>& cp, bool y2 ) {
                grab_ = cp;
                auto range_x = adcontrols::Chromatogram::toMinutes( cp->timeRange() );
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
                    auto beg = std::lower_bound( times, times + grab_->size() - 1, adcontrols::Chromatogram::toSeconds( range.first ) );
                    auto end = std::lower_bound( beg, times + grab_->size() - 1, adcontrols::Chromatogram::toSeconds( range.second ) );
                    if ( std::distance( beg, end ) < 80 ) {
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
            std::shared_ptr< adcontrols::Chromatogram > grab_;
            bool y2_;
        };
        
        typedef boost::variant< ChromatogramData, TraceData<adcontrols::Trace> > trace_variant;

        struct boundingRect_visitor : public boost::static_visitor< QRectF > {
            template<typename T> QRectF operator()( const T& t ) const { return t.boundingRect(); }
        };

        struct yAxis_visitor : public boost::static_visitor< int > {
            template< typename T > int operator ()( const T& t ) const { return t.plot_curve().yAxis(); }
        };

        struct updateMarkers_visitor : public boost::static_visitor< void > {
            QwtPlot * plot_;
            std::pair< double, double > range_;
            updateMarkers_visitor( QwtPlot * plot, const std::pair< double, double >& range ) : plot_( plot ), range_( range ) {}
            template<typename T> void operator()( T& t ) const { t.drawMarkers( plot_, range_ ); }
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
                text = QwtText( (boost::format("<i>m/z</i> %.4f @ %.3fmin") % pos.y() % pos.x() ).str().c_str(), QwtText::RichText );
                text.setBackgroundBrush( QBrush( bg ) );
                return text;
            }
            
        };
        
    }

    class ChromatogramWidget::impl {
        impl( const impl& ) = delete;
        impl& operator = ( const impl& ) = delete;
    public:
        impl() {}
        
        adcontrols::annotations peak_annotations_;
        std::vector< Annotation > annotation_markers_;
        
        std::vector< chromatogram_widget::trace_variant > traces_;
        std::vector< Peak > peaks_;
        std::vector< Baseline > baselines_;	
        std::function< bool( const QPointF&, QwtText& ) > tracker_hook_;
        std::vector< std::shared_ptr< QwtPlotCurve > > curves_;
        
        void clear();
        void removeData( int );
        void update_annotations( const std::pair<double, double>&, adcontrols::annotations& ) const;
		void clear_annotations();
        void update_markers( const std::pair<double, double>& ) const;
        
        QwtText tracker1( const QPointF& );
        QwtText tracker2( const QPointF&, const QPointF& );
        
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
    setAxisTitle(QwtPlot::xBottom, QwtText( "Time[min]", QwtText::RichText ) );
    setAxisTitle(QwtPlot::yLeft, QwtText( "Intensity" ) );
    
    // -----------
    QFont font;
    qtwrapper::font::setFont( font, qtwrapper::fontSizeSmall, qtwrapper::fontAxisLabel );
    setAxisFont( QwtPlot::xBottom, font );
    setAxisFont( QwtPlot::yLeft, font );
    
    if ( auto zoomer = plot::zoomer() ) {
        using namespace std::placeholders;
        zoomer->tracker1( std::bind( &impl::tracker1, impl_, _1 ) );
        zoomer->tracker2( std::bind( &impl::tracker2, impl_, _1, _2 ) );
        
        // connect( zoomer, static_cast<void(Zoomer::*)(QRectF&)>(&Zoomer::zoom_override), this, &ChromatogramWidget::override_zoom_rect );
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
    if ( impl_->traces_.size() > size_t(idx) )
        impl_->traces_[ idx ] = ChromatogramData( *this );
    if ( bReplot )
        replot();
}

void
ChromatogramWidget::setData( const adcontrols::Trace& c, int idx, bool yRight )
{
    if ( c.size() < 2 )
        return;
    
    while ( int( impl_->traces_.size() ) <= idx )
        impl_->traces_.emplace_back( TraceData< adcontrols::Trace >( *this ) );
    
    TraceData< adcontrols::Trace > * trace = 0;
    try {
        trace = &boost::get< TraceData< adcontrols::Trace > >( impl_->traces_[ idx ] );
    } catch ( boost::bad_get& ex ) {
        adportable::debug(__FILE__, __LINE__) << ex.what();
    }
    
    if ( trace ) {

        trace->plot_curve().setPen( QPen( color_table [ idx ] ) );
        trace->plot_curve().setYAxis( yRight ? QwtPlot::yRight : QwtPlot::yLeft );
        trace->setData( c );

        //ADDEBUG() << "setData(" << idx << ", y=" << trace->plot_curve().yAxis() << ")" << "{" << rc.left() << ", " << rc.bottom() << "},{" << rc.right() << ", " << rc.top() << "}";

        auto yAxis = yRight ? QwtPlot::yRight : QwtPlot::yLeft;

        auto rc = std::accumulate( impl_->traces_.begin(), impl_->traces_.end(), QRectF {}, [&] ( const QRectF& a, const trace_variant& b ) {
            QRectF rect( boost::apply_visitor( boundingRect_visitor(), b ) );
            if ( boost::apply_visitor( yAxis_visitor(), b ) == yAxis ) {
                return a | rect; // this flips y-coodinate upside down when negative height
            } else 
                return QRectF( QPointF( std::min( rect.left(), a.left() ), a.top() ), QPointF( std::max( rect.right(), a.right() ), a.bottom() ) );
        } );

        setAxisScale( QwtPlot::xBottom, rc.left(), rc.right() + rc.width() / 20.0 );
        setAxisScale( yAxis, rc.top(), rc.bottom() ); // flipped y-scale 

        zoomer()->setZoomBase();
    }
}

void
ChromatogramWidget::setData( const std::shared_ptr< adcontrols::Chromatogram >& cp, int idx, bool yRight )
{
    if ( cp->size() < 2 )
        return;

    using adcontrols::Chromatogram;

    while ( int ( impl_->traces_.size() ) <= idx )
		impl_->traces_.push_back( ChromatogramData( *this ) );

    auto& trace = boost::get< ChromatogramData >( impl_->traces_ [ idx ] );
    QRectF z = zoomer()->zoomRect(); // current (:= previous) zoom

	trace.plot_curve().setPen( QPen( color_table[idx] ) ); 
    trace.setData( cp, yRight );
    impl_->peak_annotations_.clear();

    for ( auto& pk: cp->peaks() )
        setPeak( pk, impl_->peak_annotations_ );
    
    impl_->peak_annotations_.sort();
    
    plotAnnotations( impl_->peak_annotations_ );
    
    QRectF rect = trace.boundingRect();
    std::pair< double, double > horizontal( std::make_pair( rect.left(), rect.right() ) );
    std::pair< double, double > vertical( std::make_pair( rect.bottom(), rect.top() ) );
    
    for ( const auto& v: impl_->traces_ ) {
        auto& trace = boost::get< ChromatogramData >( v );
        if ( trace.y2() == yRight ) {
            QRectF rc = trace.boundingRect();
            horizontal.first = std::min( horizontal.first, rc.left() );
            horizontal.second = std::max( horizontal.second, rc.right() );
            vertical.first = std::min( vertical.first, rc.bottom() );
            vertical.second = std::max( vertical.second, rc.top() );
        }
    }
    double h = vertical.second - vertical.first;
    vertical.first -= h * 0.05;
    vertical.second += h * 0.10;
    
    setAxisScale( QwtPlot::xBottom, horizontal.first, horizontal.second );
    setAxisScale( yRight ? QwtPlot::yRight : QwtPlot::yLeft, vertical.first, vertical.second );

    zoomer()->setZoomBase(); // zoom base set to data range

    //if keep zoomed
    //if ( z.left() < horizontal.first || horizontal.second < z.right() )
    //    plot::zoom( z ); // push previous rect
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
    impl_->curves_.clear();

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
    double tR = adcontrols::timeutil::toMinutes( peak.peakTime() );
    
    int pri = 0;
    std::wstring label = peak.name();
    if ( label.empty() )
        label = ( boost::wformat( L"%.3lf" ) % tR ).str();

    pri = label.empty() ? int( peak.topHeight() ) : int( peak.topHeight() ) + 0x3fffffff;
    
    adcontrols::annotation annot( label, tR, peak.topHeight(), pri );
    vec << annot;
    
    impl_->peaks_.push_back( adplot::Peak( *this, peak ) );
}

void
ChromatogramWidget::setBaseline( const adcontrols::Baseline& bs )
{
    impl_->baselines_.push_back( adplot::Baseline( *this, bs ) );
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
        QPolygonF points;

        double a, b, c;
        tr.eq( a, b, c );
        enum { width = 10 };
        for ( int i = 0; i <= width; ++i ) {
            double x = tr.boundary( 0 ) + ( ( tr.boundary( 1 ) - tr.boundary( 0 ) ) * i ) / width;
            double y = a + ( b * x ) + ( c * x * x );
            points << QPointF( adcontrols::Chromatogram::toMinutes( x ), y );
            curve->setSamples( points );
        }
        curve->setPen( QPen( QColor( 240, 0, 0, 0x80 ) ) );
        curve->attach( this );
        impl_->curves_.push_back( curve );
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
    QwtText text = QwtText( (boost::format("%.3fmin, %.1f") % pos.x() % pos.y() ).str().c_str(), QwtText::RichText );

    if ( tracker_hook_ && tracker_hook_( pos, text ) )
        return text;

    return text;
}

QwtText
ChromatogramWidget::impl::tracker2( const QPointF& p1, const QPointF& pos )
{
    QwtText text;

    double d = ( pos.x() - p1.x() );
    if ( std::abs( d ) < 1.0 )
        text = QwtText( (boost::format("%.3fmin(&delta;=%.3f<i>s</i>),%.1f") % pos.x() % (d * 60.0) % pos.y() ).str().c_str(), QwtText::RichText );
    else
        text = QwtText( (boost::format("%.3fmin(&delta;=%.3f<i>min</i>),%.1f") % pos.x() % d % pos.y() ).str().c_str(), QwtText::RichText );

    if ( tracker_hook_ && tracker_hook_( pos, text ) )
        return text;

    return text;
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

