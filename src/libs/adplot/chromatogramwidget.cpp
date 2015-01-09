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

using namespace adplot;

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

            size_t size() const { 
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

            QRectF boundingRect() const { return rect_; }
        };

        template<class T> class series_data : public QwtSeriesData< QPointF >, boost::noncopyable {
            T t_;
        public:
            series_data() { }
			series_data( const T& t ) : t_( t ) { }
			~series_data() { }
			inline operator T& () {	return t_;	};
            // implements QwtSeriesData<>
            virtual size_t size() const { return t_.size(); }
            virtual QPointF sample( size_t idx ) const { 
                using adcontrols::Chromatogram;
                return QPointF( Chromatogram::toMinutes( t_.getTimeArray()[idx] ), t_.getIntensityArray()[idx] );
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
            void drawMarkers( QwtPlot *, const std::pair< double, double >& ) {}

        private:
            PlotCurve curve_;
			QRectF rect_;
        };

        template<> void TraceData<adcontrols::Trace>::setData( const adcontrols::Trace& trace )
        {
            using adcontrols::Trace;
            namespace cw = chromatogram_widget;

            if ( trace.size() <= 2 )
                return;

            // TODO:  refactor code in order to avoid full data copy
			series_data< Trace > * d_trace = new series_data< Trace >( trace );
            rect_.setCoords( d_trace->sample( 0 ).x(), trace.range_y().second, d_trace->sample( trace.size() - 1 ).x(), trace.range_y().first );
            d_trace->boundingRect( rect_ );
			curve_.p()->setData( d_trace );
        }

        class ChromatogramData {
        public:
            ~ChromatogramData() { }
			ChromatogramData( plot& plot ) : curve_( plot ) { }
            ChromatogramData( const ChromatogramData& t ) : curve_( t.curve_ ), rect_( t.rect_ ), grab_( t.grab_ ) { }

            void setData( const std::shared_ptr< adcontrols::Chromatogram>& cp ) {
                grab_ = cp;
                auto range_x = adcontrols::Chromatogram::toMinutes( cp->timeRange() );
                auto range_y = std::pair<double, double>( cp->getMinIntensity(), cp->getMaxIntensity() );
                rect_.setCoords( range_x.first, range_y.second, range_x.second, range_y.first );
                curve_.p()->setData( new xSeriesData( cp, rect_ ) );
            }
			const QRectF& boundingRect() const { return rect_; };
			QwtPlotCurve& plot_curve() { return *curve_.p(); }
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
        };
        
        typedef boost::variant< ChromatogramData, TraceData<adcontrols::Trace> > trace_variant;

        struct boundingRect_visitor : public boost::static_visitor< QRectF > {
            template<typename T> QRectF operator()( const T& t ) const { return t.boundingRect(); }
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
    
    struct ChromatogramWidgetImpl : boost::noncopyable {
        
        adcontrols::annotations peak_annotations_;
        std::vector< Annotation > annotation_markers_;
        
        std::vector< chromatogram_widget::trace_variant > traces_;
        std::vector< Peak > peaks_;
        std::vector< Baseline > baselines_;	
        std::function< bool( const QPointF&, QwtText& ) > tracker_hook_;
        
        void clear();
        void removeData( int );
        void update_annotations( const std::pair<double, double>&, adcontrols::annotations& ) const;
		void clear_annotations();
        void update_markers( const std::pair<double, double>& ) const;
        
        QwtText tracker1( const QPointF& );
        QwtText tracker2( const QPointF&, const QPointF& );
        
    };
    
}

ChromatogramWidget::~ChromatogramWidget()
{
    delete impl_;
}

ChromatogramWidget::ChromatogramWidget(QWidget *parent) : plot(parent)
                                                        , impl_( new ChromatogramWidgetImpl )
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
        zoomer->tracker1( std::bind( &ChromatogramWidgetImpl::tracker1, impl_, _1 ) );
        zoomer->tracker2( std::bind( &ChromatogramWidgetImpl::tracker2, impl_, _1, _2 ) );
        
        connect( zoomer, static_cast<void(Zoomer::*)(QRectF&)>(&Zoomer::zoom_override), this, &ChromatogramWidget::override_zoom_rect );
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
        impl_->traces_[ idx ] = chromatogram_widget::ChromatogramData( *this );
    if ( bReplot )
        replot();
}

void
ChromatogramWidget::setData( const adcontrols::Trace& c, int idx, bool yaxis2 )
{
    (void)yaxis2;

    if ( c.size() < 2 )
        return;
    
    using adcontrols::Trace;
    using chromatogram_widget::TraceData;
    
    while ( int( impl_->traces_.size() ) <= idx )
        impl_->traces_.push_back( TraceData<Trace>( *this ) );
    
    TraceData<Trace> * trace = 0;
    try {
        trace = &boost::get< TraceData<Trace> >( impl_->traces_[ idx ] );
    } catch ( boost::bad_get& ex ) {
        adportable::debug(__FILE__, __LINE__) << ex.what();
    }
    
    if ( trace ) {
        
        trace->plot_curve().setPen( QPen( chromatogram_widget::color_table[ idx ] ) );
        trace->setData( c );
        
        if ( idx == 0 ) {
            QRectF rect = trace->boundingRect();
            for ( const auto& it: impl_->traces_ ) {
                QRectF rc = boost::apply_visitor( chromatogram_widget::boundingRect_visitor(),  it );
                if ( rc.bottom() < rect.bottom() )
                    rect.setBottom( rc.bottom() );
                if ( rc.top() > rect.top() )
                    rect.setTop( rc.top() );
            }
            setAxisScale( QwtPlot::xBottom, rect.left(), rect.right() + rect.width() / 20.0 );
            setAxisScale( yaxis2 ? QwtPlot::yRight : QwtPlot::yLeft, rect.bottom(), rect.top() );
            zoomer()->setZoomBase();
        }
    }
}

void
ChromatogramWidget::setData( const std::shared_ptr< adcontrols::Chromatogram >& cp, int idx, bool )
{
    if ( cp->size() < 2 )
        return;

    using adcontrols::Chromatogram;
    using chromatogram_widget::ChromatogramData; // TraceData;
    
    while ( int ( impl_->traces_.size() ) <= idx )
		impl_->traces_.push_back( ChromatogramData( *this ) );
    
	auto& trace = boost::get< ChromatogramData >(impl_->traces_[ idx ] );

	trace.plot_curve().setPen( QPen( chromatogram_widget::color_table[idx] ) ); 
    trace.setData( cp );
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
        QRectF rc = trace.boundingRect();
        horizontal.first = std::min( horizontal.first, rc.left() );
        horizontal.second = std::max( horizontal.second, rc.right() );
        vertical.first = std::min( vertical.first, rc.bottom() );
        vertical.second = std::max( vertical.second, rc.top() );
    }
    double h = vertical.second - vertical.first;
    vertical.first -= h * 0.05;
    vertical.second += h * 0.10;
    
    setAxisScale( QwtPlot::xBottom, horizontal.first, horizontal.second );
    setAxisScale( QwtPlot::yLeft, vertical.first, vertical.second );
    zoomer()->setZoomBase();
}

void
ChromatogramWidget::setData( const adcontrols::PeakResult& r )
{
	using adcontrols::Peaks;
	using adcontrols::Baselines;

    impl_->peaks_.clear();
    impl_->baselines_.clear();

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
    adplot::Annotations w( *this, impl_->annotation_markers_ );
    
    for ( auto& a: vec ) {
		QwtText text( QString::fromStdString( a.text() ), QwtText::RichText );
        text.setColor( Qt::darkGreen );
        text.setFont( Annotation::font() );
        w.insert( a.x(), a.y(), text, Qt::AlignTop | Qt::AlignHCenter );
    }
}

void
ChromatogramWidget::override_zoom_rect( QRectF& )
{
	// update rect if auto Y scale and/or apply minimum zoom etc.
    
}

void
ChromatogramWidget::zoomed( const QRectF& rect )
{
    auto range = std::make_pair( rect.left(), rect.right() );
    chromatogram_widget::updateMarkers_visitor drawMarker( this, range );
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
ChromatogramWidgetImpl::clear()
{
    peaks_.clear();
    baselines_.clear();
	annotation_markers_.clear();
    traces_.clear();
}

void
ChromatogramWidgetImpl::removeData( int idx )
{
    if ( traces_.size() > size_t(idx) ) {
        peaks_.clear();
        baselines_.clear();
        annotation_markers_.clear();
    }
}

void
ChromatogramWidgetImpl::clear_annotations()
{
	annotation_markers_.clear();
}

void
ChromatogramWidgetImpl::update_markers( const std::pair<double, double>& ) const
{
}

void
ChromatogramWidgetImpl::update_annotations( const std::pair<double, double>& range
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
ChromatogramWidgetImpl::tracker1( const QPointF& pos )
{
    QwtText text = QwtText( (boost::format("%.3fmin, %.1f") % pos.x() % pos.y() ).str().c_str(), QwtText::RichText );

    if ( tracker_hook_ && tracker_hook_( pos, text ) )
        return text;

    return text;
}

QwtText
ChromatogramWidgetImpl::tracker2( const QPointF& p1, const QPointF& pos )
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

