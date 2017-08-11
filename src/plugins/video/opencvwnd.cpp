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

#include "opencvwnd.hpp"
#include "document.hpp"
#include "constants.hpp"
#include "cvmat.hpp"
#include "dft2d.hpp"
#include "imagewidget.hpp"
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
#include <condition_variable>
#include <iostream>
#include <numeric>
#include <mutex>
#include <thread>

using namespace video;

OpenCVWnd::~OpenCVWnd()
{
}

OpenCVWnd::OpenCVWnd( QWidget *parent ) : QWidget( parent )
                                        , tplot_( std::make_unique< adplot::ChromatogramWidget >() )
{
    setContextMenuPolicy( Qt::CustomContextMenu );
    
    if ( auto splitter = new Core::MiniSplitter ) {

        for ( auto& widget: imgWidgets_ ) {
            widget = std::make_unique< ImageWidget >( this );
            splitter->addWidget( widget.get() );
        }
        
        splitter->setOrientation( Qt::Horizontal );
        auto layout = new QVBoxLayout( this );
        layout->setMargin( 0 );
        layout->setSpacing( 0 );
        layout->addWidget( splitter );
    }
}

void
OpenCVWnd::print( QPainter& painter, QPrinter& printer )
{
    QRectF rc0( 0.0,                 0.0, printer.width() / 2, printer.height() );
    QRectF rc1( printer.width() / 2, 0.0, printer.width() / 2, printer.height() );

    imgWidgets_.at( 0 )->graphicsView()->render( &painter, rc0 ); //, drawRect1 );
    imgWidgets_.at( 1 )->graphicsView()->render( &painter, rc1 ); // , drawRect2 );
    
}

