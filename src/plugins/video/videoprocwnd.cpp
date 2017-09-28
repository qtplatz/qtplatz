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
#include "cv_extension.hpp"
#include "cvmat.hpp"
#include "dft2d.hpp"
#include "document.hpp"
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
#if HAVE_ARRAYFIRE
# include <advision/aftypes.hpp>
#endif
#include <advision/applycolormap.hpp>
#include <advision/cvtypes.hpp>
#include <advision/imagewidget.hpp>
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
                widget = std::make_unique< advision::ImageWidget >( this );
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
    cv::Mat mat;
    cv::Mat avg;
    
    auto player = document::instance()->player();
    
    if ( player->fetch( mat ) ) {
        cv::Mat_< uchar > gray8u;
        cv::cvtColor( mat, gray8u, cv::COLOR_BGR2GRAY );
        
        cv::Mat_< float > gray32f( mat.rows, mat.cols );

        gray8u.convertTo( gray32f, CV_32FC(1), 1.0/255 ); // 0..1.0 float gray scale

        if ( !average_ ) {
            average_ = std::make_unique< cv::Mat_< float > >( gray32f );
            numAverage_ = 1;
        } else {
            *average_ += gray32f;
            numAverage_++;
        }

        if ( average_ ) {
            avg = advision::ApplyColorMap_< cv::Mat >()( *average_, 8.0 / numAverage_ );
        }

        if ( auto controls = findChild< PlayerControls * >() ) {
            controls->setPos( double( player->currentFrame() ) / player->numberOfFrames() );
            controls->setTime( double( player->currentTime() ) );
        }
        
        imgWidgets_.at( 0 )->setImage( Player::toImage( mat ) );
        imgWidgets_.at( 1 )->setImage( Player::toImage( avg ) );
    }
}



