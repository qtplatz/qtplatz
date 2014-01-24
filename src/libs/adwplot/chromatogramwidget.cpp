// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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
#include <qwt_picker_machine.h>
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
#include <boost/format.hpp>
#include <boost/variant.hpp>
#include <queue>

using namespace adwplot;

namespace adwplot {

    namespace chromatogram_widget {

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
			TraceData( Dataplot& plot ) : curve_( plot ) { }
			TraceData( const TraceData& t ) : curve_( t.curve_ ), rect_( t.rect_ ) { }
            void setData( const T& ) {  }
			const QRectF& boundingRect() const { return rect_; };
			QwtPlotCurve& plot_curve() { return *curve_.p(); }
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
			rect_.setCoords( d_trace->sample(0).x(), trace.range_y().second
                             , d_trace->sample( trace.size() - 1 ).x(), trace.range_y().first );
			d_trace->boundingRect( rect_ );
			curve_.p()->setData( d_trace );
        }

        template<> void TraceData<adcontrols::Chromatogram>::setData( const adcontrols::Chromatogram& c )
        {
            using adcontrols::Chromatogram;
            const double * intens = c.getIntensityArray();

			series_data< Chromatogram > * d_series = new series_data< Chromatogram >( c );
			rect_.setCoords( d_series->sample(0).x(), intens[ c.min_element() ]
                             , d_series->sample( c.size() - 1 ).x(), intens[ c.max_element() ] );
			d_series->boundingRect( rect_ );
			curve_.p()->setData( d_series );
        }

        typedef boost::variant< TraceData<adcontrols::Chromatogram>, TraceData<adcontrols::Trace> > trace_variant;

        struct boundingRect_visitor : public boost::static_visitor< QRectF > {
            template<typename T> QRectF operator()( const T& t ) const { return t.boundingRect(); }
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

        void clear();
        void update_annotations( const std::pair<double, double>&, adcontrols::annotations& ) const;
		void clear_annotations();

        QwtText tracker1( const QPointF& );
        QwtText tracker2( const QPointF&, const QPointF& );
    };
    
}

ChromatogramWidget::~ChromatogramWidget()
{
    delete impl_;
}

ChromatogramWidget::ChromatogramWidget(QWidget *parent) : Dataplot(parent)
                                                        , impl_( new ChromatogramWidgetImpl )
{
	QwtText axisHor( "Time[min]" );
	QFont font = axisHor.font();
	font.setFamily( "Verdana" );
	font.setBold( true );
	font.setItalic( false );
	font.setPointSize( 9 );
	axisHor.setFont( font );

    setAxisTitle(QwtPlot::xBottom, axisHor);

	QwtText axisVert( "Intensity" );
	font.setItalic( false );
	axisVert.setFont( font );
    setAxisTitle(QwtPlot::yLeft, axisVert );

    // -----------
    font.setFamily( "Colsolas" );
    font.setBold( false );
	font.setPointSize( 8 );
    setAxisFont( QwtPlot::xBottom, font );
    setAxisFont( QwtPlot::yLeft, font );

    if ( zoomer1_ ) {
        using namespace std::placeholders;
        zoomer1_->tracker1( std::bind( &ChromatogramWidgetImpl::tracker1, impl_, _1 ) );
        zoomer1_->tracker2( std::bind( &ChromatogramWidgetImpl::tracker2, impl_, _1, _2 ) );

        connect( zoomer1_.get(), SIGNAL( zoom_override( QRectF& ) ), this, SLOT( override_zoom_rect( QRectF& ) ) );
		QwtPlotZoomer * p = zoomer1_.get();
		connect( p, SIGNAL( zoomed( const QRectF& ) ), this, SLOT( zoomed( const QRectF& ) ) );
	}
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
            zoomer1_->setZoomBase();
        }
    }
}

void
ChromatogramWidget::setData( const adcontrols::Chromatogram& c, int idx, bool )
{
    impl_->clear();

    if ( c.size() < 2 )
        return;

    using adcontrols::Chromatogram;
    using chromatogram_widget::TraceData;

    while ( int ( impl_->traces_.size() ) <= idx )
		impl_->traces_.push_back( TraceData<Chromatogram>( *this ) );        

	TraceData<Chromatogram>& trace = boost::get<TraceData<Chromatogram> >(impl_->traces_[ idx ] );

	trace.plot_curve().setPen( QPen( chromatogram_widget::color_table[idx] ) ); 
    trace.setData( c );

    const double * intens = c.getIntensityArray();

    impl_->peak_annotations_.clear();
    for ( auto& pk: c.peaks() )
        setPeak( pk, impl_->peak_annotations_ );

    impl_->peak_annotations_.sort();

    plotAnnotations( impl_->peak_annotations_ );
    
    if ( idx == 0 ) {
        std::pair< double, double > time_range;
        time_range.first = adcontrols::timeutil::toMinutes( adcontrols::seconds_t( c.timeRange().first ) );
        time_range.second = adcontrols::timeutil::toMinutes( adcontrols::seconds_t( c.timeRange().second ) );
        setAxisScale( QwtPlot::xBottom, time_range.first, time_range.second );
		double hMin = intens[ c.min_element() ];
		double hMax = intens[ c.max_element() ];
        double h = hMax - hMin;
        setAxisScale( QwtPlot::yLeft, hMin - ( h * 0.05 ), hMax + ( h * 0.1 ) );
        zoomer1_->setZoomBase();
    }
}

void
ChromatogramWidget::setData( const adcontrols::PeakResult& r )
{
	using adcontrols::Peaks;
	using adcontrols::Baselines;

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

    impl_->peaks_.push_back( adwplot::Peak( *this, peak ) );
}

void
ChromatogramWidget::setBaseline( const adcontrols::Baseline& bs )
{
    impl_->baselines_.push_back( adwplot::Baseline( *this, bs ) );
}

void
ChromatogramWidget::plotAnnotations( const adcontrols::annotations& vec )
{
    adwplot::Annotations w( *this, impl_->annotation_markers_ );

    for ( auto& a: vec ) {
		QwtText text( QString::fromStdWString( a.text() ), QwtText::RichText );
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
    adcontrols::annotations vec;
    impl_->update_annotations( std::make_pair( rect.left(), rect.right() ), vec );
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
}

void
ChromatogramWidgetImpl::clear_annotations()
{
	annotation_markers_.clear();
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
    return QwtText( (boost::format("%.3fmin, %.1f") % pos.x() % pos.y() ).str().c_str(), QwtText::RichText );
}

QwtText
ChromatogramWidgetImpl::tracker2( const QPointF& p1, const QPointF& pos )
{
    double d = ( pos.x() - p1.x() );
    if ( std::abs( d ) < 1.0 )
        return QwtText( (boost::format("%.3fmin(&delta;=%.3f<i>s</i>),%.1f") % pos.x() % (d * 60.0) % pos.y() ).str().c_str(), QwtText::RichText );
    else
        return QwtText( (boost::format("%.3fmin(&delta;=%.3f<i>min</i>),%.1f") % pos.x() % d % pos.y() ).str().c_str(), QwtText::RichText );
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

