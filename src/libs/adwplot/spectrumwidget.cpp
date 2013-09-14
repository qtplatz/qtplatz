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

#include "spectrumwidget.hpp"
#include "zoomer.hpp"
#include "picker.hpp"
#include "plotcurve.hpp"
#include "annotation.hpp"
#include "annotations.hpp"
#include <adcontrols/annotation.hpp>
#include <adcontrols/annotations.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adportable/array_wrapper.hpp>
#include <adportable/debug.hpp>
#include <qwt_plot_picker.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_picker_machine.h>
#include <boost/format.hpp>
#include <tuple>

using namespace adwplot;

namespace adwplot {
    namespace spectrumwidget {

        static Qt::GlobalColor color_table[] = {
            Qt::blue,          // 0
            Qt::red,           // 1
            Qt::green,         // 2
            Qt::cyan,          // 3
            Qt::magenta,       // 4
            Qt::yellow,        // 5
            Qt::darkRed,       // 6
            Qt::darkGreen,     // 7
            Qt::darkBlue,      // 8
            Qt::darkCyan,      // 9
            Qt::darkMagenta,   // 10
            Qt::darkYellow,    // 11
            Qt::darkGray,      // 12
            Qt::gray,          // 13
            Qt::black,     // 14
            Qt::black,     // 15
            Qt::black,     // 16
            Qt::black,     // 17
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

            void addData( size_t size, const double * x, const double * y ) {
                size_t npos = x_.size();
                x_.resize( npos + size );
                y_.resize( npos + size );
                std::copy( x, x + size, x_.begin() + npos );
                std::copy( y, y + size, y_.begin() + npos );
            }

            void push_back( double x, double y ) {
                x_.push_back( x );
                y_.push_back( y );
            }
            size_t index( double x ) const   {
                return std::distance( x_.begin(), std::lower_bound( x_.begin(), x_.end(), x ) );
            }
            double maximum_value( size_t left, size_t right ) const {
                return *std::max_element( y_.begin() + left, y_.begin() + right );
            }
            double minimum_value( size_t left, size_t right ) const {
                return *std::min_element( y_.begin() + left, y_.begin() + right );
            }
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
            TraceData( int idx ) : idx_( idx ) {  }
            TraceData( const TraceData& t ) : idx_( t.idx_ ), curves_( t.curves_ ), data_( t.data_ ) {   }
            ~TraceData();
            void setData( Dataplot& plot, const adcontrols::MassSpectrum& ms, QRectF& );
            std::pair<double, double> y_range( double left, double right ) const;
            
            typedef std::map< int, SeriesDataImpl > map_type;
        private:
            void setProfileData( Dataplot& plot, const adcontrols::MassSpectrum& ms, const QRectF&, size_t fcn );
            void setCentroidData( Dataplot& plot, const adcontrols::MassSpectrum& ms, const QRectF&, size_t fcn );

            int idx_;
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
    if ( zoomer1_ ) {
        connect( zoomer1_.get(), SIGNAL( zoom_override( QRectF& ) ), this, SLOT( override_zoom_rect( QRectF& ) ) );
		QwtPlotZoomer * p = zoomer1_.get();
		connect( p, SIGNAL( zoomed( const QRectF& ) ), this, SLOT( zoomed( const QRectF& ) ) );
	}
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
SpectrumWidget::zoom( const QRectF& rect )
{
	Dataplot::zoom( rect );
    impl_->update_annotations( *this, std::make_pair<>( rect.left(), rect.right() ) );
}
void
SpectrumWidget::zoomed( const QRectF& rect )
{
    impl_->update_annotations( *this, std::make_pair<>( rect.left(), rect.right() ) );
}

void
SpectrumWidget::moved( const QPointF& pos )
{
	(void)pos;
}

void
SpectrumWidget::selected( const QPointF& pos )
{
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

    bool addedTrace = impl_->traces_.size() <= size_t( idx );

    while ( int( impl_->traces_.size() ) <= idx ) 
		impl_->traces_.push_back( TraceData( impl_->traces_.size() ) );

    TraceData& trace = impl_->traces_[ idx ];
    QRectF rect;
    trace.setData( *this, ms, rect );

    setAxisScale( QwtPlot::xBottom, rect.left(), rect.right() );
    setAxisScale( yaxis2 ? QwtPlot::yRight : QwtPlot::yLeft, rect.top(), rect.bottom() );

    QRectF z = zoomer1_->zoomRect();

    zoomer1_->setZoomBase();
    if ( ! addedTrace )
        zoomer1_->zoom( z );

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
TraceData::setProfileData( Dataplot& plot, const adcontrols::MassSpectrum& ms, const QRectF& rect, size_t fcn )
{
    curves_.push_back( PlotCurve( plot ) );
    PlotCurve &curve = curves_.back();
    if ( idx_ )
        curve.p()->setPen( QPen( color_table[ idx_ ] ) );
    data_[ fcn ].addData( ms.size(), ms.getMassArray(), ms.getIntensityArray() );
    curve.p()->setData( new SeriesData( data_[ fcn ], rect ) );
}

void
TraceData::setCentroidData( Dataplot& plot, const adcontrols::MassSpectrum& ms, const QRectF& rect, size_t fcn )
{
	(void)fcn;

    const unsigned char * colors = ms.getColorArray();
    if ( colors ) {
        for ( size_t i = 0; i < ms.size(); ++i )
            data_[ colors[i] ].push_back( ms.getMass( i ), ms.getIntensity( i ) );
    } else {
        data_[ 0 ].addData( ms.size(), ms.getMassArray(), ms.getIntensityArray() );
    }
    for ( const map_type::value_type& pair: data_ ) {
        curves_.push_back( PlotCurve( plot ) );
        PlotCurve& curve = curves_.back();
        if ( pair.first != 0 && unsigned( pair.first ) < sizeof( color_table ) / sizeof( color_table[0] ) )
            curve.p()->setPen( QPen( color_table[ pair.first ] ) );
        curve.p()->setData( new SeriesData( pair.second, rect ) );
        curve.p()->setStyle( QwtPlotCurve::Sticks );
    }
}

void
TraceData::setData( Dataplot& plot, const adcontrols::MassSpectrum& ms, QRectF& rect )
{
    curves_.clear();
    data_.clear();
 
    double bottom = ms.getMinIntensity();
	
    double top = ms.getMaxIntensity();
    for ( size_t fcn = 0; fcn < ms.numSegments(); ++fcn ) {
        if ( bottom > ms[ fcn ].getMinIntensity() )
            bottom = ms[ fcn ].getMinIntensity();
        if ( top < ms[ fcn ].getMaxIntensity() )
            top = ms[ fcn ].getMaxIntensity();
    }
	if ( ms.isCentroid() )
		bottom = 0;
	top = top + ( top - bottom ) * 0.12; // add 12% margine for annotation

    rect.setCoords( ms.getAcquisitionMassRange().first, bottom, ms.getAcquisitionMassRange().second, top );

    if ( ms.isCentroid() ) {
        setCentroidData( plot, ms, rect, 0 );
        for ( size_t fcn = 0; fcn < ms.numSegments(); ++fcn )
            setCentroidData( plot, ms[fcn], rect, fcn + 1 );
    } else { // Profile
        setProfileData( plot, ms, rect, 0 );
        for ( size_t fcn = 0; fcn < ms.numSegments(); ++fcn )
            setProfileData( plot, ms[fcn], rect, fcn + 1 );
    }
}

std::pair< double, double >
TraceData::y_range( double left, double right ) const
{
    double top = 100;
    double bottom = -10;
    for ( const map_type::value_type& pair: data_ ) {

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

    typedef std::tuple< size_t, size_t, int, double, double > peak; // fcn, idx, color, mass, intensity
    enum { c_fcn, c_idx, c_color, c_intensity, c_mass };

    std::vector< peak > peaks;
    adcontrols::sequence_wrapper< const adcontrols::MassSpectrum > segments( centroid_ );

    adcontrols::annotations auto_annotations;
    adcontrols::annotations annotations;
    
    for ( size_t fcn = 0; fcn < segments.size(); ++fcn ) {
        const adcontrols::MassSpectrum& ms = segments[ fcn ];
        const unsigned char * colors = ms.getColorArray();
        double max_y = ms.getMaxIntensity();

        array_wrapper< const double > masses( ms.getMassArray(), ms.size() );
        size_t beg = std::distance( masses.begin(), std::lower_bound( masses.begin(), masses.end(), range.first ) );
        size_t end = std::distance( masses.begin(), std::lower_bound( masses.begin(), masses.end(), range.second ) );

        if ( beg < end ) {
            const adcontrols::annotations& attached = ms.get_annotations();
            for ( auto& a : attached ) {
                if ( ( int(beg) <= a.index() && a.index() <= int(end) ) || ( range.first < a.x() && a.x() < range.second ) )
                    annotations << a;
            }

            // generate auto-annotation
            for ( size_t idx = beg; idx <= end; ++idx ) {
                if ( std::find_if( attached.begin(), attached.end()
                                   , [idx]( const adcontrols::annotation& a ){ return a.index() == idx; } ) 
                     == attached.end() ) {
                    // if no attached annotation
                    int pri = ms.getIntensity( idx ) / max_y * 1000;
                    if ( colors )
                        pri *= 100;
                    adcontrols::annotation annot( ( boost::wformat( L"%.4lf" ) % ms.getMass( idx ) ).str()
                                                  , ms.getMass( idx ), ms.getIntensity( idx ), ( fcn << 24 | idx ), pri );
                    auto_annotations << annot;
                }
            }
        }
    }

	auto_annotations.sort();
    annotations.sort();

	annotations_.clear();
    Annotations annots(plot, annotations_);

    size_t n = 0;

    for ( const auto& a: annotations ) {
        if ( ++n <= 20 ) {
            Annotation anno = annots.add( a.x(), a.y(), a.text() );
			anno.setLabelAlighment( Qt::AlignTop | Qt::AlignCenter );
		}
	}

    for ( const auto& a: auto_annotations ) {
        if ( ++n <= 20 ) {
            Annotation anno = annots.add( a.x(), a.y(), a.text() );
			anno.setLabelAlighment( Qt::AlignTop | Qt::AlignCenter );
		}
	}
}

void
SpectrumWidgetImpl::clear()
{
    centroid_ = adcontrols::MassSpectrum();
    annotations_.clear();
    traces_.clear();
}
