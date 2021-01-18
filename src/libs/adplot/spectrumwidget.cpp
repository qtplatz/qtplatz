// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-     Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2018 MS-Cheminformatics LLC
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
#include "adplotcurve.hpp"
#include "annotation.hpp"
#include "annotations.hpp"
#include <adcontrols/annotation.hpp>
#include <adcontrols/annotations.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/samplinginfo.hpp>
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
#include <QDebug>
#include <QSignalBlocker>
#include <boost/format.hpp>
#include <atomic>
#include <mutex>
#include <set>

using namespace adplot;

namespace adplot {

    namespace spectrumwidget {

        static QColor color_table [] = {
            QColor( 0x00, 0x00, 0xff )    // 0  blue
            , QColor( 0xff, 0x00, 0x00 )  // 1  red
            , QColor( 0x00, 0x80, 0x00 )  // 2  green
            , QColor( 0x94, 0x00, 0xd3 )  // 3  dark violet
            , QColor( 0xff, 0x14, 0x93 )  // 4  deep pink
            , QColor( 0x80, 0x00, 0x80 )  // 5  purple
            , QColor( 0x4b, 0x00, 0x82 )  // 6  indigo
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
                if ( ! indices_.empty() )
                    return indices_.size();
                return ms_.size();
            }

            QPointF sample( size_t idx ) const override {
                if ( ! indices_.empty() && idx < indices_.size() ) // if colored
                    idx = indices_[ idx ];
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
                        indices_.push_back( i );
                }
                return indices_.size();
            }

        private:
            QRectF rect_;
            const adcontrols::MassSpectrum& ms_;
            std::vector< size_t > indices_; // if centroid with color,
            bool axisTime_;
        };

        class TraceData {
        public:
            TraceData( int idx ) : idx_( idx )
                                 , focusedFcn_( -1 )
                                 , alpha_( 255 )
                                 , yRight_( false )  {

                color_ = color_table[ idx % ( sizeof( color_table ) / sizeof( color_table[ 0 ] ) ) ];

            }

            TraceData( const TraceData& t ) : idx_( t.idx_ )
                                            , focusedFcn_( t.focusedFcn_ )
                                            , alpha_( t.alpha_ )
                                            , rect_( t.rect_ )
                                            , yRight_( t.yRight_ )
                                            , color_( t.color_ )
                                            , curves_( t.curves_ )
                                            , pSpectrum_( t.pSpectrum_ )
                                            , isTimeAxis_( t.isTimeAxis_ ) {
            }

            ~TraceData();
            void setData( plot& plot
                          , std::shared_ptr< const adcontrols::MassSpectrum>&
                          , QRectF&, SpectrumWidget::HorizontalAxis, bool yRight );
            void redraw( plot& plot, SpectrumWidget::HorizontalAxis, QRectF&, QRectF& );
            void setFocusedFcn( int fcn );
            std::pair<double, double> y_range( double left, double right, int fcn ) const;
            bool yRight() const { return yRight_; }
            void setAlpha( int alpha );
            void setColor( const QColor& );
            const QRectF& rect() const { return rect_; }

        private:
            void setProfileData( plot& plot, const adcontrols::MassSpectrum& ms, const QRectF&, bool yRight );
            void setCentroidData( plot& plot, const adcontrols::MassSpectrum& ms, const QRectF&, bool yRight );
            void changeFocus( int focusedFcn );

            int idx_;
            int focusedFcn_;
            uint8_t alpha_;
            QRectF rect_;
            bool yRight_;
            QColor color_;
        public:
            std::vector< std::shared_ptr< adPlotCurve > > curves_;
            std::shared_ptr< const adcontrols::MassSpectrum > pSpectrum_;
        private:
			bool isTimeAxis_;
        };

    } // namespace spectrumwidget

    ///////////////////////////////////

    class SpectrumWidget::impl {
    public:
        impl() : autoAnnotation_( true )
               , isTimeAxis_( false )
               , keepZoomed_( true )
               , haxis_( HorizontalAxisMass )
               , focusedFcn_( -1 ) // no focus
               , scaleFcn_( -1 )
               , yScale1_{0,0}
               , hasYScale1_( false )
            {}
        bool autoAnnotation_;
        bool isTimeAxis_;
        // std::weak_ptr< const adcontrols::MassSpectrum > centroid_;  // for annotation
        std::weak_ptr< const adcontrols::MassSpectrum > ms_;  // for annotation
        std::vector< Annotation > annotations_;
        std::vector< std::unique_ptr< spectrumwidget::TraceData > > traces_;

        std::atomic<bool> keepZoomed_;
        std::atomic<HorizontalAxis> haxis_;
        std::atomic<int> focusedFcn_;
        int scaleFcn_;
        std::mutex mutex_;
        std::pair< double, double > yScale1_;
        bool hasYScale1_;

        void clear();
        void update_annotations( plot&, const std::pair<double, double>& );
        void set_annotations( const adcontrols::annotations& );
		void clear_annotations();

        void handleZoomRect( QRectF& );
        QwtText tracker1( const QPointF& );
        QwtText tracker2( const QPointF&, const QPointF& );

        boost::optional< std::pair< double, double > > scaleY( const QRectF&, QwtPlot::Axis ) const;

        std::pair<bool,bool> scaleY( const QRectF&, std::pair< double, double >& left, std::pair< double, double >& right );
        void baseScale( bool, QRectF& rc );
    };

} // namespace adplot


QColor
SpectrumWidget::index_color( unsigned int idx )
{
    using namespace adplot::spectrumwidget;
    idx = idx % ( sizeof( color_table ) / sizeof( color_table[ 0 ] ) - 1); // subtract for transparent [17]
    return color_table[ idx ];
}

SpectrumWidget::~SpectrumWidget()
{
    delete impl_;
}

SpectrumWidget::SpectrumWidget(QWidget *parent) : plot(parent)
                                                , impl_( new SpectrumWidget::impl )
                                                , viewid_( 0 )
{
    if ( auto zoomer = plot::zoomer() ) {

        zoomer->autoYScale( true );

        // using namespace std::placeholders;
        zoomer->tracker1( std::bind( &SpectrumWidget::impl::tracker1, impl_, std::placeholders::_1 ) );
        zoomer->tracker2( std::bind( &SpectrumWidget::impl::tracker2, impl_, std::placeholders::_1, std::placeholders::_2 ) );

        zoomer->autoYScaleHock( [this]( QRectF& rc ){ yScaleHock( rc ); } );

        connect( zoomer, &QwtPlotZoomer::zoomed, this, &SpectrumWidget::zoomed );
    }

    if ( auto picker = plot::picker() ) {
        connect( picker, SIGNAL( moved( const QPointF& ) ), this, SLOT( moved( const QPointF& ) ) );
        connect( picker, SIGNAL( selected( const QRectF& ) ), this, SLOT( selected( const QRectF& ) ) );
        picker->setEnabled( true );
    }

    setAxisTitle( QwtPlot::xBottom, QwtText( "<i>m/z</i>", QwtText::RichText ) );
    setAxisTitle( QwtPlot::yLeft, QwtText( "Intensity" ) );
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
SpectrumWidget::yScaleHock( QRectF& rc )
{
    std::pair<double, double > left, right;

    auto hasAxis = impl_->scaleY( rc, left, right );
    if ( hasAxis.second ) {
        if ( ! adportable::compare<double>::approximatelyEqual( right.first, right.second ) )
            setAxisScale( QwtPlot::yRight, right.first, right.second ); // set yRight
    }

    if ( hasAxis.first )
        rc.setCoords( rc.left(), left.first, rc.right(), left.second );
}

void
SpectrumWidget::yZoom( double xmin, double xmax )
{
    std::pair<double, double > left, right;
    QRectF rc( QPointF( xmin, 0 ), QPointF( xmax, 0 ) );

    auto hasAxis = impl_->scaleY( rc, left, right );
    if ( hasAxis.second && ! adportable::compare<double>::approximatelyEqual( right.first, right.second ) ) {
        setAxisScale( QwtPlot::yRight, right.first, right.second ); // set yRight
    }

    ADDEBUG() << "******************* yZoom **************************";
    if ( hasAxis.first && ! adportable::compare<double>::approximatelyEqual( left.first, left.second ) ) {
        setAxisScale( QwtPlot::yLeft, left.first, left.second ); // set yLeft
    }
    replot();
}

void
SpectrumWidget::setZoomBase( const std::pair< double, double >& range, bool horizontal )
{
    QRectF bz = plot::zoomer()->zoomBase();
    if ( horizontal ) {
        bz.setLeft( range.first );
        bz.setRight( range.second );
    } else {
        bz.setBottom( range.first );
        bz.setTop( range.second );
    }
    zoom( bz );
    plot::zoomer()->setZoomBase();
}

void
SpectrumWidget::setZoomStack( const QRectF& rc )
{
    zoom( rc );
    plot::zoomer()->setZoomBase();
    QStack< QRectF > zstack;
    zstack.push_back( rc );
    zstack.push_back( rc );
    QSignalBlocker block( zoomer() );
    zoomer()->setZoomStack( zstack );
}

void
SpectrumWidget::setVectorCompression( int compression )
{
    plot::setVectorCompression( compression );
    for ( auto& trace : impl_->traces_ ) {
        if ( trace ) {
            for ( auto& curve: trace->curves_ )
                curve->setVectorCompression( compression );
        }
    }
}

boost::optional< std::pair< double, double > >
SpectrumWidget::impl::scaleY( const QRectF& rc, QwtPlot::Axis axisId ) const
{
    using spectrumwidget::TraceData;

    const bool yRight = axisId == QwtPlot::yRight;

    boost::optional< std::pair< double, double > > range{ boost::none };

    for ( const auto& trace: traces_ ) {
        if ( trace && (trace->yRight() == yRight) ) {
            auto y = trace->y_range( rc.left(), rc.right(), scaleFcn_ );
            range = range ? std::make_pair( std::min( y.first, range->first ), std::max( y.second, range->second ) ) : y;
        }
    }
    return range;
}

std::pair<bool, bool>
SpectrumWidget::impl::scaleY( const QRectF& rc, std::pair< double, double >& left, std::pair< double, double >& right )
{
    using spectrumwidget::TraceData;
    bool hasYLeft( false ), hasYRight( false );

    left = right = std::make_pair( std::numeric_limits<double>::max(), std::numeric_limits<double>::lowest() );

    for ( const auto& trace: traces_ ) {
        if ( trace ) {
            std::pair<double, double> y = trace->y_range( rc.left(), rc.right(), scaleFcn_ );
            if ( trace->yRight() ) {
                hasYRight = true;
                right = std::make_pair( std::min( y.first, right.first ), std::max( y.second, right.second ) );
            } else {
                hasYLeft = true;
                left = std::make_pair( std::min( y.first, left.first ), std::max( y.second, left.second ) );
            }
        }
    }
    if ( ! hasYLeft )
        left = std::make_pair( -5.0, 100 );

    if ( hasYRight )
        right.second = right.second + (right.second - right.first) * 0.12;

    return std::make_pair( hasYLeft, hasYRight );
}

void
SpectrumWidget::impl::baseScale( bool yRight, QRectF& rc )
{
    using spectrumwidget::TraceData;

    rc = QRectF(); // make them 'null'

    for ( const auto& trace: traces_ ) {

        if ( !trace )
            continue;

        const QRectF& t = trace->rect();

        if ( rc.isNull() ) {

            rc = t;

        } else {

            rc.setLeft( std::min( t.left(), rc.left() ) );
            rc.setRight( std::max( t.right(), rc.right() ) );

            if ( trace->yRight() == yRight ) {

                rc.setBottom( std::min( t.bottom(), rc.bottom() ) );
                rc.setTop( std::max( t.top(), rc.top() ) );

            }
        }
    }
}

void
SpectrumWidget::setKeepZoomed( bool value )
{
    impl_->keepZoomed_ = value;
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
    emit onSelected( QRectF( pos, pos ) );
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

SpectrumWidget::HorizontalAxis
SpectrumWidget::axis() const
{
    return impl_->haxis_;
}

void
SpectrumWidget::setAxis( HorizontalAxis axis, bool replot
                         , std::function< QRectF( const QRectF&, const adcontrols::MassSpectrum&, HorizontalAxis ) > axisConverter )
{
    if ( impl_->haxis_ == axis )
        return; // nothing to be done

    // on trial -- it may be a problem when various number of laps protocols are combined.
    QRectF zRect;

    if ( zoomer()->zoomRectIndex() > 0 && axisConverter ) {
        auto it = std::find_if( impl_->traces_.begin(), impl_->traces_.end()
                                , [&]( std::unique_ptr< spectrumwidget::TraceData >& trace ){ return trace && trace->pSpectrum_; } );
        if ( it != impl_->traces_.end() ) {
            auto ms( (*it)->pSpectrum_ );
            if ( it != impl_->traces_.end() )
                zRect = axisConverter( zoomer()->zoomRect(), *ms, impl_->haxis_ );
        }
    }

    impl_->haxis_ = axis;
    impl_->isTimeAxis_ = axis == HorizontalAxisTime;
    setAxisTitle(QwtPlot::xBottom, QwtText( impl_->haxis_ == HorizontalAxisMass ? "<i>m/z</i>" : "Time[&mu;s]", QwtText::RichText) );

    if ( replot ) {
        redraw_all();
        zoomer()->setZoomBase();
        if ( zRect.isValid() )
            zoomer()->zoom( zRect );
    } else {
        clear();
    }
}

void
SpectrumWidget::removeData( int idx, bool bReplot )
{
    if ( idx < int(impl_->traces_.size()) ) {
        impl_->traces_[ idx ].reset();
        impl_->clear_annotations();
        if ( bReplot )
            replot();
    }
}

void
SpectrumWidget::redraw_all( bool keepX )
{
    QRectF rcLeft, rcRight;

    for ( auto& trace: impl_->traces_ ) {
        if ( trace )
            trace->redraw( *this, impl_->haxis_, rcLeft, rcRight );
    }

    if ( !rcLeft.isNull() ) {
        if ( ! keepX )
            setAxisScale( QwtPlot::xBottom, rcLeft.left(), rcLeft.right() );
        setAxisScale( QwtPlot::yLeft, rcLeft.bottom(), rcLeft.top() );
    }

    if ( !rcRight.isNull() ) {
        if ( ! keepX && rcLeft.isNull() )
            setAxisScale( QwtPlot::xBottom, rcRight.left(), rcRight.right() );
        setAxisScale( QwtPlot::yRight, rcRight.bottom(), rcRight.top() );
    }
    replot();
}

void
SpectrumWidget::setAlpha( int idx, int alpha )
{
    if ( impl_->traces_.size() > idx ) {
        if ( auto& trace = impl_->traces_[ idx ] )
            trace->setAlpha( alpha );
    }
}

void
SpectrumWidget::setColor( int idx, const QColor& color )
{
    if ( impl_->traces_.size() > idx ) {
        if ( auto& trace = impl_->traces_[ idx ] )
            trace->setColor( color );
    }
}

void
SpectrumWidget::setData( std::shared_ptr< const adcontrols::MassSpectrum > ptr, int idx, bool yRight )
{
    using spectrumwidget::TraceData;

    impl_->scaleFcn_ = (-1);

    if ( !ptr || ptr->size() == 0 ) {
        removeData( idx );
        return;
    }

    if ( ptr->isHistogram() ) {
        setAxisTitle( yRight ? QwtPlot::yRight : QwtPlot::yLeft, QwtText( "Counts" ) );
    } else {
        auto label = ptr->isCentroid() ? QwtText( "Intensity" ) : QwtText( "Intensity (mV)" );
        setAxisTitle( yRight ? QwtPlot::yRight : QwtPlot::yLeft, label );
    }

    if ( impl_->traces_.size() <= idx )
        impl_->traces_.resize( idx + 1 );

    if ( ! impl_->traces_[ idx ] )
        impl_->traces_[ idx ] = std::make_unique< TraceData >( idx );

    auto& trace = impl_->traces_[ idx ];

    do {
        QRectF rect;
        trace->setData( *this, ptr, rect, impl_->haxis_, yRight ); // clear canvas if ptr == nullptr
    } while ( 0 );

    QRectF baseRect;
    impl_->baseScale( yRight, baseRect );

    if ( impl_->hasYScale1_ ) {
        baseRect.setTop( impl_->yScale1_.first );
        baseRect.setBottom( impl_->yScale1_.second );
    }

    auto rectIndex = zoomer()->zoomRectIndex();

    if ( ( yRight == false ) && ( rectIndex == 0 || !impl_->keepZoomed_ ) ) {

        setAxisScale( yRight ? QwtPlot::yRight : QwtPlot::yLeft, baseRect.bottom(), baseRect.top() );
        setAxisScale( QwtPlot::xBottom, baseRect.left(), baseRect.right() );
        zoomer()->setZoomBase();

    } else {
        QRectF z = zoomer()->zoomRect(); // current

        std::pair<double, double> left, right;
        auto hasAxis = impl_->scaleY( z, left, right );

        if ( yRight && hasAxis.second ) {

            setAxisScale( QwtPlot::yRight, right.first, right.second );

        } else {
            if ( hasAxis.first ) {
                QStack< QRectF > zstack;
                zstack.push_back( QRectF( baseRect.x(), baseRect.bottom(), baseRect.width(), -baseRect.height() ) ); // upside down
                if ( impl_->hasYScale1_ )
                    zstack.push_back( QRectF( z.x(), impl_->yScale1_.second, z.width(), impl_->yScale1_.first - impl_->yScale1_.second ) );
                else
                    zstack.push_back( QRectF( z.x(), left.first, z.width(), left.second - left.first ) );
                QSignalBlocker block( zoomer() );
                zoomer()->setZoomStack( zstack );
            }
        }
    }

    if ( ptr && ptr->isCentroid() && !ptr->isHistogram() ) {
        impl_->ms_ = ptr;
        update_annotation( false );
    } else {
        impl_->ms_ = ptr;
        update_annotation( false );
    }
    replot();
}


void
SpectrumWidget::setFocusedFcn( int fcn )
{
    for ( auto& trace: impl_->traces_ ) {
        if ( trace )
            trace->setFocusedFcn( fcn );
    }
}

void
SpectrumWidget::rescaleY( int fcn )
{
    impl_->scaleFcn_ = fcn;
    QRectF z = zoomer()->zoomRect(); // current
    yZoom( z.x(), z.x() + z.width() );
}

std::tuple< bool, double, double >
SpectrumWidget::yScale( bool axisRight ) const
{
    if ( axisRight ) {
        return std::make_tuple( false, 0.0, 0.0 );
    } else {
        return std::make_tuple( impl_->hasYScale1_, impl_->yScale1_.first, impl_->yScale1_.second );
    }
}

void
SpectrumWidget::setYScale( double top, double bottom, bool axisRight )
{
    if ( axisRight == false ) {

        impl_->yScale1_ = std::make_pair(top, bottom);
        impl_->hasYScale1_ = !adportable::compare< double >::essentiallyEqual( top, bottom );

        if ( auto zoomer = plot::zoomer() )
            zoomer->autoYScale( !impl_->hasYScale1_ );
    }
}

void
SpectrumWidget::replotYScale()
{
    auto rectIndex = zoomer()->zoomRectIndex();

    QRectF baseRect;
    impl_->baseScale( false, baseRect ); // always get left-Y axis
    if ( impl_->hasYScale1_ ) {
        baseRect.setTop( impl_->yScale1_.first );
        baseRect.setBottom( impl_->yScale1_.second );
    }

    if ( rectIndex == 0 || !impl_->keepZoomed_ ) {
        setAxisScale( QwtPlot::yLeft, baseRect.bottom(), baseRect.top() );
        setAxisScale( QwtPlot::xBottom, baseRect.left(), baseRect.right() );
        zoomer()->setZoomBase();
    } else {
        QRectF z = zoomer()->zoomRect(); // current zoom rect
        // std::pair<double, double> left, right;
        QStack< QRectF > zstack;
        zstack.push_back( QRectF( baseRect.x(), baseRect.bottom(), baseRect.width(), -baseRect.height() ) ); // upside down
        if ( impl_->hasYScale1_ ) {
            zstack.push_back( QRectF( z.x(), impl_->yScale1_.second, z.width(), impl_->yScale1_.first - impl_->yScale1_.second ) );
        } else {
            if ( auto yLeft = impl_->scaleY( z, QwtPlot::yLeft ) )
                zstack.push_back( QRectF( z.x(), yLeft->first, z.width(), yLeft->second - yLeft->first ) );
        }
        QSignalBlocker block( zoomer() );
        zoomer()->setZoomStack( zstack );
    }
    replot();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

using namespace adplot::spectrumwidget;

TraceData::~TraceData()
{
    curves_.clear();
}

void
TraceData::setProfileData( plot& plot, const adcontrols::MassSpectrum& ms, const QRectF& rect, bool yRight )
{
    adcontrols::segment_wrapper< const adcontrols::MassSpectrum > segments( ms );

    int fcn = 0;
    for ( auto& seg: segments ) {

        auto ptr = std::make_shared< adPlotCurve >();
        ptr->attach( &plot );
        curves_.push_back( ptr );

        int cid = ( idx_ + fcn ) % ( sizeof(color_table)/sizeof(color_table[0]) );
        QColor color( color_table[ cid ] );
        color.setAlpha( alpha_ );
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
    return;
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
TraceData::setCentroidData( plot& plot, const adcontrols::MassSpectrum& _ms, const QRectF& rect, bool yRight )
{
    curves_.clear();

    for ( auto& seg: adcontrols::segment_wrapper< const adcontrols::MassSpectrum >( _ms ) ) {
        if ( const unsigned char * colors = seg.getColorArray() ) {
            std::set< unsigned char > color;
            for ( size_t i = 0; i < seg.size(); ++i )
                color.insert( colors[i] );

            for ( const auto& c: color ) {
                xSeriesData * xp = new xSeriesData( seg, rect, isTimeAxis_ );
                if ( xp->make_color_index( c ) ) {
                    auto curve = std::make_shared< adPlotCurve >();
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
            auto curve = std::make_shared< adPlotCurve >();
            curve->attach( &plot );
            curves_.push_back( curve );

            int cid = ( idx_ ) % ( sizeof( color_table ) / sizeof( color_table[ 0 ] ) );
            QColor color( color_table[ cid ] );
            if ( alpha_ == 255 )
                color.setAlpha( 255 - ( idx_ * 16 ) ); // if user not specified alpha, graduation by trace id
            else
                color.setAlpha( alpha_ ); // if user speicified, set it (modified at 2018-06-8)
            curve->setPen( QPen( color ) );
            curve->setData( new xSeriesData( seg, rect, isTimeAxis_ ) );
            curve->setStyle( QwtPlotCurve::Sticks );
            if ( yRight )
                curve->setYAxis( QwtPlot::yRight );
        }
    }
}

void
TraceData::redraw( plot& plot, SpectrumWidget::HorizontalAxis axis, QRectF& rcLeft, QRectF& rcRight )
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
TraceData::setData( plot& plot
                    , std::shared_ptr< const adcontrols::MassSpectrum >& ms
                    , QRectF& rect
                    , SpectrumWidget::HorizontalAxis haxis
                    , bool yRight )
{
    curves_.clear();

    if ( !ms ) {
        pSpectrum_.reset();
        return;
    }

    if ( pSpectrum_ != ms )
        pSpectrum_ = ms;

    isTimeAxis_ = haxis == SpectrumWidget::HorizontalAxisTime;

	double top = adcontrols::segments_helper::max_intensity( *ms );
    double bottom = adcontrols::segments_helper::min_intensity( *ms );

	top = top + ( top - bottom ) * 0.12; // add 12% margine for annotation

    if ( adportable::compare< double >::essentiallyEqual( top, bottom ) )
        top = 1.0;

    if ( isTimeAxis_ ) {

        adcontrols::segment_wrapper< const adcontrols::MassSpectrum > segments( *ms );
        std::pair< double, double > time_range = std::make_pair( std::numeric_limits<double>::max(), 0 );
        for ( auto& m: segments ) {
            std::pair< double, double > range = m.getMSProperty().instTimeRange();
            time_range.first = std::min( time_range.first, range.first );
            time_range.second = std::max( time_range.second, range.second );
        }

        // check if zero width
        if ( adportable::compare<double>::approximatelyEqual( time_range.first, time_range.second ) && ms->size() ) {
            time_range.first = ms->getTime( 0 );
            time_range.second = ms->getTime( ms->size() - 1 );
            if ( ms->isCentroid() || ms->size() == 0 ) {
                time_range.first = double( int( time_range.first * 10000 ) ) / 10000.0;  // round to 0.1us
                time_range.second = double( int( time_range.second * 10000 + 1 ) ) / 10000.0;
            }
        }

        using namespace adcontrols::metric;
        // Origin of Qt's coordinate system is top,left (0,0) (opposit y-scale to GKS)
        rect.setCoords( scale_to_micro( time_range.first ), top, scale_to_micro( time_range.second ), bottom );

    } else {

        std::pair< double, double > mass_range = ms->getAcquisitionMassRange();
        // check if zero width
        if ( adportable::compare<double>::approximatelyEqual( mass_range.first, mass_range.second ) && ms->size() ) {
            mass_range.first = double( int( ms->getMass(0) * 10 ) ) / 10.0;  // round to 0.1Da
            mass_range.second = double( int( ms->getMass( ms->size() - 1 ) * 10 + 1 ) ) / 10.0;
        }
        rect.setCoords( mass_range.first, top, mass_range.second, bottom );
    }

    yRight_ = yRight;
    rect_ = rect;
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

void
TraceData::setAlpha( int alpha )
{
    if ( alpha != alpha_ ) {
        alpha_ = alpha;
        int fcn = 0;
        for ( auto& curve: curves_ ) {
            color_.setAlpha( alpha );
            curve->setPen( color_ );
            ++fcn;
        }
    }
}

void
TraceData::setColor( const QColor& color )
{
    color_ = color;
    for ( auto& curve: curves_ )
        curve->setPen( color_ );
}

std::pair< double, double >
TraceData::y_range( double left, double right, int fcn ) const
{
    namespace metric = adcontrols::metric;
    double top = std::numeric_limits<double>::lowest();
    double bottom = 0;
    double xleft = isTimeAxis_ ? metric::scale_to_base( left, metric::micro ) : left;
    double xright = isTimeAxis_ ? metric::scale_to_base( right, metric::micro ) : right;

    if ( pSpectrum_ ) {

        adcontrols::segment_wrapper< const adcontrols::MassSpectrum > segments( *pSpectrum_ );

        int _fcn(0);

        for ( auto& seg: segments ) {

            if ( fcn >= 0 && _fcn++ != fcn )
                continue;

            bool isCentroid = seg.isCentroid();

            if ( seg.size() == 0 )
                continue;
            std::pair<double, double> range = isTimeAxis_ ?
                std::make_pair( seg.getTime( 0 ), seg.getTime( seg.size() - 1 ) ) :
                std::make_pair( seg.getMass( 0 ), seg.getMass( seg.size() - 1 ) );

            if ( xright < range.first || range.second < xleft )
                continue;

            size_t idleft( 0 ), idright( 0 );

            if ( isTimeAxis_ ) {
                if ( const double * t = seg.getTimeArray() ) {
                    idleft = std::distance( t, std::lower_bound( t, t + seg.size(), xleft ) );
                    idright = std::distance( t, std::lower_bound( t, t + seg.size(), xright ) );
                } else {
                    idleft  = adcontrols::MSProperty::toIndex( xleft, seg.getMSProperty().samplingInfo() );
                    idright = adcontrols::MSProperty::toIndex( xright, seg.getMSProperty().samplingInfo() );
                }
            } else {
                if ( const double * x = seg.getMassArray() ) {
                    idleft = std::distance( x, std::lower_bound( x, x + seg.size(), xleft ) );
                    idright = std::distance( x, std::lower_bound( x, x + seg.size(), xright ) );
                }
            }

            if ( idleft >= seg.size() )
                continue;

            if ( idright > seg.size() )
                idright = seg.size();

            if ( idleft <= idright ) {
                const double * y = seg.getIntensityArray();

                auto minmax = std::minmax_element( y + idleft, y + idright );
                double min = *minmax.first;
                double max = *minmax.second;

                top = std::max( top, max );
                if ( isCentroid ) {
                    bottom = 0;
                } else {
                    bottom = min - (max - min) / 25;
                }
            }
        }
    }
    if ( top < bottom )
        return std::make_pair( 0.0, 1.0 );
    return std::make_pair<>(bottom, top);
}

void
SpectrumWidget::impl::clear_annotations()
{
	annotations_.clear();
}

void
SpectrumWidget::impl::update_annotations( plot& plot, const std::pair<double, double>& range )
{
    using adportable::array_wrapper;
    using namespace adcontrols::metric;

    // adportable::scoped_debug<> scope( __FILE__, __LINE__ ); scope << "update_annotateion:";
    plot.setUpdatesEnabled( false );

    typedef std::tuple< size_t, size_t, int, double, double > peak; // fcn, idx, color, mass, intensity
    enum { c_fcn, c_idx, c_color, c_intensity, c_mass };

    if ( auto ms = ms_.lock() ) {

        std::vector< peak > peaks;
        adcontrols::segment_wrapper< const adcontrols::MassSpectrum > segments( *ms );

        adcontrols::annotations auto_annotations;
        adcontrols::annotations annotations;

        double max_y = adcontrols::segments_helper::max_intensity( *ms );

        for ( size_t fcn = 0; fcn < segments.size(); ++fcn ) {
            const adcontrols::MassSpectrum& ms = segments[ fcn ];
            const unsigned char * colors = ms.getColorArray();

            size_t beg( 0 ), end( 0 );
            if ( isTimeAxis_ ) {
                if ( const double * times = ms.getTimeArray() ) {
                    beg = std::distance( times, std::lower_bound( times, times + ms.size(), scale_to_base( range.first, micro ) ) );
                    end = std::distance( times, std::lower_bound( times, times + ms.size(), scale_to_base( range.second, micro ) ) );
                }
            } else {
                if ( const double * masses = ms.getMassArray() ) {
                    beg = std::distance( masses, std::lower_bound( masses, masses + ms.size(), range.first ) );
                    end = std::distance( masses, std::lower_bound( masses, masses + ms.size(), range.second ) );
                }
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
                    auto it = std::find_if( a.second.begin(), a.second.end()
                                            , [] ( const adcontrols::annotation& x ) { return x.dataFormat() == adcontrols::annotation::dataFormula; } );
                    if ( it == a.second.end() )
                        it = std::find_if( a.second.begin(), a.second.end()
                                           , [] ( const adcontrols::annotation& x ) { return x.dataFormat() == adcontrols::annotation::dataText; } );
                    if ( it != a.second.end() )
                        annotations << *it;
                }

                if ( ms.isCentroid() && !ms.isHistogram() && autoAnnotation_ ) {
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
            bool added = annots.insert( a.x(), a.y(), text, Qt::AlignTop | Qt::AlignHCenter );
            (void)added;
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
			auto added = annots.insert( a.x(), a.y(), text, Qt::AlignTop | Qt::AlignHCenter );
            (void)added;
        }
    }
    plot.setUpdatesEnabled( true );
}

void
SpectrumWidget::impl::clear()
{
    ms_.reset();
    annotations_.clear();
    if ( ! traces_.empty() )
        traces_.clear();
}

QwtText
SpectrumWidget::impl::tracker1( const QPointF& pos )
{
    if ( isTimeAxis_ ) {
        return QwtText( (boost::format( "%.4f&mu;s" ) % pos.x()).str().c_str(), QwtText::RichText );
    }
    else {
        return QwtText( (boost::format( "<i>m/z=</i>%.4f" ) % pos.x()).str().c_str(), QwtText::RichText );
    }
}

QwtText
SpectrumWidget::impl::tracker2( const QPointF& p1, const QPointF& pos )
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

void
SpectrumWidget::setViewId( uint32_t id )
{
    viewid_ = id;
}

uint32_t
SpectrumWidget::viewId() const
{
    return viewid_;
}
