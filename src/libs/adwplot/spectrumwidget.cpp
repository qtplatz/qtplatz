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

#include "spectrumwidget.hpp"
#include "zoomer.hpp"
#include "picker.hpp"
#include "adwplotcurve.hpp"
#include "annotation.hpp"
#include "annotations.hpp"
#include <adcontrols/annotation.hpp>
#include <adcontrols/annotations.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adportable/array_wrapper.hpp>
#include <adportable/float.hpp>
#include <adportable/debug.hpp>
#include <adportable/scoped_debug.hpp>
#include <qtwrapper/font.hpp>
#include <qwt_plot_picker.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_picker_machine.h>
#include <qwt_text.h>
#include <boost/format.hpp>
#include <set>

using namespace adwplot;

namespace adwplot {
    namespace spectrumwidget {

        static Qt::GlobalColor color_table[] = {
            Qt::blue          // 0
            , Qt::red           // 1
            , Qt::green         // 2
            , Qt::cyan          // 3
            , Qt::magenta       // 4
            , Qt::yellow        // 5
            , Qt::darkRed       // 6
            , Qt::darkGreen     // 7
            , Qt::darkBlue      // 8
            , Qt::darkCyan      // 9
            , Qt::darkMagenta   // 10
            , Qt::darkYellow    // 11
            , Qt::darkGray      // 12
            , Qt::black         // 13
            , Qt::lightGray      // 14
            , Qt::white          // 15
            , Qt::gray    // 16
            , Qt::transparent    // 17
        };
        
        class xSeriesData : public QwtSeriesData<QPointF>, boost::noncopyable {
        public:
            virtual ~xSeriesData() {
            }

            xSeriesData( const adcontrols::MassSpectrum& ms
                         , const QRectF& rc
                         , bool axisTime ) : rect_( rc )
                                           , ms_( ms )
                                           , axisTime_( axisTime ) {
            }

            size_t size() const override { 
                if ( ! indecies_.empty() )
                    return indecies_.size();
                return ms_.size();
            }

            QPointF sample( size_t idx ) const override {
                if ( ! indecies_.empty() && idx < indecies_.size() ) // if colored
                    idx = indecies_[ idx ];
                using namespace adcontrols::metric;
                if ( axisTime_ )
                    return QPointF( scale_to<double, micro>( ms_.getTime( idx  )), ms_.getIntensity( idx ) );
                else
                    return QPointF( ms_.getMass( idx ), ms_.getIntensity( idx ) );
            }

            QRectF boundingRect() const override {
                return rect_;
            }

            size_t make_color_index( unsigned char color ) {
                const unsigned char * colors = ms_.getColorArray();
                for ( size_t i = 0; i < ms_.size(); ++i ) {
                    if ( color == colors[i] )
                        indecies_.push_back( i );
                }
                return indecies_.size();
            }

        private:
            QRectF rect_;
            const adcontrols::MassSpectrum& ms_;
            std::vector< size_t > indecies_; // if centroid with color, 
            bool axisTime_;
        };
	
        class TraceData {
        public:
            TraceData( int idx ) : idx_( idx )
                                 , focusedFcn_( -1 )
                                 , yRight_( false )  {
            }
            TraceData( const TraceData& t ) : idx_( t.idx_ )
                , focusedFcn_( t.focusedFcn_ )
                , rect_( t.rect_ )
                , yRight_( t.yRight_ )
                , curves_( t.curves_ )
                , pSpectrum_( t.pSpectrum_ )
                , isTimeAxis_( t.isTimeAxis_ ) {
            }
            ~TraceData();
            void setData( Dataplot& plot
                          , const std::shared_ptr< adcontrols::MassSpectrum>&
                          , QRectF&, SpectrumWidget::HorizontalAxis, bool yRight );
            void redraw( Dataplot& plot, SpectrumWidget::HorizontalAxis, QRectF&, QRectF& );
            void setFocusedFcn( int fcn );
            std::pair<double, double> y_range( double left, double right ) const;
            bool yRight() const { return yRight_; }

        private:
            void setProfileData( Dataplot& plot, const adcontrols::MassSpectrum& ms, const QRectF&, bool yRight );
            void setCentroidData( Dataplot& plot, const adcontrols::MassSpectrum& ms, const QRectF&, bool yRight );
            void changeFocus( int focusedFcn );

            int idx_;
            int focusedFcn_;
            QRectF rect_;
            bool yRight_;
            std::vector< std::shared_ptr< AdwPlotCurve > > curves_;
            std::shared_ptr< adcontrols::MassSpectrum > pSpectrum_;
			bool isTimeAxis_;
        };
        
    } // namespace spectrumwidget

    struct SpectrumWidgetImpl {
        SpectrumWidgetImpl() : autoAnnotation_( true )
                             , isTimeAxis_( false ) {
        }
        std::weak_ptr< adcontrols::MassSpectrum > centroid_;  // for annotation
        std::vector< Annotation > annotations_;
        std::vector< spectrumwidget::TraceData > traces_;
        bool autoAnnotation_;
        bool isTimeAxis_;

        void clear();
        void update_annotations( Dataplot&, const std::pair<double, double>& );
		void clear_annotations();

        QwtText tracker1( const QPointF& );
        QwtText tracker2( const QPointF&, const QPointF& );
    };

} // namespace adwplot

SpectrumWidget::~SpectrumWidget()
{
    delete impl_;
}

SpectrumWidget::SpectrumWidget(QWidget *parent) : Dataplot(parent)
                                                , impl_( new SpectrumWidgetImpl )
                                                , autoYZoom_( true ) 
                                                , keepZoomed_( true )
                                                , axisHadScaled_( false )
                                                , haxis_( HorizontalAxisMass )
                                                , focusedFcn_( -1 ) // no focus
{
	zoomer1_->autoYScale( autoYZoom_ );

    using namespace std::placeholders;
    zoomer1_->tracker1( std::bind( &SpectrumWidgetImpl::tracker1, impl_, _1 ) );
    zoomer1_->tracker2( std::bind( &SpectrumWidgetImpl::tracker2, impl_, _1, _2 ) );

	if ( picker_ ) {
		connect( picker_.get(), SIGNAL( moved( const QPointF& ) ), this, SLOT( moved( const QPointF& ) ) );
		connect( picker_.get(), SIGNAL( selected( const QRectF& ) ), this, SLOT( selected( const QRectF& ) ) );
		picker_->setEnabled( true );
	}

    setAxisTitle(QwtPlot::xBottom, QwtText( "<i>m/z</i>", QwtText::RichText ) );
    setAxisTitle(QwtPlot::yLeft, QwtText( "Intensity" ) );

    // -----------
    QFont font;
    qtwrapper::font::setFont( font, qtwrapper::fontSizeSmall, qtwrapper::fontAxisLabel );

    // handle zoom rect by this
    if ( zoomer1_ ) {
        connect( zoomer1_.get(), SIGNAL( zoom_override( QRectF& ) ), this, SLOT( override_zoom_rect( QRectF& ) ) );
		QwtPlotZoomer * p = zoomer1_.get();
        connect( p, &QwtPlotZoomer::zoomed, this, &SpectrumWidget::zoomed );
	}
}

void
SpectrumWidget::update_annotation( bool bReplot )
{
    QRectF rc = zoomRect();
    impl_->update_annotations( *this, std::make_pair( rc.left(), rc.right() ) );
    if ( bReplot )
        replot();
}

void
SpectrumWidget::setZoomBase( const std::pair< double, double >& range, bool horizontal )
{
    QRectF bz = zoomer1_->zoomBase();
    if ( horizontal ) {
        bz.setLeft( range.first );
        bz.setRight( range.second );
    } else {
        bz.setBottom( range.first );
        bz.setTop( range.second );
    }
    zoomer1_->setZoomBase();
}

bool
SpectrumWidget::scaleY( const QRectF& rc, std::pair< double, double >& left, std::pair< double, double >& right )
{
    using spectrumwidget::TraceData;
    bool hasYLeft( false ), hasYRight( false );
    
    left = right = std::make_pair( 0.0, 0.0 );

    for ( const TraceData& trace: impl_->traces_ ) {
        std::pair<double, double> y = trace.y_range( rc.left(), rc.right() );
        if ( trace.yRight() ) {
            if ( !hasYRight ) {
                hasYRight = true;
                right = std::make_pair( y.first, y.second );
            } else
                right = std::make_pair( std::min( y.first, right.first ), std::max( y.second, right.second ) );
        } else {
            if ( !hasYLeft ) {
                hasYLeft = true;
                left = std::make_pair( y.first, y.second );
            } else
                left = std::make_pair( std::min( y.first, left.first ), std::max( y.second, left.second ) );
        }
    }

    left.second = left.second + (left.second - left.first) * 0.12;
    right.second = right.second + (right.second - right.first) * 0.12;

    if ( hasYLeft && hasYRight ) {
        if ( ( left.first <= 0 && left.second > 0 ) && ( right.first <= 0 && right.second > 0 ) ) {
            // adjust zero level
            double left_base = left.first / ( left.second - left.first ); // should be negative
            double right_base = right.first / (right.second - right.first); // negative too
            if ( left_base < right_base ) { // left axis has higher zero position
                right.first = (right.second - right.first) * left_base;
            } else {
                left.first = (left.second - left.first) * right_base;
            }
        }
    }
    return hasYRight;
}

void
SpectrumWidget::override_zoom_rect( QRectF& rc )
{
    if ( autoYZoom_ ) {
        std::pair< double, double > left, right;
        bool hasYRight = scaleY( rc, left, right );
        // override
        rc.setBottom( left.first );
        rc.setTop( left.second );
        if ( hasYRight )
            setAxisScale( QwtPlot::yRight, right.first, right.second ); // immediate set
    }
}

void
SpectrumWidget::setKeepZoomed( bool value )
{
    keepZoomed_ = value;
}

void
SpectrumWidget::zoomed( const QRectF& rect )
{
    if ( autoYZoom_ ) {
        std::pair< double, double > left, right;
        if ( scaleY( rect, left, right ) ) { // has Y-right
            setAxisScale( QwtPlot::yRight, right.first, right.second );
            replot();
        }
    }
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
SpectrumWidget::setAutoAnnotation( bool enable )
{
    impl_->autoAnnotation_ = enable;
}

bool
SpectrumWidget::autoAnnotation() const
{
    return impl_->autoAnnotation_;
}

void
SpectrumWidget::setAxis( HorizontalAxis haxis, bool replot )
{
    haxis_ = haxis;
    impl_->isTimeAxis_ = haxis == HorizontalAxisTime;
    setAxisTitle(QwtPlot::xBottom, QwtText( haxis_ == HorizontalAxisMass ? "<i>m/z</i>" : "Time[&mu;s]", QwtText::RichText) );
    if ( replot ) {
        redraw_all();
    } else
        clear();
}

void
SpectrumWidget::removeData( int idx, bool bReplot )
{
    if ( idx < int(impl_->traces_.size()) ) {
        impl_->traces_[ idx ] = spectrumwidget::TraceData( idx );
        impl_->clear_annotations();
        if ( bReplot )
            replot();
    }
}

void
SpectrumWidget::redraw_all()
{
    QRectF rcLeft, rcRight;

    for ( auto& trace: impl_->traces_ )
        trace.redraw( *this, haxis_, rcLeft, rcRight );

    if ( !rcLeft.isNull() ) {
        setAxisScale( QwtPlot::xBottom, rcLeft.left(), rcLeft.right() );
        setAxisScale( QwtPlot::yLeft, rcLeft.bottom(), rcLeft.top() );
    }
    if ( !rcRight.isNull() ) {
        if ( rcLeft.isNull() )
            setAxisScale( QwtPlot::xBottom, rcRight.left(), rcRight.right() );
        setAxisScale( QwtPlot::yRight, rcRight.bottom(), rcRight.top() );
    }
    replot();
}

void
SpectrumWidget::setData( const std::shared_ptr< adcontrols::MassSpectrum >& ptr, int idx, bool yRight )
{
    using spectrumwidget::TraceData;

    while ( int( impl_->traces_.size() ) <= idx ) 
		impl_->traces_.push_back( TraceData( static_cast<int>(impl_->traces_.size()) ) );

    TraceData& trace = impl_->traces_[ idx ];

    QRectF rect;
    trace.setData( *this, ptr, rect, haxis_, yRight );

	QRectF z = zoomer1_->zoomRect(); // current (:= previous) zoom

    setAxisScale( QwtPlot::xBottom, rect.left(), rect.right() );
    std::pair< double, double > left, right;

    if ( scaleY( rect, left, right ) && yRight ) {
        setAxisScale( QwtPlot::yRight, right.first, right.second );
    }
    else {
        setAxisScale( QwtPlot::yLeft, left.first, left.second );
    }
    zoomer1_->setZoomBase();

    if ( axisHadScaled_ && keepZoomed_ )
        Dataplot::zoom( z ); // push previous rect

    axisHadScaled_ = true;

    if ( ptr->isCentroid() ) {
        impl_->centroid_ = ptr;
        update_annotation( false );
    }
}

void
SpectrumWidget::setFocusedFcn( int fcn )
{
    for ( auto& trace: impl_->traces_ )
		trace.setFocusedFcn( fcn );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

using namespace adwplot::spectrumwidget;

TraceData::~TraceData()
{
    curves_.clear();
}

void
TraceData::setProfileData( Dataplot& plot, const adcontrols::MassSpectrum& ms, const QRectF& rect, bool yRight )
{
    adcontrols::segment_wrapper< const adcontrols::MassSpectrum > segments( ms );
    int fcn = 0;
    for ( auto& seg: segments ) {
        auto ptr = std::make_shared< AdwPlotCurve >();
        ptr->attach( &plot );
        curves_.push_back( ptr );

        QColor color( color_table[ idx_ ] );
        ptr->setPen( color );
        ptr->setData( new xSeriesData( seg, rect, isTimeAxis_ ) );
        if ( yRight )
            ptr->setYAxis( QwtPlot::yRight );
        ++fcn;
    }
}

void
TraceData::changeFocus( int focusedFcn )
{
    int fcn = 0;
    for ( auto& curve: curves_ ) {
        QColor color( color_table[ idx_ ] );
        if ( focusedFcn >= 0 ) {
            if ( focusedFcn != fcn )
                color.setAlpha( 0x20 );
        }
        curve->setPen( color );
        ++fcn;
    }
}

void
TraceData::setCentroidData( Dataplot& plot, const adcontrols::MassSpectrum& _ms, const QRectF& rect, bool yRight )
{
    adcontrols::segment_wrapper< const adcontrols::MassSpectrum > segments( _ms );

    curves_.clear();

    for ( auto& seg: segments ) {
        if ( const unsigned char * colors = seg.getColorArray() ) {
            std::set< unsigned char > color;
            for ( size_t i = 0; i < seg.size(); ++i )
                color.insert( colors[i] );

            for ( const auto& c: color ) {
                xSeriesData * xp = new xSeriesData( seg, rect, isTimeAxis_ );
                if ( xp->make_color_index( c ) ) {
                    auto curve = std::make_shared< AdwPlotCurve >();
                    curve->attach( &plot );
                    curves_.push_back( curve );

                    curve->setData( xp );
                    curve->setPen( QPen( color_table[ c ] ) );
                    curve->setStyle( QwtPlotCurve::Sticks );
                    if ( yRight )
                        curve->setYAxis( QwtPlot::yRight );
                }
            }

        } else {
            auto curve = std::make_shared< AdwPlotCurve >();
            curve->attach( &plot );
            curves_.push_back( curve );

            QColor color( color_table[ 0 ] );
            color.setAlpha( 255 - (idx_ * 16) );
            curve->setPen( QPen( color ) );
            curve->setData( new xSeriesData( seg, rect, isTimeAxis_ ) );
            curve->setStyle( QwtPlotCurve::Sticks );
            if ( yRight )
                curve->setYAxis( QwtPlot::yRight );
        }
    }
}

void
TraceData::redraw( Dataplot& plot, SpectrumWidget::HorizontalAxis axis, QRectF& rcLeft, QRectF& rcRight )
{
    QRectF rect;
    if ( auto p = pSpectrum_ ) {
        setData( plot, p, rect, axis, yRight_ );

        QRectF& rc = (yRight_) ? rcRight : rcLeft;
        if ( rc.isNull() )
            rc = rect;
        else {
            rc.setBottom( std::min( rc.bottom(), rect.bottom() ) );
            rc.setTop( std::max( rc.top(), rect.bottom() ) );

            rc.setLeft( std::min( rc.left(), rect.left() ) );
            rc.setRight( std::max( rc.right(), rect.right() ) );
        }
    }
}

void
TraceData::setData( Dataplot& plot
                    , const std::shared_ptr< adcontrols::MassSpectrum >& ms
                    , QRectF& rect
                    , SpectrumWidget::HorizontalAxis haxis
                    , bool yRight )
{
    curves_.clear();

    if ( pSpectrum_ != ms )
        pSpectrum_ = ms;

    isTimeAxis_ = haxis == SpectrumWidget::HorizontalAxisTime;

	double top = adcontrols::segments_helper::max_intensity( *ms );
    double bottom = adcontrols::segments_helper::min_intensity( *ms );
	
	if ( ms->isCentroid() )
		bottom = 0;
	top = top + ( top - bottom ) * 0.12; // add 12% margine for annotation

    if ( isTimeAxis_ ) {

        adcontrols::segment_wrapper< const adcontrols::MassSpectrum > segments( *ms );
        std::pair< double, double > time_range = std::make_pair( std::numeric_limits<double>::max(), 0 );
        for ( auto& m: segments ) {
            std::pair< double, double > range = m.getMSProperty().instTimeRange();
            time_range.first = std::min( time_range.first, range.first );
            time_range.second = std::max( time_range.second, range.second );
        }

        // check if zero width
        if ( adportable::compare<double>::approximatelyEqual( time_range.first, time_range.second ) ) {
            time_range.first = ms->getTime( 0 );
            time_range.second = ms->getTime( ms->size() - 1 );
            if ( ms->isCentroid() || ms->size() == 0 ) {
                time_range.first = double( int( time_range.first * 10000 ) ) / 10000.0;  // round to 0.1us
                time_range.second = double( int( time_range.second * 10000 + 1 ) ) / 10000.0;
            }
        }

        using namespace adcontrols::metric;
        // setCoods( tof-left corner := (x1, y1) -- bottom-right corner := (x2, y2)
        // Origin of Qt's coordinate system is top,left (0,0) (opposit y-scale to GKS)
        rect.setCoords( scale_to_micro(time_range.first), top, scale_to_micro(time_range.second), bottom );

    } else {

        std::pair< double, double >& mass_range = ms->getAcquisitionMassRange();
        // check if zero width
        if ( adportable::compare<double>::approximatelyEqual( mass_range.first, mass_range.second ) ) {
            mass_range.first = double( int( ms->getMass(0) * 10 ) ) / 10.0;  // round to 0.1Da
            mass_range.second = double( int( ms->getMass( ms->size() - 1 ) * 10 + 1 ) ) / 10.0;
        }
        rect.setCoords( mass_range.first, top, mass_range.second, bottom );

    }
    rect_ = rect;
    yRight_ = yRight;
    if ( ms->isCentroid() ) { // sticked
        setCentroidData( plot, *ms, rect, yRight );
    } else { // Profile
        setProfileData( plot, *ms, rect, yRight );
    }
}

void
TraceData::setFocusedFcn( int fcn )
{
    if ( focusedFcn_ != fcn ) {
        focusedFcn_ = fcn;
        if ( auto p = pSpectrum_ ) {
            changeFocus( focusedFcn_ );
        }
    }
}


std::pair< double, double >
TraceData::y_range( double left, double right ) const
{
    double top = 100;
    double bottom = -10;

    // if ( const std::shared_ptr< adcontrols::MassSpectrum > ms = pSpectrum_.lock() ) {
    if ( auto ms = pSpectrum_ ) {
        adcontrols::segment_wrapper< const adcontrols::MassSpectrum > segments( *ms );
        bool isCentroid = ms->isCentroid();
        for ( auto& seg: segments ) {

            if ( seg.size() == 0 )
                continue;

            size_t idleft(0), idright(0);

            if ( isTimeAxis_ ) {
                using namespace adcontrols::metric;
                double uleft = scale_to_base(left, micro);
                double uright = scale_to_base(right, micro);

                if ( isCentroid ) {
                    if ( ms->isCentroid() ) {
                        const double * x = seg.getTimeArray();
                        idleft = std::distance( x, std::lower_bound( x, x + seg.size(), uleft ) );
                        idright = std::distance( x, std::lower_bound( x, x + seg.size(), uright ) );
                    }
                } else {
                    // std::pair< double, double > range = seg.getMSProperty().instTimeRange();
                    
                    struct X {
                        static uint32_t index( double interval, uint32_t delay, uint32_t nSamples, double t ) {
                            uint32_t idx = uint32_t( t / interval + 0.5 );
                            if ( idx < delay )
                                return 0;
                            idx -= delay;
                            if ( idx >= nSamples )
                                return nSamples - 1;
                            return idx;
                        }
                    };
                    const adcontrols::MSProperty::SamplingInfo& info = seg.getMSProperty().getSamplingInfo();
                    idleft = X::index( info.fSampInterval(), info.nSamplingDelay, info.nSamples, uleft );
                    idright = X::index( info.fSampInterval(), info.nSamplingDelay, info.nSamples, uright );
                }
            } else {
                // mass axis
				const double * x = seg.getMassArray();
                idleft = std::distance( x, std::lower_bound( x, x + seg.size(), left ) );
                idright = std::distance( x, std::lower_bound( x, x + seg.size(), right ) );
            }

            if ( idleft )
                --idleft;

            if ( idleft >= ms->size() )
                idleft = 0;

            if ( idright >= ms->size() && ms->size() )
                idright = ms->size() - 1;
            
            if ( idleft < idright ) {
                const double * y = seg.getIntensityArray();
                
                auto minmax = std::minmax_element( y + idleft, y + idright );
                double min = *minmax.first;
                double max = *minmax.second;

                top = std::max( top, max );
                if ( ! isCentroid ) {
                    bottom = min - (max - min) / 25;
                } else {
                    bottom = -10;
                }
            }
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
    using namespace adcontrols::metric;        

    // adportable::scoped_debug<> scope( __FILE__, __LINE__ ); scope << "update_annotateion:";
    plot.setUpdatesEnabled( false );

	// QRectF zrc = plot.zoomRect();

    typedef std::tuple< size_t, size_t, int, double, double > peak; // fcn, idx, color, mass, intensity
    enum { c_fcn, c_idx, c_color, c_intensity, c_mass };

    if ( auto centroid = centroid_.lock() ) {

        std::vector< peak > peaks;
        adcontrols::segment_wrapper< const adcontrols::MassSpectrum > segments( *centroid );
        
        adcontrols::annotations auto_annotations;
        adcontrols::annotations annotations;

        double max_y = adcontrols::segments_helper::max_intensity( *centroid );
        
        for ( size_t fcn = 0; fcn < segments.size(); ++fcn ) {
            const adcontrols::MassSpectrum& ms = segments[ fcn ];
            const unsigned char * colors = ms.getColorArray();

            size_t beg, end;
            if ( isTimeAxis_ ) {
                array_wrapper< const double > times( ms.getTimeArray(), ms.size() );
                beg = std::distance( times.begin(), std::lower_bound( times.begin(), times.end(), scale_to_base( range.first, micro ) ) );
                end = std::distance( times.begin(), std::lower_bound( times.begin(), times.end(), scale_to_base( range.second, micro) ) );
            } else {
                array_wrapper< const double > masses( ms.getMassArray(), ms.size() );
                beg = std::distance( masses.begin(), std::lower_bound( masses.begin(), masses.end(), range.first ) );
                end = std::distance( masses.begin(), std::lower_bound( masses.begin(), masses.end(), range.second ) );
            }
            
            if ( beg < end ) {
                const adcontrols::annotations& attached = ms.get_annotations();
                std::map< size_t, adcontrols::annotations > marge;
                for ( auto a : attached ) {
                    if ( ( int(beg) <= a.index() && a.index() <= int(end) ) || ( range.first < a.x() && a.x() < range.second ) ) {
                        if ( a.index() >= 0 ) {
							if ( isTimeAxis_ ) {
								a.x( scale_to_micro( ms.getTime( a.index() ) ) );
							} else {
								a.x( ms.getMass( a.index() ) );
							}
							a.y( ms.getIntensity( a.index() ) );
						}
						if ( a.dataFormat() == adcontrols::annotation::dataFormula ) {
							a.text( adcontrols::ChemicalFormula::formatFormulae( a.text () ), adcontrols::annotation::dataFormula );
                        }
                        marge[ a.index() ] << a;
                    }
                }
                for ( auto& a : marge ) {
                    // if more than two annotations attached to an index, formula is a priority on spectrum
                    auto it = std::find_if( a.second.begin(), a.second.end(), [] ( const adcontrols::annotation& x ) { return x.dataFormat() == adcontrols::annotation::dataFormula; } );
                    if ( it == a.second.end() )
                        it = std::find_if( a.second.begin(), a.second.end(), [] ( const adcontrols::annotation& x ) { return x.dataFormat() == adcontrols::annotation::dataText; } );
                    if ( it != a.second.end() )
                        annotations << *it;
                }

                if ( autoAnnotation_ ) {
                    // generate auto-annotation
                    for ( size_t idx = beg; idx <= end; ++idx ) {
                        if ( std::find_if( attached.begin()
                                           , attached.end()
                                           , [idx]( const adcontrols::annotation& a ){ return a.index() == int(idx); } ) == attached.end() ) {
                            int pri = ms.getIntensity( idx ) / max_y * 1000;
                            if ( colors )
                                pri *= 100;
                            if ( isTimeAxis_ ) {
                                double microseconds = adcontrols::metric::scale_to_micro( ms.getTime( idx ) );
                                adcontrols::annotation annot( ( boost::wformat( L"%.4lf" ) % microseconds ).str()
                                                              , microseconds, ms.getIntensity( idx )
                                                              , int( fcn << 24 | idx ), pri );
                                auto_annotations << annot;
                            } else {
                                adcontrols::annotation annot( ( boost::wformat( L"%.4lf" ) % ms.getMass( idx ) ).str()
                                                              , ms.getMass( idx ), ms.getIntensity( idx ), int( fcn << 24 | idx ), pri );
                                auto_annotations << annot;
                            }
                        }
                    }
                }
            }
        }
        
        auto_annotations.sort();
        annotations.sort();
        
        annotations_.clear();
        Annotations annots(plot, annotations_);
        
        for ( const auto& a: annotations ) {
            QwtText text( QString::fromStdString(a.text()), QwtText::RichText);
            text.setColor( Qt::darkGreen );
            text.setFont( Annotation::font() );
            annots.insert( a.x(), a.y(), text, Qt::AlignTop | Qt::AlignHCenter );
        }
        
        QColor color = Qt::darkGreen;
        QFont font = Annotation::font();
        if ( ! annotations.empty() ) {
            // if user defined annotations were exist
            font.setPointSize( font.pointSize() - 2 );
            color = Qt::gray;
        }
        for ( const auto& a: auto_annotations ) {
            QwtText text( QString::fromStdString(a.text()), QwtText::RichText );
            text.setColor( color );
            text.setFont( font );
			annots.insert( a.x(), a.y(), text, Qt::AlignTop | Qt::AlignHCenter );
        }
    }
    plot.setUpdatesEnabled( true );
}

void
SpectrumWidgetImpl::clear()
{
    centroid_.reset();
    annotations_.clear();
    if ( !traces_.empty() ) {
        traces_.clear();
    }
}

QwtText
SpectrumWidgetImpl::tracker1( const QPointF& pos )
{
    if ( isTimeAxis_ ) {
        return QwtText( (boost::format( "%.4f&mu;s" ) % pos.x()).str().c_str(), QwtText::RichText );
    }
    else {
        return QwtText( (boost::format( "<i>m/z=</i>%.4f" ) % pos.x()).str().c_str(), QwtText::RichText );
    }
}

QwtText
SpectrumWidgetImpl::tracker2( const QPointF& p1, const QPointF& pos )
{
    double d = ( pos.x() - p1.x() );

    if ( isTimeAxis_ ) {
        if ( std::abs( d ) < 1.0 )
            return QwtText( (boost::format( "%.4f&mu;s (&delta;=%gns)" ) % pos.x() % (d * 1000)).str().c_str(), QwtText::RichText );
        else
            return QwtText( (boost::format( "%.4f&mu;s (&delta;=%g&mu;s)" ) % pos.x() % d).str().c_str(), QwtText::RichText );
    }
    else {
        if ( std::abs( d ) < 1.0 )
            return QwtText( (boost::format( "<i>m/z=</i>%.4f (&delta;=%gmDa)" ) % pos.x() % (d * 1000)).str().c_str(), QwtText::RichText );
        else
            return QwtText( (boost::format( "<i>m/z=</i>%.4f (&delta;=%gDa)" ) % pos.x() % d).str().c_str(), QwtText::RichText );
    }
}
