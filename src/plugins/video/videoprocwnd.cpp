/**************************************************************************
** Copyright (C) 2010-2021 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2021 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#include "videoprocwnd.hpp"
#include "constants.hpp"
#include "cv_extension.hpp"
#include "cvmat.hpp"
#include "dft2d.hpp"
#include "document.hpp"
#include "player.hpp"
#include "processor.hpp"
#include "recorder.hpp"
#include <adcv/applycolormap.hpp>
#include <adcv/cvtypes.hpp>
#include <adcv/imagewidget.hpp>
#include <adplot/chromatogramwidget.hpp>
#include <adplot/spanmarker.hpp>
#include <adcv/spectrogramplot.hpp>
#include <adportable/debug.hpp>
#include <adportable/float.hpp>
#include <adportfolio/folder.hpp>
#include <adportfolio/folium.hpp>
#include <adportfolio/portfolio.hpp>
#include <adwidgets/playercontrols.hpp>
#include <adwidgets/progresswnd.hpp>
#include <qtwrapper/font.hpp>
#include <qtwrapper/progresshandler.hpp>
#include <coreplugin/minisplitter.h>
#include <coreplugin/progressmanager/progressmanager.h>
#include <utils/styledbar.h>
#include <QBoxLayout>
#include <QCoreApplication>
#include <QDebug>
#include <QEvent>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QPainter>
#include <QPrinter>
#include <QSignalBlocker>
#include <opencv2/core/core.hpp>
#include <boost/any.hpp>
#include <boost/exception/all.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <algorithm>
#include <condition_variable>
#include <iostream>
#include <limits>
#include <numeric>
#include <mutex>
#include <thread>

namespace video {

    class VideoProcWnd::impl {
    public:
        std::array< std::unique_ptr< adcv::ImageWidget >, 2 > imgWidgets_;
        std::unique_ptr< adcv::SpectrogramPlot > map_;
        std::unique_ptr< adplot::ChromatogramWidget > tplot_;
        std::unique_ptr< adplot::SpanMarker > tplotMarker_;
    };

}

using namespace video;

VideoProcWnd::~VideoProcWnd()
{
    delete impl_;
}

VideoProcWnd::VideoProcWnd( QWidget *parent ) : QWidget( parent )
                                              , impl_( new impl() )
{
    setContextMenuPolicy( Qt::CustomContextMenu );

    if ( auto splitter = new Core::MiniSplitter ) {

        if ( auto splitter2 = new Core::MiniSplitter ) {

            for ( auto& widget: impl_->imgWidgets_ ) {
                widget = std::make_unique< adcv::ImageWidget >( this );
                splitter2->addWidget( widget.get() );
            }

            if (( impl_->map_ = std::make_unique< adcv::SpectrogramPlot >( this ) )) {
                splitter2->addWidget( impl_->map_.get() );
            }

            splitter2->setOrientation( Qt::Horizontal );
            splitter2->setStretchFactor( 0, 1 );
            splitter2->setStretchFactor( 1, 1 );
            splitter2->setStretchFactor( 2, 1 );
            splitter->addWidget( splitter2 );
        }

        if ( auto toolBar = new Utils::StyledBar ) {
            auto tbLayout = new QHBoxLayout( toolBar );
            tbLayout->setMargin( 0 );
            tbLayout->setSpacing( 0 );

            if ( auto widget = new adwidgets::PlayerControls() ) {
                using adwidgets::PlayerControls;
                // start new .mp4 file
                connect( widget, &PlayerControls::play, this, [=](){
                    ADDEBUG() << "play";
                    document::instance()->player()->Play();
                    widget->setState( QMediaPlayer::PlayingState );
                });

                connect( widget, &PlayerControls::pause, this, [=](){
                    ADDEBUG() << "pause/stop";
                    document::instance()->player()->Stop();
                    widget->setState( QMediaPlayer::PausedState );
                });

                connect( widget, &PlayerControls::stop, this, [=](){
                    ADDEBUG() << "stop";
                    document::instance()->player()->Stop();
                    widget->setState( QMediaPlayer::StoppedState );
                });
                tbLayout->addWidget( widget );
            }
            splitter->addWidget( toolBar );
        }

        if ( ( impl_->tplot_ = std::make_unique< adplot::ChromatogramWidget >( this ) ) ) {
            impl_->tplot_->setMaximumHeight( 120 );
            connect( impl_->tplot_.get(), SIGNAL( onSelected( const QRectF& ) ), this, SLOT( handleSelectedOnTime( const QRectF& ) ) );
            splitter->addWidget( impl_->tplot_.get() );
            splitter->setOrientation( Qt::Vertical );
            if ( ( impl_->tplotMarker_ =
                   std::make_unique< adplot::SpanMarker >( QColor( 0xff, 0xa5, 0x00, 0x80 ), QwtPlotMarker::VLine, 2.5 ) ) ) { // orange
                impl_->tplotMarker_->setXValue( 0.0, 0.0 );
                impl_->tplotMarker_->attach( impl_->tplot_.get() );
            }
            impl_->tplot_->installEventFilter( this );
        }

        auto layout = new QVBoxLayout( this );
        layout->setMargin( 0 );
        layout->setSpacing( 0 );
        layout->addWidget( splitter );
    }

    connect( impl_->imgWidgets_[0].get(), &adcv::ImageWidget::onZoom, this, [&]( double scale ){
            emit onZoom( scale );
        });
    // connect( impl_->imgWidgets_[1].get(), &adcv::ImageWidget::onZoom, impl_->imgWidgets_[0].get(), &adcv::ImageWidget::handleZoom );
    impl_->imgWidgets_[ 0 ]->sync( impl_->imgWidgets_[ 1 ].get() );
    impl_->imgWidgets_[ 1 ]->sync( impl_->imgWidgets_[ 0 ].get() );

    connect( document::instance(), &document::fileChanged, this, &VideoProcWnd::handleFileChanged );
    connect( document::instance()->player(), &Player::dataChanged, this, &VideoProcWnd::handleData );
    connect( this, &VideoProcWnd::nextFrame, this, &VideoProcWnd::handleNextFrame );
}

void
VideoProcWnd::print( QPainter& painter, QPrinter& printer )
{
    QRectF rc0( 0.0,                 0.0, printer.width() / 2, printer.height() );
    QRectF rc1( printer.width() / 2, 0.0, printer.width() / 2, printer.height() );

    impl_->imgWidgets_.at( 0 )->graphicsView()->render( &painter, rc0 ); //, drawRect1 );
    impl_->imgWidgets_.at( 1 )->graphicsView()->render( &painter, rc1 ); // , drawRect2 );
}

void
VideoProcWnd::handleZoomScale( double scale )
{
    impl_->imgWidgets_.at( 0 )->handleZoom( scale );
    impl_->imgWidgets_.at( 1 )->handleZoom( scale );
}

void
VideoProcWnd::handleFileChanged( const QString& name )
{
    document::instance()->currentProcessor()->reset();
    document::instance()->currentProcessor()->set_filename( name.toStdString() );

    boost::filesystem::path path( name.toStdString() );
    boost::filesystem::path record_name = path.parent_path() / path.stem();
    record_name += "_canny.mp4";
    if ( auto processor = document::instance()->currentProcessor() ) {
        if ( boost::filesystem::exists( record_name ) )
            boost::filesystem::remove( record_name );
#if 0
        if ( auto recorder = processor->create_recorder() )
            recorder->open( record_name.string()
                            , document::instance()->player()->frameRate(), document::instance()->player()->frameSize(), true );
#endif
    }

    document::instance()->player()->Play();

    if ( auto controls = findChild< adwidgets::PlayerControls * >() ) {
        controls->setState( QMediaPlayer::PlayingState );
        boost::filesystem::path path( name.toStdString() );
        controls->setName( QString::fromStdString( path.filename().string() ) );
    }

}

void
VideoProcWnd::handlePlayer( QImage img )
{
    if ( !img.isNull() ) {

        impl_->imgWidgets_.at( 0 )->setImage( img );

        auto player = document::instance()->player();

        if ( auto controls = findChild< adwidgets::PlayerControls * >() ) {

            controls->setPos( double( player->currentFrame() ) / player->numberOfFrames() );
            controls->setTime( double( player->currentTime() ) );
        }

    }
}

void
VideoProcWnd::handleData()
{
    auto processor = document::instance()->currentProcessor();
    // processor->reset();

    auto player = document::instance()->player();

    while ( auto tuple = player->fetch() ) {

        const size_t pos_frames = std::get< 0 >( *tuple );
        const double pos = std::get< 1 >( *tuple );
        const cv::Mat& mat = std::get< 2 >( *tuple );
        impl_->imgWidgets_.at( 0 )->setImage( Player::toImage( mat ) );

        if ( mat.empty() ) {
            continue;
        } else {
            processor->addFrame( pos_frames, pos, mat );
        }
        //----->
#if __cplusplus >= 201703L
        auto [ average, n ] = processor->avg();
#else // __cplusplus < 201703L
        const cv::Mat * average;
        size_t n;
        std::tie( average, n ) = processor->avg();
#endif
        if ( average ) {
            auto avg = adcv::ApplyColorMap_< cv::Mat >()( *average, 8.0 / n );
            impl_->imgWidgets_.at( 1 )->setImage( Player::toImage( avg ) );
        }

        //<-----  contour plot -----
        if ( auto drawable = processor->contours() ) {
            processor->record( std::get< 2 >( *drawable ) );
            //imgWidgets_.at( 1 )->setImage( Player::toImage( std::get< 2 >(*drawable) ) );
        }
    }

    // if ( auto tic = processor->time_profile_tic() )
    //     tplot_->setData( std::move( tic ), 0, false );

    // tplot_->enableAxis( QwtPlot::yRight, true );
    // tplot_->setAxisScale( QwtPlot::yRight, 0, 200 );
    // if ( auto bp = processor->time_profile_bp() )
    //     tplot_->setData( std::move( bp ), 1, false );

    if ( auto counts = processor->time_profile_counts() )
        impl_->tplot_->setData( std::move( counts ), 1, false );

    if ( player->isStopped() ) {
        processor->close_recorder();
    }
}

void
VideoProcWnd::handleSelectedOnTime( const QRectF& rc )
{
    ADDEBUG() << "handle selcted on time";
    qDebug() << rc;
    if ( auto processor = document::instance()->currentProcessor() ){
        auto [ average, n ] = processor->avg();
        if ( average ) {
            auto avg = adcv::ApplyColorMap_< cv::Mat >()( *average, 8.0 / n );
            impl_->map_->setData( avg );
        }
    }
}

void
VideoProcWnd::handleNextFrame( bool forward )
{
    if ( auto processor = document::instance()->currentProcessor() ) {
        auto frame_pos = processor->next_frame_pos( forward );
        if ( auto frame = processor->frame( frame_pos ) ) {
            auto [ fpos, pos, mat ] = *frame;
            impl_->imgWidgets_.at( 0 )->setImage( Player::toImage( mat ) );
            impl_->tplotMarker_->setXValue( pos, pos );
            impl_->tplot_->replot();

            ///
            impl_->map_->setData( mat );
        }
        if ( auto canny = processor->contours( frame_pos ) ) {
            //if ( auto canny = processor->canny( frame_pos ) ) {
            auto [ fpos, pos, mat ] = *canny;
            impl_->imgWidgets_.at( 1 )->setImage( Player::toImage( mat ) );
        }
    }
}

bool
VideoProcWnd::eventFilter( QObject * object, QEvent * event )
{
    if ( event->type() == QEvent::KeyPress ) {
        if ( object == impl_->tplot_.get() ) {
            if ( QKeyEvent * keyEvent = static_cast< QKeyEvent * >( event ) ) {
                switch ( keyEvent->key() ) {
                case Qt::Key_Left:  emit nextFrame( false ); break;
                case Qt::Key_Right: emit nextFrame( true ); break;
                }
            }
        }
    }
    return QWidget::eventFilter( object, event );
}

void
VideoProcWnd::handleZAutoScaleEnabled( bool enable )
{
    if ( enable ) {
        impl_->map_->setAxisZMax( -1 );
    } else {
        impl_->map_->setAxisZMax( document::instance()->zScale() );
    }
}

void
VideoProcWnd::handleZScale( int scale )
{
    if ( ! document::instance()->zScaleAutoEnabled() )
        impl_->map_->setAxisZMax( scale );
}
