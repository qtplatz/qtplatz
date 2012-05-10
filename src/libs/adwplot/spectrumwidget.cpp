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
#include "picker.hpp"
#include "plotcurve.hpp"
#include "annotation.hpp"
#include "annotations.hpp"
#include <adcontrols/massspectrum.hpp>
#include <adportable/array_wrapper.hpp>
#include <qwt_plot_picker.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_picker_machine.h>
#include <boost/foreach.hpp>
#include <boost/format.hpp>

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
	    void setData( Dataplot& plot, const adcontrols::MassSpectrum& ms );
        std::pair<double, double> y_range( double left, double right ) const;

	    typedef std::map< int, SeriesDataImpl > map_type;
	private:
	    std::vector< PlotCurve > curves_;
	    map_type data_;
	};
    } // namespace spectrumwidget

    struct SpectrumWidgetImpl {
        adcontrols::MassSpectrum centroid_;  // for annotation
        std::vector< Annotation > annotations_;
        std::vector< spectrumwidget::TraceData > traces_;

        void clear();
        void update_annotations( Dataplot&, const std::pair<double, double>& );
		void clear_annotations();
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
    zoomer2_.reset();
	zoomer1_->autoYScale( autoYZoom_ );

    setAxisTitle(QwtPlot::xBottom, "m/z");
    setAxisTitle(QwtPlot::yLeft, "Intensity");

    // handle zoom rect by this
    if ( zoomer1_ )
        connect( zoomer1_.get(), SIGNAL( zoom_override( QRectF& ) ), this, SLOT( override_zoom_rect( QRectF& ) ) );

	if ( picker_ ) {
		connect( picker_.get(), SIGNAL( moved( const QPointF& ) ), this, SLOT( moved( const QPointF& ) ) );
		connect( picker_.get(), SIGNAL( selected( const QRectF& ) ), this, SLOT( selected( const QRectF& ) ) );
		picker_->setEnabled( true );
	}
}

void
SpectrumWidget::override_zoom_rect( QRectF& rc )
{
    if ( autoYZoom_ ) {
        using spectrumwidget::TraceData;
        double bottom = rc.bottom();
        double top = rc.top();
        BOOST_FOREACH( const TraceData& trace, impl_->traces_ ) {
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
SpectrumWidget::zoom( const QRectF& rect )
{
	Dataplot::zoom( rect );
    impl_->update_annotations( *this, std::make_pair<>( rect.left(), rect.right() ) );
}

void
SpectrumWidget::moved( const QPointF& pos )
{
	(void)pos;
	//std::cout << "SpectrumWidget::moved( " << pos.x() << ", " << pos.y() << ")" << std::endl;
}

void
SpectrumWidget::selected( const QPointF& pos )
{
	//std::cout << "SpectrumWidget::selected( " << pos.x() << ", " << pos.y() << ")" << std::endl;
	emit onSelected( pos );
}

void
SpectrumWidget::selected( const QRectF& rect )
{
	emit onSelected( rect );
}

void
SpectrumWidget::clear()
{
    impl_->clear();
    zoomer1_->setZoomBase();
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

    double top = ms.getMaxIntensity() + ( ms.getMaxIntensity() - ms.getMinIntensity() ) * 0.12; // 12% increase
    double bottom = ms.getMinIntensity();
    setAxisScale( QwtPlot::xBottom, ms.getAcquisitionMassRange().first, ms.getAcquisitionMassRange().second );
    setAxisScale( yaxis2 ? QwtPlot::yRight : QwtPlot::yLeft, bottom, top );

    zoomer1_->setZoomBase();
    // todo: annotations
    if ( ms.isCentroid() ) {
        impl_->centroid_ = ms;
        impl_->clear_annotations();
        impl_->update_annotations( *this, ms.getAcquisitionMassRange() );
    }
    // replot();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

using namespace adwplot::spectrumwidget;

TraceData::~TraceData()
{
    curves_.clear();
    data_.clear();
}

void
TraceData::setData( Dataplot& plot, const adcontrols::MassSpectrum& ms )
{
    curves_.clear();
    data_.clear();
 
    const double * intens = ms.getIntensityArray();
    const double * masses = ms.getMassArray();
    const size_t size = ms.size();

    double bottom = ms.getMinIntensity();
    double top = ms.getMaxIntensity() + ( ms.getMaxIntensity() - ms.getMinIntensity() ) * 0.12;
    QRectF rect;
    rect.setCoords( ms.getAcquisitionMassRange().first, bottom, ms.getAcquisitionMassRange().second, top );

    if ( ms.isCentroid() ) {
        const unsigned char * colors = ms.getColorArray();
        if ( colors ) {
            for ( size_t i = 0; i < size; ++i )
                data_[ colors[i] ].push_back( masses[i], intens[i] );
        } else {
            data_[ 0 ].setData( size, masses, intens );
        }
        BOOST_FOREACH( const map_type::value_type& pair, data_ ) {
            curves_.push_back( PlotCurve( plot ) );
            PlotCurve& curve = curves_.back();
            if ( pair.first != 0 && unsigned( pair.first ) < sizeof( color_table ) / sizeof( color_table[0] ) )
                curve.p()->setPen( QPen( color_table[ pair.first ] ) );
            curve.p()->setData( new SeriesData( pair.second, rect ) );
            curve.p()->setStyle( QwtPlotCurve::Sticks );
        }

    } else {
        curves_.push_back( PlotCurve( plot ) );
        PlotCurve &curve = curves_[0];
        data_[ 0 ].setData( size, masses, intens );
        curve.p()->setData( new SeriesData( data_[ 0 ], rect ) );
    }
}

std::pair< double, double >
TraceData::y_range( double left, double right ) const
{
    double top = 100;
    double bottom = -10;
    BOOST_FOREACH( const map_type::value_type& pair, data_ ) {

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

namespace adwplot {
    namespace spectrumwidget {

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
SpectrumWidgetImpl::clear_annotations()
{
	annotations_.clear();
}

void
SpectrumWidgetImpl::update_annotations( Dataplot& plot
                                       , const std::pair<double, double>& range )
{
    using adportable::array_wrapper;

    const adcontrols::MassSpectrum& ms = centroid_;

    array_wrapper< const double > masses( ms.getMassArray(), ms.size() );
    size_t beg = std::distance( masses.begin(), std::lower_bound( masses.begin(), masses.end(), range.first ) );
    size_t end = std::distance( masses.begin(), std::lower_bound( masses.begin(), masses.end(), range.second ) );

    std::vector< size_t > indecies;
    for ( size_t idx = beg; idx <= end; ++idx )
        indecies.push_back( idx );

    std::sort( indecies.begin(), indecies.end(), compare_priority( ms.getIntensityArray(), ms.getColorArray() ) );

    Annotations annots(plot, annotations_);
    size_t n = 0;
    for ( std::vector<size_t>::const_iterator it = indecies.begin(); it != indecies.end() && n <= 5; ++it, ++n ) {
        std::wstring label = ( boost::wformat( L"%.4lf" ) % ms.getMass( *it ) ).str();
        Annotation anno = annots.add( ms.getMass( *it ), ms.getIntensity( *it ), label );
        anno.setLabelAlighment( Qt::AlignTop | Qt::AlignCenter );
    }
}

void
SpectrumWidgetImpl::clear()
{
    centroid_ = adcontrols::MassSpectrum();
    annotations_.clear();
    traces_.clear();
}
