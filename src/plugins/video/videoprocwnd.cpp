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
#include <opencv2/core/core.hpp>
#include <utils/styledbar.h>
#include <adportable/debug.hpp>
#include <adportable/float.hpp>
#include <adportfolio/portfolio.hpp>
#include <adportfolio/folder.hpp>
#include <adportfolio/folium.hpp>
#include <adplot/chromatogramwidget.hpp>
#include <adwidgets/playercontrols.hpp>
#include <adcv/applycolormap.hpp>
#include <adcv/cvtypes.hpp>
#include <adcv/imagewidget.hpp>
#include <adwidgets/progresswnd.hpp>
#include <qtwrapper/font.hpp>
#include <qtwrapper/progresshandler.hpp>
#include <coreplugin/minisplitter.h>
#include <coreplugin/progressmanager/progressmanager.h>
#include <QBoxLayout>
#include <QCoreApplication>
#include <QEvent>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QPainter>
#include <QPrinter>
#include <QSignalBlocker>
#include <boost/any.hpp>
#include <boost/exception/all.hpp>
#include <boost/format.hpp>
#include <boost/filesystem/path.hpp>
#include <algorithm>
#include <condition_variable>
#include <iostream>
#include <limits>
#include <numeric>
#include <mutex>
#include <thread>

using namespace video;

VideoProcWnd::~VideoProcWnd()
{
}

VideoProcWnd::VideoProcWnd( QWidget *parent ) : QWidget( parent )
{
    setContextMenuPolicy( Qt::CustomContextMenu );

    if ( auto splitter = new Core::MiniSplitter ) {

        if ( auto splitter2 = new Core::MiniSplitter ) {

            for ( auto& widget: imgWidgets_ ) {
                widget = std::make_unique< adcv::ImageWidget >( this );
                splitter2->addWidget( widget.get() );
            }
            splitter2->setOrientation( Qt::Horizontal );
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
                    // average_.reset();
                    widget->setState( QMediaPlayer::StoppedState );
                });
                tbLayout->addWidget( widget );
            }

            splitter->addWidget( toolBar );
        }

        if ( tplot_ = std::make_unique< adplot::ChromatogramWidget >( this ) ) {
            tplot_->setMaximumHeight( 120 );
            connect( tplot_.get(), SIGNAL( onSelected( const QRectF& ) ), this, SLOT( handleSelectedOnTime( const QRectF& ) ) );
            splitter->addWidget( tplot_.get() );
            splitter->setOrientation( Qt::Vertical );
        }

        auto layout = new QVBoxLayout( this );
        layout->setMargin( 0 );
        layout->setSpacing( 0 );
        layout->addWidget( splitter );
    }

    connect( document::instance(), &document::fileChanged, this, &VideoProcWnd::handleFileChanged );
    connect( document::instance()->player(), &Player::dataChanged, this, &VideoProcWnd::handleData );
}

void
VideoProcWnd::print( QPainter& painter, QPrinter& printer )
{
    QRectF rc0( 0.0,                 0.0, printer.width() / 2, printer.height() );
    QRectF rc1( printer.width() / 2, 0.0, printer.width() / 2, printer.height() );

    imgWidgets_.at( 0 )->graphicsView()->render( &painter, rc0 ); //, drawRect1 );
    imgWidgets_.at( 1 )->graphicsView()->render( &painter, rc1 ); // , drawRect2 );

}

void
VideoProcWnd::handleFileChanged( const QString& name )
{
    document::instance()->currentProcessor()->reset();
    document::instance()->currentProcessor()->set_filename( name.toStdString() );
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

        imgWidgets_.at( 0 )->setImage( img );

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
        imgWidgets_.at( 0 )->setImage( Player::toImage( mat ) );

        if ( mat.empty() ) {
            continue;
        } else {
            processor->addFrame( pos_frames, pos, mat );
        }
        //----->
        auto [ average, n ] = processor->avg();
#if 0 // __cplusplus < 201703L
        const cv::Mat * average;
        size_t n;
        std::tie( average, n ) = processor->avg();
#endif
        if ( average ) {
            auto avg = adcv::ApplyColorMap_< cv::Mat >()( *average, 8.0 / n );
            imgWidgets_.at( 1 )->setImage( Player::toImage( avg ) );
        }

        //<-----  contour plot -----
        if ( auto drawable = processor->contours() ) {
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
        tplot_->setData( std::move( counts ), 1, false );
}

void
VideoProcWnd::handleSelectedOnTime( const QRectF& rc )
{
    ADDEBUG() << "handle selcted on time";
}
