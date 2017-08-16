/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
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
#if HAVE_CUDA
# include "cudacolormap.hpp"
#endif
#include "cv_extension.hpp"
#include "cvmat.hpp"
#include "dft2d.hpp"
#include "document.hpp"
#include "imagewidget.hpp"
#include "player.hpp"
#include "playercontrols.hpp"
#include <opencv2/core/core.hpp>
#include <utils/styledbar.h>
#include <adportable/debug.hpp>
#include <adportable/float.hpp>
#include <adportfolio/portfolio.hpp>
#include <adportfolio/folder.hpp>
#include <adportfolio/folium.hpp>
#include <adplot/chromatogramwidget.hpp>
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
                                              , tplot_( std::make_unique< adplot::ChromatogramWidget >() )
{
    setContextMenuPolicy( Qt::CustomContextMenu );
    
    if ( auto splitter = new Core::MiniSplitter ) {

        if ( auto splitter2 = new Core::MiniSplitter ) {

            for ( auto& widget: imgWidgets_ ) {
                widget = std::make_unique< ImageWidget >( this );
                splitter2->addWidget( widget.get() );
            }
            splitter2->setOrientation( Qt::Horizontal );

            splitter->addWidget( splitter2 );
        }

        if ( auto toolBar = new Utils::StyledBar ) {
            auto tbLayout = new QHBoxLayout( toolBar );
            tbLayout->setMargin( 0 );
            tbLayout->setSpacing( 0 );

            if ( auto widget = new PlayerControls() ) {
                // start new .mp4 file
                connect( widget, &PlayerControls::play, this, [=](){
                        document::instance()->player()->Play();
                        widget->setState( QMediaPlayer::PlayingState );
                    });

                connect( widget, &PlayerControls::pause, this, [=](){
                        document::instance()->player()->Stop();
                        widget->setState( QMediaPlayer::PausedState );
                    });
                
                connect( widget, &PlayerControls::stop, this, [=](){
                        document::instance()->player()->Stop();
                        average_.reset();
                        widget->setState( QMediaPlayer::StoppedState );
                    });
                
                tbLayout->addWidget( widget );
            }
            splitter->addWidget( toolBar );
        }

        tplot_->setMaximumHeight( 120 );

        splitter->addWidget( tplot_.get() );
        splitter->setOrientation( Qt::Vertical );

        auto layout = new QVBoxLayout( this );
        layout->setMargin( 0 );
        layout->setSpacing( 0 );
        layout->addWidget( splitter );
    }

    connect( document::instance(), &document::fileChanged, this, &VideoProcWnd::handleFileChanged );
    // connect( document::instance()->player(), &Player::processedImage, this, &VideoProcWnd::handlePlayer );
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
    document::instance()->player()->Play();
    average_.reset();

    if ( auto controls = findChild< PlayerControls * >() ) {
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

        if ( auto controls = findChild< PlayerControls * >() ) {
            controls->setPos( double( player->currentFrame() ) / player->numberOfFrames() );
            controls->setTime( double( player->currentTime() ) );
        }
    }
}

void
VideoProcWnd::handleData()
{
    typedef cv_extension::mat_t< float, 1 > average_data_t;
    typedef cv_extension::mat_t< uint8_t, 3 > image_data_t;

    cv::Mat mat;
    image_data_t avg;
    
    auto player = document::instance()->player();
    
    if ( player->fetch( mat ) ) {
        cv::Mat_< uchar > gray;
        cv::cvtColor( mat, gray, cv::COLOR_BGR2GRAY );
        
        average_data_t gs( mat.rows, mat.cols );

        gray.convertTo( gs, average_data_t::type_value, 1.0/255 );

        if ( !average_ ) {
            average_ = std::make_unique< average_data_t >( gs );
            numAverage_ = 1;
        } else {
            *average_ += gs;
            numAverage_++;
        }

        if ( average_ ) {
            //avg = cvColor()( *average_, 8.0/numAverage_ );
#if HAVE_CUDA
            cudaApplyColorMap( *average_, avg, 8.0 / numAverage_ );
#else
            average_->convertTo( avg, image_data_t::type_value, 255.0 / numAverage_ );
            cv::applyColorMap( avg, avg, cv::COLORMAP_JET );
#endif
        }

        if ( auto controls = findChild< PlayerControls * >() ) {
            controls->setPos( double( player->currentFrame() ) / player->numberOfFrames() );
            controls->setTime( double( player->currentTime() ) );
        }
        
        imgWidgets_.at( 0 )->setImage( Player::toImage( mat ) );
        imgWidgets_.at( 1 )->setImage( Player::toImage( avg ) );
    }
}



