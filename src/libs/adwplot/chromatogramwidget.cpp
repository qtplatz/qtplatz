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
#include <adportable/debug.hpp>
#include <boost/format.hpp>
#include <boost/variant.hpp>
#include <queue>

using namespace adwplot;

namespace adwplot {

    namespace chromatogram_internal {

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
            namespace ci = chromatogram_internal;

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
    }

    struct ChromatogramWidgetImpl : boost::noncopyable {
        std::vector< Annotation > annotations_;
        std::vector< chromatogram_internal::trace_variant > traces_;
        std::vector< Peak > peaks_;
        std::vector< Baseline > baselines_;	
    };
}

ChromatogramWidget::~ChromatogramWidget()
{
    delete impl_;
}

ChromatogramWidget::ChromatogramWidget(QWidget *parent) : Dataplot(parent)
							, impl_( new ChromatogramWidgetImpl )
{
    setAxisTitle(QwtPlot::xBottom, "Time[min]");
    setAxisTitle(QwtPlot::yLeft, "Intensity[uV]");

	if ( picker_ ) {
		// picker_->setStateMachine( new QwtPickerClickPointMachine() );
		connect( picker_.get(), SIGNAL( moved( const QPointF& ) ), this, SLOT( moved( const QPointF& ) ) );
		connect( picker_.get(), SIGNAL( selected( const QPointF& ) ), this, SLOT( selected( const QPointF& ) ) );
		connect( picker_.get(), SIGNAL( selected( const QRectF& ) ), this, SLOT( selected( const QRectF& ) ) );
        picker_->setEnabled( true );
	}
}

void
ChromatogramWidget::setData( const adcontrols::Trace& c, int idx, bool yaxis2 )
{
    (void)yaxis2;

    if ( c.size() < 2 )
        return;

    using adcontrols::Trace;
    using chromatogram_internal::TraceData;

    while ( int( impl_->traces_.size() ) <= idx )
        impl_->traces_.push_back( TraceData<Trace>( *this ) );

    TraceData<Trace> * trace = 0;
    try {
        trace = &boost::get< TraceData<Trace> >( impl_->traces_[ idx ] );
    } catch ( boost::bad_get& ex ) {
        adportable::debug(__FILE__, __LINE__) << ex.what();
    }

    if ( trace ) {

        trace->plot_curve().setPen( QPen( chromatogram_internal::color_table[ idx ] ) );
        trace->setData( c );

        if ( idx == 0 ) {
            QRectF rect = trace->boundingRect();
            for ( const auto& it: impl_->traces_ ) {
                QRectF rc = boost::apply_visitor( chromatogram_internal::boundingRect_visitor(),  it );
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
    impl_->annotations_.clear();
    impl_->peaks_.clear();
    impl_->baselines_.clear();

    if ( c.size() < 2 )
        return;

    using adcontrols::Chromatogram;
    using chromatogram_internal::TraceData;

    while ( int ( impl_->traces_.size() ) <= idx )
		impl_->traces_.push_back( TraceData<Chromatogram>( *this ) );        

	TraceData<Chromatogram>& trace = boost::get<TraceData<Chromatogram> >(impl_->traces_[ idx ] );

	trace.plot_curve().setPen( QPen( chromatogram_internal::color_table[idx] ) ); 
    trace.setData( c );

    const double * intens = c.getIntensityArray();

    const adcontrols::Peaks& peaks = c.peaks();
    for ( adcontrols::Peaks::vector_type::const_iterator it = peaks.begin(); it != peaks.end(); ++it )
        setPeak( *it );
    
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

	for ( Peaks::vector_type::const_iterator it = r.peaks().begin(); it != r.peaks().end(); ++it )
		setPeak( *it );
}    

void
ChromatogramWidget::setPeak( const adcontrols::Peak& peak )
{
    double tR = adcontrols::timeutil::toMinutes( peak.peakTime() );

    std::wstring label = peak.name();
    if ( label.empty() )
        label = ( boost::wformat( L"%.3lf" ) % tR ).str();

    Annotations annots( *this, impl_->annotations_ );

	Annotation anno = annots.add( tR, peak.topHeight(), label );
    anno.setLabelAlighment( Qt::AlignTop | Qt::AlignCenter );

    impl_->peaks_.push_back( adwplot::Peak( *this, peak ) );
}

void
ChromatogramWidget::setBaseline( const adcontrols::Baseline& bs )
{
    impl_->baselines_.push_back( adwplot::Baseline( *this, bs ) );
}

void
ChromatogramWidget::override_zoom_rect( QRectF& )
{
	// update rect if auto Y scale and/or apply minimum zoom etc.
}

void
ChromatogramWidget::zoom( const QRectF& rect )
{
    zoomer1_->zoom( rect );
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

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

