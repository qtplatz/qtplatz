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

#include "videocapturewnd.hpp"
#include "document.hpp"
#include "constants.hpp"
#include "imagewidget.hpp"
#include "player.hpp"
#include "playercontrols.hpp"
#include <adcontrols/mappedspectrum.hpp>
#include <adcontrols/mappedspectra.hpp>
#include <adcontrols/mappedimage.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adinterface/signalobserver.hpp>
#include <adplot/chromatogramwidget.hpp>
#include <adplot/picker.hpp>
#include <adplot/plotcurve.hpp>
#include <adplot/spanmarker.hpp>
#include <adplot/spectrumwidget.hpp>
#include <adplot/zoomer.hpp>
#include <adportable/bzip2.hpp>
#include <adportable/debug.hpp>
#include <adportable/float.hpp>
#include <adportfolio/portfolio.hpp>
#include <adportfolio/folder.hpp>
#include <adportfolio/folium.hpp>
#include <adwidgets/progresswnd.hpp>
#include <qtwrapper/font.hpp>
#include <qtwrapper/progresshandler.hpp>
#include <coreplugin/minisplitter.h>
#include <coreplugin/progressmanager/progressmanager.h>
#include <QBoxLayout>
#include <QCoreApplication>
#include <QEvent>
#include <QImage>
#include <QKeyEvent>
#include <QLabel>
#include <QSignalBlocker>
#include <QTextEdit>
#include <boost/any.hpp>
#include <boost/exception/all.hpp>
#include <boost/format.hpp>
#include <condition_variable>
#include <iostream>
#include <numeric>
#include <mutex>
#include <thread>

using namespace video;

VideoCaptureWnd::~VideoCaptureWnd()
{
}

VideoCaptureWnd::VideoCaptureWnd( QWidget *parent ) : QWidget( parent )
                                                      //, qlabel_( std::make_unique< QLabel >() )
                                                    , view_( std::make_unique< ImageWidget >() )
{
    setContextMenuPolicy( Qt::CustomContextMenu );

    //qlabel_->setAlignment( Qt::AlignCenter );

    if ( auto splitter = new Core::MiniSplitter ) {

        //splitter->addWidget( qlabel_.get() );
        splitter->addWidget( view_.get() );
        
        auto layout = new QVBoxLayout( this );
        layout->setMargin( 0 );
        layout->setSpacing( 0 );
        layout->addWidget( splitter );
    }

    connect( document::instance(), &document::playerChanged, this, &VideoCaptureWnd::handlePlayerChanged );
    
    connect( document::instance()->player(), &Player::processedImage, this, &VideoCaptureWnd::handlePlayer );

    setStyleSheet( "background-color:black;");
}

void
VideoCaptureWnd::handlePlayerChanged( const QString& )
{
    document::instance()->player()->Play();
}

void
VideoCaptureWnd::handlePlayer( QImage img )
{
    if ( !img.isNull() ) {
        //qlabel_->setAlignment( Qt::AlignCenter );
        //qlabel_->setPixmap( QPixmap::fromImage( img ).scaled( qlabel_->size(), Qt::KeepAspectRatio, Qt::FastTransformation ) );
        view_->setImage( img );
    }
}


