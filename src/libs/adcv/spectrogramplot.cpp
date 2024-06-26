/**************************************************************************
** Copyright (C) 2010-2021 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2021 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "spectrogramplot.hpp"
#include <adportable/debug.hpp>
#include <adcontrols/mappedimage.hpp>
#include <adplot/zoomer.hpp>
#include <qtwrapper/font.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <qwt_color_map.h>
#include <qwt_interval.h>
#include <qwt_picker_machine.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_renderer.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_plot_zoomer.h>
#include <qwt_scale_draw.h>
#include <qwt_scale_map.h>
#include <qwt_scale_engine.h>
#include <qwt_scale_widget.h>
#include <QBrush>
#include <QColor>
#include <QPainter>
#include <QPen>
#include <QPrinter>
//#include <qwt_plot_rescaler.h>
#include <boost/numeric/ublas/matrix.hpp>
#include <iostream>

namespace adcv {

    class marker : public QwtPlotItem {
        QRect rect_;

    public:
        marker() : rect_( 7, 7, 1, 1 ) { setZ( 10 ); }

        void setRect( const QPointF& p ) {
            rect_ = QRect( int( p.x() ), int( p.y() ), 1, 1 );
        }

        void setRect( const QRectF& rc ) {
            rect_ = QRect( int( rc.x() ), int( rc.y() ), int( rc.width() + 1 ), int( rc.height() + 1 ) );
        }

        int x() const { return rect_.x(); }
        int y() const { return rect_.y(); }
        int width() const { return rect_.width(); }
        int height() const { return rect_.height(); }
        QRect& rect() { return rect_; }
        const QRect& rect() const { return rect_; }


    private:
        void draw( QPainter * painter, const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QRectF& canvasRect ) const override {
            // QRectF scaleRect = this->scaleRect( xMap, yMap );
            painter->save();
            QPen pen( QColor( 0xff, 0xff, 0xff, 0x80 ) ); pen.setWidth( 2 );
            painter->setPen( pen );
            QRectF xrc = QwtScaleMap::transform( xMap, yMap, rect_ );
            if ( canvasRect.contains( xrc ) )
                painter->drawRect( xrc );
            painter->restore();
        }
    };

    // ------------------ picker ----------------------
    class picker : public QwtPlotPicker {
    public:
        explicit picker( QWidget * parent = 0 ) : QwtPlotPicker( parent ) {
            setTrackerMode( QwtPicker::AlwaysOff );
            setStateMachine( new QwtPickerDragRectMachine() );
            setRubberBand( QwtPicker::RectRubberBand );
            setRubberBandPen( QColor( Qt::red ) );
            setTrackerPen( QColor( Qt::white ) );
        }

        QwtText trackerTextF( const QPointF &pos ) const override {
            QwtText text( QString( "(%1,%2)" ).arg( QString::number( int( pos.x() ) ), QString::number( int( pos.y() ) ) ) );
            QColor bg( Qt::white );
            bg.setAlpha( 100 );
            text.setBackgroundBrush( QBrush( bg ) );
            return text;
        }
    };

    // ------------------ zoomer ----------------------
    class zoomer : public QwtPlotZoomer {
    public:
        zoomer( QWidget * canvas ) : QwtPlotZoomer( canvas ) {
            setTrackerMode( ActiveOnly );
        }

        virtual QwtText trackerTextF( const QPointF &pos ) const {
            QColor bg( Qt::white );
            bg.setAlpha( 200 );

            QwtText text( QString( "%1,%2" ).arg( QString::number( int( pos.x() ) ), QString::number( int( pos.y() ) ) ) );
            text.setBackgroundBrush( QBrush( bg ) );
            return text;
        }
    };

    // ---------------- SpectrogramData ------------------
    class SpectrogramData: public QwtRasterData  {
    public:
        SpectrogramData( uint32_t size1, uint32_t size2 ) : dimension_( std::make_pair( size1, size2 ) ) {
            setInterval( Qt::XAxis, QwtInterval( 0, size1, QwtInterval::ExcludeMaximum ) ); // columns
            setInterval( Qt::YAxis, QwtInterval( 0, size2, QwtInterval::ExcludeMaximum ) ); // rows
            setInterval( Qt::ZAxis, QwtInterval( 0, 10.0 ) );
        }

        QwtInterval  interval( Qt::Axis axis ) const override {
            switch ( axis ) {
            case Qt::XAxis: return std::get< 0 >( interval_ );
            case Qt::YAxis: return std::get< 1 >( interval_ );
            case Qt::ZAxis: return std::get< 2 >( interval_ );
            }
            return {};
        }

        void setData( cv::Mat&& data ) {
            data_ = std::move( data );
        }

        bool dimension( size_t nrows, size_t ncolumns ) {
            if ( dimension_ == std::make_pair( nrows, ncolumns ) ) {
                dimension_ = std::make_pair( nrows, ncolumns );
                setInterval( Qt::XAxis, QwtInterval( 0, ncolumns ) );
                setInterval( Qt::YAxis, QwtInterval( 0, nrows ) );
                return true;
            }
            return false;
        }

        double value( double _x, double _y ) const  override {
            if ( ! data_.empty() ) {
                size_t x = size_t( _x + 0.5 );
                size_t y = size_t( _y + 0.5 );
                if ( x < data_.cols && y < data_.rows ) {
                    return data_.at< float >( y, x );
                }
            }
            return 0;
        }

        void setInterval( Qt::Axis axis, QwtInterval&& interval ) {
            switch ( axis ) {
            case Qt::XAxis: std::get< 0 >( interval_ ) = std::move( interval ); break;
            case Qt::YAxis: std::get< 1 >( interval_ ) = std::move( interval ); break;
            case Qt::ZAxis: std::get< 2 >( interval_ ) = std::move( interval ); break;
            }
        }
    private:
        cv::Mat data_;
        std::pair< size_t, size_t > dimension_;
        std::tuple< QwtInterval, QwtInterval, QwtInterval > interval_;
    };

    //------------------------- ColorMap -------------------------------
    class ColorMap: public QwtLinearColorMap {
    public:
        ColorMap(): QwtLinearColorMap( Qt::black, Qt::white ) {
            addColorStop( 0.05, Qt::blue );
            addColorStop( 0.30, Qt::cyan );
            addColorStop( 0.60, Qt::green );
            addColorStop( 0.80, Qt::yellow );
            addColorStop( 0.97, Qt::red );
        }
    };

    //------------------------- impl ----------------------------------
    class SpectrogramPlot::impl {
    public:
        impl( SpectrogramPlot * p ) : spectrogram_( new QwtPlotSpectrogram() )
                                    , drawable_( new SpectrogramData( 1280, 960 ) )
                                    , marker_( new marker() ) {
        }

        ~impl()  {
            // following two objects are self-delete in QWT
            drawable_ = 0;
            spectrogram_ = 0;
        }

        QwtPlotSpectrogram * spectrogram_;
        SpectrogramData * drawable_;
        marker * marker_;
    };

    //----------------------------------------------------------------

}

using namespace adcv;

SpectrogramPlot::~SpectrogramPlot()
{
}

SpectrogramPlot::SpectrogramPlot( QWidget *parent ) : QwtPlot(parent)
                                                    , impl_( new impl( this ) )
{
    impl_->spectrogram_->setRenderThreadCount( 0 ); // use system specific thread count

    impl_->spectrogram_->setColorMap( new ColorMap() );
    impl_->spectrogram_->setCachePolicy( QwtPlotRasterItem::PaintCache );

    impl_->spectrogram_->setData( impl_->drawable_ );
    impl_->spectrogram_->attach( this );

    impl_->marker_->attach( this );

    // //plotLayout()->setAlignCanvasToScales( true );
    // impl_->rescaler_->setExpandingDirection( QwtPlotRescaler::ExpandUp );
    // impl_->rescaler_->rescale();

    // QList<double> contourLevels;
    // for ( double level = 0.5; level < 10.0; level += 1.0 )
    //     contourLevels += level;
    // impl_->spectrogram_->setContourLevels( contourLevels );

    const QwtInterval zInterval = impl_->spectrogram_->data()->interval( Qt::ZAxis );
    // A color bar on the right axis
    QwtScaleWidget *rightAxis = axisWidget( QwtPlot::yRight );
    rightAxis->setTitle( "Intensity" );
    rightAxis->setColorBarEnabled( true );

    // rightAxis->setColorMap( zInterval, new ColorMap() );
    setAxisZMax( 10.0 );

    //setAxisScale( QwtPlot::xBottom, 0, 64 );
    //setAxisScale( QwtPlot::yLeft,  64, 0 );
    axisScaleEngine( QwtPlot::xBottom )->setAttribute( QwtScaleEngine::Floating, true );
    axisScaleEngine( QwtPlot::yLeft )->setAttribute( QwtScaleEngine::Floating, true );
    axisScaleEngine( QwtPlot::yLeft )->setAttribute( QwtScaleEngine::Inverted, true );

	setAxisScale(QwtPlot::yRight, zInterval.minValue(), zInterval.maxValue());
    enableAxis( QwtPlot::yRight );

    plotLayout()->setAlignCanvasToScales( true );
    replot();

    if ( auto zoomer = new adplot::Zoomer( QwtPlot::xBottom, QwtPlot::yLeft, canvas() ) ) {
        const QColor c( 0xe0, 0xff, 0xff, 0x40 ); // lightcyan
        zoomer->setRubberBandPen( c );
        zoomer->setTrackerPen( c );
        zoomer->setTrackerMode( QwtPlotPicker::ActiveOnly );
    }

    QwtPlotPanner *panner = new QwtPlotPanner( canvas() );
    panner->setAxisEnabled( QwtPlot::yRight, false );
    panner->setMouseButton( Qt::MiddleButton );

    if ( auto picker = new adcv::picker( canvas() ) ) {

        //picker->setStateMachine( new QwtPickerClickPointMachine() );
        picker->setStateMachine( new QwtPickerDragRectMachine() );
        picker->setRubberBand( QwtPicker::RectRubberBand );
        picker->setRubberBandPen( QColor( Qt::red ) );
        picker->setTrackerPen( QColor( Qt::white ) );

        picker->setMousePattern( QwtEventPattern::MouseSelect1, Qt::RightButton );
        picker->setTrackerMode( QwtPicker::AlwaysOff );
        picker->setEnabled( true );

        connect( picker, static_cast<void( QwtPlotPicker::* )( const QPointF& )>( &QwtPlotPicker::selected ),
                 this, [this] ( const QPointF& p ) {
                     impl_->marker_->setRect( p ); replot();
                     emit cellSelected( impl_->marker_->rect() );
                 } );

        connect( picker, static_cast<void( QwtPlotPicker::* )( const QRectF& )>( &QwtPlotPicker::selected ),
                 this, [this] ( const QRectF& rc ) {
                     impl_->marker_->setRect( rc ); replot();
                     emit cellSelected( impl_->marker_->rect() );
                 } );
    }

    QFont font = qtwrapper::font()( QFont(), qtwrapper::fontSizeSmall, qtwrapper::fontAxisLabel );
    setAxisFont( QwtPlot::xBottom, font );
    setAxisFont( QwtPlot::yLeft, font );

    // Set fixed 'extent' for axis in order to avoid jumping when labels with more/less digits
    // appear/disappear when scrolling vertically

    const QFontMetrics fm( axisWidget( QwtPlot::yLeft )->font() );
    QwtScaleDraw *sd = axisScaleDraw( QwtPlot::yLeft );
    sd->setMinimumExtent( fm.horizontalAdvance( "888.0" ) );

    setContextMenuPolicy( Qt::CustomContextMenu );
}

void
SpectrogramPlot::setData( const cv::Mat& mat )
{
#if 0
    auto depth = mat.type() & CV_MAT_DEPTH_MASK;
    auto chans = 1 + (mat.type() >> CV_CN_SHIFT);

    std::ostringstream o;
    switch ( depth ) {
    case CV_8U:  o << "8U"; break;
    case CV_8S:  o << "8S"; break;
    case CV_16U: o << "16U"; break;
    case CV_16S: o << "16S"; break;
    case CV_32S: o << "32S"; break;
    case CV_32F: o << "32F"; break;
    case CV_64F: o << "64F"; break;
    default:     o << "User"; break;
    }
    o << "C" << char( chans + '0' );
    ADDEBUG() << "mat.type: " << mat.type() << ", dim: " << std::make_pair( mat.cols, mat.rows ) << ", " << o.str();
#endif
    cv::Mat_< float > gray( mat.rows, mat.cols );

    if ( mat.type() == CV_8UC3 ) {
        cv::Mat_< uchar > gray8u;
        cv::cvtColor( mat, gray8u, cv::COLOR_BGR2GRAY );
        gray8u.convertTo( gray, CV_32FC(1) );
    } else if ( mat.type() == CV_32FC1 ){
        mat.copyTo( gray );
    }

    if ( impl_->drawable_->dimension( gray.rows, gray.cols ) ) {
        if ( auto zoomer = findChild< QwtPlotZoomer * >() ) {
            zoomer->setZoomBase();
        }
    }

    impl_->drawable_->setData( std::move( gray ) );


	impl_->spectrogram_->invalidateCache();

	replot();
}

void
SpectrogramPlot::setAxisZMax( double z )
{
    if ( z > 0 ) {
        impl_->drawable_->setInterval( Qt::ZAxis, { 0.0, z} );
        QwtInterval zInterval( 0.0, z );

        // A color bar on the right axis
        axisWidget( QwtPlot::yRight )->setColorMap( zInterval, new ColorMap() );
        setAxisScale( QwtPlot::yRight, zInterval.minValue(), zInterval.maxValue() );

        replot();
    }
}

double
SpectrogramPlot::z() const
{
    return impl_->spectrogram_->data()->interval( Qt::ZAxis ).maxValue();
}

void
SpectrogramPlot::setCellSelection( const QRect& rect )
{
    impl_->marker_->rect() = rect;
    replot();
}

void
SpectrogramPlot::setCellSelectionEnabled( bool enable )
{
    if ( impl_->marker_->isVisible() != enable ) {
        impl_->marker_->setVisible( enable );
        replot();
    }
}

QSize
SpectrogramPlot::sizeHint() const
{
    return QSize( 10, 10 );
}
