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
#include <adcontrols/segment_wrapper.hpp>
#include <adportable/array_wrapper.hpp>
#include <adportable/bounds.hpp>
#include <adportable/debug.hpp>
#include <adportable/float.hpp>
#include <adportable/scoped_debug.hpp>
#include <adportable/tuple_arith.hpp>
#include <qtwrapper/font.hpp>
#include <qwt_plot_picker.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_picker_machine.h>
#include <qwt_text.h>
#include <QDebug>
#include <QPen>
#include <QSignalBlocker>
#include <boost/format.hpp>
#include <array>
#include <atomic>
#include <mutex>
#include <set>

using namespace adplot;

namespace adplot {

    namespace {
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
            , QColor( 0xff, 0x00, 0xff )  //15  magenta
            , QColor( 0xff, 0x8c, 0x00 )  //16  dark orange
            , QColor( 0x00, 0x00, 0x00, 0x00 )  //17
        };
    }

    class SpectrumWidget::impl {
    public:
        impl() : autoAnnotation_( true )
               , isTimeAxis_( false )
               , yAxisForAnnotation_( QwtPlot::yLeft )
               , keepZoomed_( true )
               , haxis_( HorizontalAxisMass )
               , focusedFcn_( -1 ) // no focus
               , scaleFcn_( -1 )
               , yScale1_( {} )       // yLeft user specified
               , normalizedY_{ false }
            {}
        bool autoAnnotation_;
        bool isTimeAxis_;

        std::weak_ptr< const adcontrols::MassSpectrum > msForAnnotation_;  // for annotation
        QwtPlot::Axis yAxisForAnnotation_;

        std::vector< Annotation > annotations_;
        std::vector< std::unique_ptr< TraceData > > traces_;

        std::atomic<bool> keepZoomed_;
        std::atomic<HorizontalAxis> haxis_;
        std::atomic<int> focusedFcn_;
        int scaleFcn_;
        std::mutex mutex_;
        std::optional< std::pair< double, double > > yScale1_;
        std::array< bool, QwtPlot::axisCnt > normalizedY_;

        bool normalizedY( QwtPlot::Axis axis ) const { return normalizedY_.at( axis ); }

        void clear();
        void update_annotations( plot&, const QRectF&, QwtPlot::Axis );
		void clear_annotations();

        // void handleZoomRect( QRectF& );
        QwtText tracker1( const QPointF& );
        QwtText tracker2( const QPointF&, const QPointF& );

        boost::optional< std::pair< double, double > > scaleY( const QRectF&, QwtPlot::Axis ) const;

        std::pair<bool,bool> scaleY( const QRectF&, std::pair< double, double >& left, std::pair< double, double >& right );
        void baseScale( bool, QRectF& rc );

    };

    /////////////////////////////////////////////////////////////////////////////

    class SpectrumWidget::xSeriesData : public QwtSeriesData<QPointF> {
        xSeriesData( const xSeriesData& ) = delete;
        xSeriesData& operator = ( const xSeriesData& ) = delete;
    public:
        virtual ~xSeriesData() {
        }

        xSeriesData( SpectrumWidget * pThis
                     , const adcontrols::MassSpectrum& ms
                     , const QRectF& rc
                     , bool axisTime ) : pThis_( pThis )
                                       , rect_( rc )
                                       , xrange_( rc.left(), rc.right() )
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

            if ( pThis_->normalizedY( QwtPlot::yLeft ) ) {
                double i = 1000 * ms_.intensity( idx ) / minmax_.second;
                if ( axisTime_ )
                    return QPointF( scale_to<double, micro>( ms_.time( idx )), i );
                else
                    return QPointF( ms_.mass( idx ), i );
            } else {
                if ( axisTime_ )
                    return QPointF( scale_to<double, micro>( ms_.time( idx  )), ms_.intensity( idx ) );
                else
                    return QPointF( ms_.mass( idx ), ms_.intensity( idx ) );
            }
        }

        QRectF boundingRect() const override {
            return rect_;
        }

        size_t make_color_index( unsigned char color ) {
            const unsigned char * colors = ms_.getColorArray();
            for ( size_t i = 0; i < ms_.size(); ++i ) {
                if ( color == colors[i] )
                    indices_.emplace_back( i );
            }
            return indices_.size();
        }

        void handleZoomed( const QRectF& rc ) {
            xrange_ = std::make_pair( rc.left(), rc.right() );
            auto mm = ms_.minmax_element( xrange_, axisTime_ );
            minmax_ = std::make_pair( ms_.intensity( mm.first ), ms_.intensity( mm.second ) );
            if ( pThis_->impl_->normalizedY_[ QwtPlot::yLeft ] ) {
                ADDEBUG() << "--- xSeriesData: " << xrange_ << ", mm=" << minmax_;
            }
        }

    private:
        SpectrumWidget * pThis_;
        QRectF rect_;
        std::pair< double, double > xrange_;
        std::pair< double, double > minmax_;
        const adcontrols::MassSpectrum& ms_;
        std::vector< size_t > indices_; // if centroid with color,
        bool axisTime_;
    };

    ///////////////////////////////////////////////////////////////////////////

    class SpectrumWidget::TraceData {
    public:
        TraceData( SpectrumWidget * pThis
                   , int idx ) : pThis_( pThis )
                               , idx_( idx )
                               , focusedFcn_( -1 )
                               , alpha_( 255 )
                               , yAxis_( QwtPlot::yLeft ) {

            color_ = color_table[ idx % ( sizeof( color_table ) / sizeof( color_table[ 0 ] ) ) ];
        }

        TraceData( const TraceData& t ) : pThis_( t.pThis_ )
                                        , idx_( t.idx_ )
                                        , focusedFcn_( t.focusedFcn_ )
                                        , alpha_( t.alpha_ )
                                        , rect_( t.rect_ )
                                        , currZoomRect_( t.currZoomRect_ )
                                        , color_( t.color_ )
                                        , yAxis_( t.yAxis_ )
                                        , curves_( t.curves_ )
                                        , pSpectrum_( t.pSpectrum_ )
                                        , isTimeAxis_( t.isTimeAxis_ )   {
        }

        ~TraceData();
        void __set( plot& plot
                    , std::shared_ptr< const adcontrols::MassSpectrum>&
                    , QRectF&, SpectrumWidget::HorizontalAxis, bool yRight );
        void __set( plot& plot
                    , std::shared_ptr< const adcontrols::MassSpectrum>&
                    , QRectF&, SpectrumWidget::HorizontalAxis, QwtPlot::Axis );
        void redraw( plot& plot, SpectrumWidget::HorizontalAxis, QRectF&, QRectF& );
        void setFocusedFcn( int fcn );
        std::pair<double, double> y_range( double left, double right, int fcn ) const;
        inline bool yRight() const { return yAxis_ == QwtPlot::yRight ; }
        inline QwtPlot::Axis yAxis() const { return yAxis_; }
        void setAlpha( int alpha );
        void setColor( const QColor& );
        const QRectF& rect() const { return rect_; }

        void handleZoomed( const QRectF& rc ) {
            currZoomRect_ = rc;
            for ( auto& curve: curves_ ) {
                if ( auto data = dynamic_cast< xSeriesData * >(curve->data()) ) {
                    data->handleZoomed( rc );
                    // ADDEBUG() << "tracedata zoomed: " << std::make_pair( rc.left(), rc.right() ) << " <-- " << std::make_pair( rect_.left(), rect_.right() );
                }
            }
        }

    private:
        void setProfileData( plot& plot, const adcontrols::MassSpectrum& ms, const QRectF&, QwtPlot::Axis );
        void setCentroidData( plot& plot, const adcontrols::MassSpectrum& ms, const QRectF&, QwtPlot::Axis );
        void changeFocus( int focusedFcn );

        SpectrumWidget * pThis_;
        int idx_;
        int focusedFcn_;
        uint8_t alpha_;
        QRectF rect_;
        QRectF currZoomRect_;
        QColor color_;
        QwtPlot::Axis yAxis_;
    public:
        std::vector< std::shared_ptr< adPlotCurve > > curves_;
        std::shared_ptr< const adcontrols::MassSpectrum > pSpectrum_;
    private:
        bool isTimeAxis_;
    };

    ///////////////////////////////////

} // namespace adplot

namespace {

    struct make_text {
        QwtText operator()( const adcontrols::annotation& a ) const {
            if ( a.dataFormat() == adcontrols::annotation::dataFormula ) {
                return QwtText( QString::fromStdString( adcontrols::ChemicalFormula::formatFormulae( a.text() ) ), QwtText::RichText );
            } else {
                return QwtText( QString::fromStdString( a.text() ), QwtText::RichText );
            }
        }
    };

    struct compare_range {
        bool operator()( const std::tuple<double, double >& range, const adcontrols::mass_value_type& value, bool isTime ) const {
            if (isTime)
                return adportable::bounds( range ).contains( std::get<0>(value) );
            else
                return adportable::bounds( range ).contains( std::get<1>(value) );
        }
    };
}


QColor
SpectrumWidget::index_color( unsigned int idx )
{
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

        zoomer->autoYScaleHock( [this]( const QRectF& rc ){ return yScaleHock( rc ); } );

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
    impl_->update_annotations( *this, rc, impl_->yAxisForAnnotation_ );
    if ( bReplot )
        replot();
}

QRectF
SpectrumWidget::yScaleHock( const QRectF& rc )
{
    QRectF rect(rc);

    if ( auto range = impl_->scaleY( rc, QwtPlot::yLeft ) ) {
        rect.setCoords( rc.left(), range->first, rc.right(), range->second );
    }
    return rect;
}

void
SpectrumWidget::yZoom( double xmin, double xmax )
{
    std::pair<double, double > left, right;
    QRectF rc( QPointF( xmin, 0 ), QPointF( xmax, 0 ) );

    auto hasAxes = impl_->scaleY( rc, left, right );
    if ( hasAxes.second && ! adportable::compare<double>::approximatelyEqual( right.first, right.second ) ) {
        setAxisScale( QwtPlot::yRight, right.first, right.second ); // set yRight
    }

    if ( hasAxes.first && ! adportable::compare<double>::approximatelyEqual( left.first, left.second ) ) {
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
SpectrumWidget::impl::scaleY( const QRectF& rc, QwtPlot::Axis yAxis ) const
{
    boost::optional< std::pair< double, double > > range{ boost::none };

    if ( normalizedY_[ yAxis ] ) {
        return {{-10,1100}};
    }

    for ( const auto& trace: traces_ ) {
        if ( trace && (trace->yAxis() == yAxis ) ) {
            auto y = trace->y_range( rc.left(), rc.right(), scaleFcn_ );
            range = range ? std::make_pair( std::min( y.first, range->first ), std::max( y.second, range->second ) ) : y;
        }
    }
    return range;
}

std::pair<bool, bool>
SpectrumWidget::impl::scaleY( const QRectF& rc, std::pair< double, double >& left, std::pair< double, double >& right )
{
    bool hasYLeft( false ), hasYRight( false );

    left = right = std::make_pair( std::numeric_limits<double>::max(), std::numeric_limits<double>::lowest() );

    // ADDEBUG() << "normalizedY: " << normalizedY_[ QwtPlot::yLeft ];

    if ( normalizedY_[ QwtPlot::yLeft ] ) {
        left = {-10,1100};
        return {true,false};
    }

    for ( const auto& trace: traces_ ) {
        if ( trace ) {
            std::pair<double, double> y = trace->y_range( rc.left(), rc.right(), scaleFcn_ );
            if ( trace->yAxis() == QwtPlot::yRight ) {
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
    if ( auto range = impl_->scaleY( rect, QwtPlot::yRight ) ) {
        setAxisScale( QwtPlot::yRight, range->first, range->second ); // set yRight
    }

    for ( auto& trace: impl_->traces_ ) {
        if ( trace )
            trace->handleZoomed( rect );
    }

    impl_->update_annotations( *this, rect, impl_->yAxisForAnnotation_ );
    replot();
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

size_t
SpectrumWidget::size() const
{
    return impl_->traces_.size();
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
                                , [&]( std::unique_ptr< TraceData >& trace ){ return trace && trace->pSpectrum_; } );
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
SpectrumWidget::setNormalizedY( QwtPlot::Axis axis, bool normalized )
{
    if ( QwtPlot::isAxisValid( axis ) )
        impl_->normalizedY_[ axis ] = normalized;
}

bool
SpectrumWidget::normalizedY( QwtPlot::Axis axis ) const
{
    return impl_->normalizedY_[ axis ];
}

void
SpectrumWidget::removeData( int idx, bool bReplot )
{
    if ( idx < 0 ) {
        impl_->traces_.clear();
    } else if ( idx < int(impl_->traces_.size()) ) {
        impl_->traces_[ idx ].reset();
    }
    impl_->clear_annotations();
    if ( bReplot )
        replot();
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
SpectrumWidget::setData( std::shared_ptr< const adcontrols::MassSpectrum > ptr, int idx, QwtPlot::Axis axis )
{
    // ScopedDebug(__t);
    impl_->scaleFcn_ = (-1);

    if ( !ptr || ptr->size() == 0 ) {
        removeData( idx );
        return;
    }

    if ( ptr->isHistogram() ) {
        setAxisTitle( axis, QwtText( "Counts" ) );
    } else {
        auto label = ptr->isCentroid() ? QwtText( "Intensity" ) : QwtText( "Intensity (mV)" );
        setAxisTitle( axis, label );
    }

    if ( impl_->traces_.size() <= idx )
        impl_->traces_.resize( idx + 1 );

    if ( ! impl_->traces_[ idx ] )
        impl_->traces_[ idx ] = std::make_unique< TraceData >( this, idx );

    auto& trace = impl_->traces_[ idx ];

    do {
        QRectF rect;
        trace->__set( *this, ptr, rect, impl_->haxis_, axis ); // clear canvas if ptr == nullptr
    } while ( 0 );

    QRectF baseRect;
    impl_->baseScale( yRight, baseRect );

    if ( impl_->yScale1_ ) {
        baseRect.setTop( impl_->yScale1_->first );
        baseRect.setBottom( impl_->yScale1_->second );
    }

    auto rectIndex = zoomer()->zoomRectIndex();

    if ( ( axis == QwtPlot::yLeft ) && ( rectIndex == 0 || !impl_->keepZoomed_ ) ) {

        setAxisScale( axis, baseRect.bottom(), baseRect.top() );
        setAxisScale( QwtPlot::xBottom, baseRect.left(), baseRect.right() );
        zoomer()->setZoomBase();

    } else {
        QRectF z = zoomer()->zoomRect(); // current

        std::pair<double, double> left, right;
        auto hasAxes = impl_->scaleY( z, left, right );

        if ( axis == QwtPlot::yRight && hasAxes.second ) {

            setAxisScale( QwtPlot::yRight, right.first, right.second );

        } else {
            if ( hasAxes.first ) {
                QStack< QRectF > zstack;
                zstack.push_back( QRectF( baseRect.x(), baseRect.bottom(), baseRect.width(), -baseRect.height() ) ); // upside down
                if ( impl_->yScale1_ )
                    zstack.push_back( QRectF( z.x(), impl_->yScale1_->second, z.width(), impl_->yScale1_->first - impl_->yScale1_->second ) );
                else
                    zstack.push_back( QRectF( z.x(), left.first, z.width(), left.second - left.first ) );
                QSignalBlocker block( zoomer() );
                zoomer()->setZoomStack( zstack );
            }
        }
    }

    // ADDEBUG() << __FUNCTION__ << ", autoAnnotation: " << impl_->autoAnnotation_ << ", axis: " << axis << ", annotation.size:" << ptr->annotations().size();

    // take annotation on last drawn spectrum, which has annotations
    //if ( ! ptr->annotations().empty() ) {
    do {
        impl_->msForAnnotation_ = ptr;
        impl_->yAxisForAnnotation_ = axis;
        update_annotation( false );
        //}
    } while ( 0 );

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

std::optional< std::pair< double, double > >
SpectrumWidget::yScale( QwtPlot::Axis yAxis ) const
{
    if ( yAxis == QwtPlot::yRight ) {
        return {};
    } else {
        return impl_->yScale1_;
    }
}

void
SpectrumWidget::setYScale( double top, double bottom, QwtPlot::Axis yAxis )
{
    if ( yAxis == QwtPlot::yLeft ) {
        bool yAuto(false);
        if ( adportable::compare< double >::essentiallyEqual( top, bottom ) ) {
            yAuto = true;
            impl_->yScale1_ = {};
        } else {
            yAuto = false;
            impl_->yScale1_ = std::make_pair(top, bottom);
        }

        if ( auto zoomer = plot::zoomer() )
            zoomer->autoYScale( yAuto );
    }
}

void
SpectrumWidget::replotYScale()
{
    auto rectIndex = zoomer()->zoomRectIndex();

    QRectF baseRect;
    impl_->baseScale( false, baseRect ); // always get left-Y axis
    if ( impl_->yScale1_ ) {
        baseRect.setTop( impl_->yScale1_->first );
        baseRect.setBottom( impl_->yScale1_->second );
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
        if ( impl_->yScale1_ ) {
            zstack.push_back( QRectF( z.x(), impl_->yScale1_->second, z.width(), impl_->yScale1_->first - impl_->yScale1_->second ) );
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

SpectrumWidget::TraceData::~TraceData()
{
    curves_.clear();
}

void
SpectrumWidget::TraceData::setProfileData( plot& plot, const adcontrols::MassSpectrum& ms, const QRectF& rect, QwtPlot::Axis yAxis )
{
    adcontrols::segment_wrapper< const adcontrols::MassSpectrum > segments( ms );

    int fcn = 0;
    for ( auto& seg: segments ) {

        auto ptr = std::make_shared< adPlotCurve >();
        ptr->attach( &plot );
        curves_.emplace_back( ptr );

        int cid = ( idx_ + fcn ) % ( sizeof(color_table)/sizeof(color_table[0]) );
        QColor color( color_table[ cid ] );
        color.setAlpha( alpha_ );
        ptr->setPen( color );
        ptr->setData( new xSeriesData( pThis_, seg, rect, isTimeAxis_ ) );
        ptr->setYAxis( yAxis );
        ++fcn;
    }
}

void
SpectrumWidget::TraceData::changeFocus( int focusedFcn )
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
SpectrumWidget::TraceData::setCentroidData( plot& plot, const adcontrols::MassSpectrum& _ms, const QRectF& rect, QwtPlot::Axis yAxis )
{
    curves_.clear();

    for ( auto& seg: adcontrols::segment_wrapper< const adcontrols::MassSpectrum >( _ms ) ) {
        if ( const unsigned char * colors = seg.getColorArray() ) {
            std::set< unsigned char > color;
            for ( size_t i = 0; i < seg.size(); ++i )
                color.insert( colors[i] );

            for ( const auto& c: color ) {
                xSeriesData * xp = new xSeriesData( pThis_, seg, rect, isTimeAxis_ );
                if ( xp->make_color_index( c ) ) {
                    auto curve = std::make_shared< adPlotCurve >();
                    curve->attach( &plot );
                    curves_.push_back( curve );

                    curve->setData( xp );
                    curve->setPen( QPen( color_table[ c ] ) );
                    curve->setStyle( QwtPlotCurve::Sticks );
                    curve->setYAxis( yAxis );
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
            curve->setData( new xSeriesData( pThis_, seg, rect, isTimeAxis_ ) );
            curve->setStyle( QwtPlotCurve::Sticks );
            curve->setYAxis( yAxis );
        }
    }
}

void
SpectrumWidget::TraceData::redraw( plot& plot, SpectrumWidget::HorizontalAxis axis, QRectF& rcLeft, QRectF& rcRight )
{
    QRectF rect;

    if ( auto p = pSpectrum_ ) {
        __set( plot, p, rect, axis, yAxis_ );

        QRectF& rc = (yAxis_ == QwtPlot::yRight ) ? rcRight : rcLeft;
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
SpectrumWidget::TraceData::__set( plot& plot
                  , std::shared_ptr< const adcontrols::MassSpectrum >& ms
                  , QRectF& rect
                  , SpectrumWidget::HorizontalAxis haxis
                  , QwtPlot::Axis axis )
{
    curves_.clear();

    if ( !ms ) {
        pSpectrum_.reset();
        return;
    }

    if ( pSpectrum_ != ms )
        pSpectrum_ = ms;

    isTimeAxis_ = haxis == SpectrumWidget::HorizontalAxisTime;
    yAxis_ = axis;

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
            time_range.first = ms->time( 0 );
            time_range.second = ms->time( ms->size() - 1 );
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
            mass_range.first = double( int( ms->mass(0) * 10 ) ) / 10.0;  // round to 0.1Da
            mass_range.second = double( int( ms->mass( ms->size() - 1 ) * 10 + 1 ) ) / 10.0;
        }
        rect.setCoords( mass_range.first, top, mass_range.second, bottom );
    }

    rect_ = rect;
    if ( ms->isCentroid() ) { // sticked
        setCentroidData( plot, *ms, rect, yAxis_ );
    } else { // Profile
        setProfileData( plot, *ms, rect, yAxis_ );
    }
}

void
SpectrumWidget::TraceData::setFocusedFcn( int fcn )
{
    if ( focusedFcn_ != fcn ) {
        focusedFcn_ = fcn;
        if ( auto p = pSpectrum_ ) {
            changeFocus( focusedFcn_ );
        }
    }
}

void
SpectrumWidget::TraceData::setAlpha( int alpha )
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
SpectrumWidget::TraceData::setColor( const QColor& color )
{
    color_ = color;
    for ( auto& curve: curves_ )
        curve->setPen( color_ );
}

std::pair< double, double >
SpectrumWidget::TraceData::y_range( double left, double right, int fcn ) const
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
                std::make_pair( seg.time( 0 ), seg.time( seg.size() - 1 ) ) :
                std::make_pair( seg.mass( 0 ), seg.mass( seg.size() - 1 ) );

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
    //return std::make_pair<>(bottom, top );
    return std::make_pair<>(bottom, top + ( top - bottom ) * 0.14 );
}

void
SpectrumWidget::impl::clear_annotations()
{
	annotations_.clear();
}

void
SpectrumWidget::impl::update_annotations( plot& plot, const QRectF& rc, QwtPlot::Axis axis )
{
    using adportable::array_wrapper;
    // using namespace adcontrols::metric;

    plot.setUpdatesEnabled( false );
    yAxisForAnnotation_ = axis;

    using namespace adportable::tuple_arith;
    auto range = std::make_tuple( rc.left(), rc.right() );
    if ( isTimeAxis_ )
        range = range * (1.0/std::micro::den);

    typedef std::tuple< size_t, size_t, int, double, double > peak; // fcn, idx, color, mass, intensity
    enum { c_fcn, c_idx, c_color, c_intensity, c_mass };

    if ( auto ms = msForAnnotation_.lock() ) {

        std::vector< peak > peaks;
        adcontrols::segment_wrapper< const adcontrols::MassSpectrum > segments( *ms );

        std::vector< adcontrols::annotation > avec;
        std::vector< adcontrols::annotation > a_avec; // auto-generated

        for ( size_t fcn = 0; fcn < segments.size(); ++fcn ) {
            const adcontrols::MassSpectrum& ms = segments[ fcn ];
            double max_y = ms.intensity( ms.max_element( std::make_pair( std::get<0>(range), std::get<1>(range) ) ) );

            for ( const auto& a: ms.annotations() ) {
                // ADDEBUG() << "-- annotation: " << std::make_tuple( a.text(), a.dataFormat(), a.flags() );
                if ( a.dataFormat() != adcontrols::annotation::dataJSON ) {
                    if (( a.index() >= 0 ) && adportable::bounds( range ).contains( a.x() ) ) {
                        auto tmp( a );
                        tmp.x( isTimeAxis_ ? ms.time( a.index() ) : a.x() );
                        avec.emplace_back( std::move( tmp ) );
                    }
                }
            }

            if ( ms.isCentroid() && !ms.isHistogram() && autoAnnotation_ ) {
                // generate auto-annotation
                for ( size_t i = 0; i < ms.size(); ++i ) {
                    if ( compare_range()( range, ms.value( i ), isTimeAxis_ ) ) {
                        if ( std::find_if ( avec.begin(), avec.end()
                                            , [i]( const auto& a ){ return a.index() == int(i); } ) == avec.end() ) {
                            a_avec.emplace_back( ( boost::format( "%.3f" ) % ms.mass( i ) ).str()
                                                 , isTimeAxis_ ? (ms.time( i ) * std::micro::den) : ms.mass( i )
                                                 , ms.intensity( i )
                                                 , i
                                                 , (ms.intensity(i) / max_y) * 256 );
                        }
                    }
                }
                std::sort( a_avec.begin(), a_avec.end()
                           , [](const auto& a, const auto& b){ return a.priority() > b.priority(); } );
            }
        }

        annotations_.clear();
        Annotations annots(plot, annotations_);
        for ( const auto& a: avec ) {
            auto text = make_text()( a );// QwtText text( QString::fromStdString(a.text()), QwtText::RichText);
            text.setColor( Qt::darkGreen );
            text.setFont( Annotation::font() );
            annots.insert( a.x(), a.y(), yAxisForAnnotation_, text, Qt::AlignTop | Qt::AlignHCenter );
        }

        QFont font = Annotation::font();
        font.setPointSize( font.pointSize() - 1 );
        for ( const auto& a: a_avec ) {
            auto text = QwtText( QString::fromStdString(a.text()), QwtText::RichText );
            text.setColor( Qt::darkGray );
            text.setFont( font );
            annots.insert( a.x(), a.y(), yAxisForAnnotation_, text, Qt::AlignTop | Qt::AlignHCenter );
        }
    }
    plot.setUpdatesEnabled( true );
}

void
SpectrumWidget::impl::clear()
{
    msForAnnotation_.reset();
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
