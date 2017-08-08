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
#include "imageview.hpp"
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

namespace video {

    class OpenCVWnd::Drawable {
        Drawable( const Drawable& ) = delete;
        Drawable& operator = ( const Drawable& ) = delete;
        cv::Mat mat_;
    public:
        Drawable() {}
        void setImage( ImageView *, const cv::Mat& );
        void draw( ImageView * );
        void zeroFilling( ImageView * );
        void raw( ImageView * );
        void dft( ImageView * );
        void logScale( ImageView * );
        // void setMaxZ( ImageView *, int );
        void blur( ImageView * );
        void x2( ImageView * );
    };

    class OpenCVWnd::impl {
        OpenCVWnd * parent_;
    public:
        impl( OpenCVWnd * p ) : parent_( p )
            {}

        std::array< std::unique_ptr< ImageView >, 2 > map_;
        std::array< OpenCVWnd::Drawable, 2 > drawable_;
    };
}

using namespace video;

OpenCVWnd::~OpenCVWnd()
{
    delete impl_;
}

OpenCVWnd::OpenCVWnd( QWidget *parent ) : QWidget( parent )
                                                , impl_( new impl( this ) )
{
    setContextMenuPolicy( Qt::CustomContextMenu );
    
    if ( auto splitter = new Core::MiniSplitter ) {

        impl_->map_[ 0 ] = std::make_unique< ImageView >( 0, this );
        impl_->map_[ 1 ] = std::make_unique< ImageView >( 1, this );

        for ( auto& map: impl_->map_ ) {

            splitter->addWidget( map.get() );
            
            connect( map.get(), &ImageView::zValue, [&]( ImageView * view, int z ){
                    impl_->drawable_.at( view->index() ).draw( view );
                });

            connect( map.get(), &ImageView::toggled, this, [&]( ImageView * view, const QString& name, bool toggle ){
                    auto& drawable = impl_->drawable_.at( view->index() );
                    if ( name == "RAW" && toggle )
                        drawable.raw( view );
                    if ( name == "DFT" && toggle )
                        drawable.dft( view );
                    if ( name == "0-Fill" && toggle )
                        drawable.zeroFilling( view );
                    if ( name == "Blur" && toggle )
                        drawable.blur( view );
                    if ( name == "x2" && toggle )
                        drawable.x2( view );                    
                    
                    if ( name == "Log" )
                        drawable.logScale( view );
                    if ( name == "Gray" )
                        drawable.draw( view );
                });
        }
        
        splitter->setOrientation( Qt::Horizontal );
        auto layout = new QVBoxLayout( this );
        layout->setMargin( 0 );
        layout->setSpacing( 0 );
        layout->addWidget( splitter );
    }
}

void
OpenCVWnd::handleCheckStateChanged( const portfolio::Folium& folium )
{
}

void
OpenCVWnd::handleDataChanged( const portfolio::Folium& folium )
{
}

void
OpenCVWnd::handleMappedImage()
{
    for ( size_t i = 0; i < impl_->drawable_.size(); ++i ) {
        impl_->drawable_.at( i ).raw( impl_->map_.at( i ).get() );
        impl_->map_.at( i )->resetButtons();
    }
}

void
OpenCVWnd::setImage( const cv::Mat& image )
{
    for ( int i = 0; i < impl_->drawable_.size(); ++i ) {
        impl_->drawable_.at( i ).setImage( impl_->map_.at( i ).get(), image );
    }
}

void
OpenCVWnd::print( QPainter& painter, QPrinter& printer )
{
    QRectF rc0( 0.0,                 0.0, printer.width() / 2, printer.height() );
    QRectF rc1( printer.width() / 2, 0.0, printer.width() / 2, printer.height() );

    impl_->map_.at( 0 )->graphicsView()->render( &painter, rc0 ); //, drawRect1 );
    impl_->map_.at( 1 )->graphicsView()->render( &painter, rc1 ); // , drawRect2 );
    
}

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
void
OpenCVWnd::Drawable::draw( ImageView * view )
{
    bool gray = view->isChecked( "Gray" );
    double z = double( view->z() ) / 1000;

    if ( !mat_.empty() ) {
        view->setImage( cvColor( gray )( mat_, 1.0/z ) );
    }

}

void
OpenCVWnd::Drawable::zeroFilling( ImageView * view )
{
    bool gray = view->isChecked( "Gray" );
 
    // if ( mat_.empty() ) {
    //     if ( auto image = filtered_ ? filtered_ : document::instance()->mappedImage() )
    //         mat_ = cvmat()( *image );
    // }
   
    // if ( !mat_.empty() ) {
    //     mat_ = dft2d().zerofill( mat_, 4 );
    //     view->setImage( cvColor( gray )( mat_ ) );
    // }
}

void
OpenCVWnd::Drawable::raw( ImageView * view )
{
    bool gray = view->isChecked( "Gray" );

    // if ( auto image = document::instance()->mappedImage() ) {
    //     mat_ = cvmat()( *image );
    //     // ADDEBUG() << mat_;
    // }
    // if ( ! mat_.empty() ) {
    //     auto z = *std::max_element( mat_.begin< float >(), mat_.end< float >() );
    //     view->setImage( cvColor( gray )( mat_, 1.0 / z ) );
    // }
}

void
OpenCVWnd::Drawable::dft( ImageView * view )
{
    bool gray = view->isChecked( "Gray" );    

    // if ( mat_.empty() ) {
    //     if ( auto image = filtered_ ? filtered_ : document::instance()->mappedImage() )
    //         mat_ = cvmat()( *image );
    // }
    
    // if ( ! mat_.empty() ) {
    //     mat_ = dft2d().dft( mat_ );
    //     view->setImage( cvColor( gray )( mat_ ) );
    // }
}

void
OpenCVWnd::Drawable::logScale( ImageView * view )
{
    const bool gray = view->isChecked( "Gray" );
    
    // if ( mat_.empty() ) {
    //     if ( auto image = filtered_ ? filtered_ : document::instance()->mappedImage() )
    //          mat_ = cvmat()( *image );
    // }

    // if ( !mat_.empty() ) {
    //     mat_ += cv::Scalar::all(1);                    // switch to logarithmic scale
    //     cv::log(mat_, mat_);
    //     view->setImage( cvColor( gray )( mat_ ) );
    // }
}

// void
// OpenCVWnd::Drawable::setMaxZ( ImageView * view, int z )
// {
//     view->setMaxZ( z );
//     draw( view );
// }

void
OpenCVWnd::Drawable::blur( ImageView * view )
{
    const bool gray = view->isChecked( "Gray" );    

    // if ( mat_.empty() ) {
    //     if ( auto image = filtered_ ? filtered_ : document::instance()->mappedImage() )
    //         mat_ = cvmat()( *image );
    // }

    // if ( mat_.rows < 256 )
    //     cv::resize( mat_, mat_, cv::Size(0,0), 256/mat_.cols, 256/mat_.rows, CV_INTER_LINEAR );
    
    // cv::GaussianBlur( mat_, mat_, cv::Size( 5, 5 ), 0, 0 );
    // view->setImage( cvColor( gray )( mat_ ) );
}

void
OpenCVWnd::Drawable::x2( ImageView * view )
{
    const bool gray = view->isChecked( "Gray" );

    // if ( mat_.empty() ) {
    //     if ( auto image = filtered_ ? filtered_ : document::instance()->mappedImage() )
    //         mat_ = cvmat()( * image );
    // }
    // if ( !mat_.empty() ) {
    //     cv::resize( mat_, mat_, cv::Size(0,0), 2, 2, CV_INTER_LINEAR );
    //     view->setImage( cvColor( gray )( mat_ ) );
    // }
}

void
OpenCVWnd::Drawable::setImage( ImageView * view, const cv::Mat& m )
{
    m.convertTo( mat_, CV_32F, 1.0/255.0 );
    mat_ = m;
    view->setImage( mat_ );
}



