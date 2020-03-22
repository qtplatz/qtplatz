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
// #include <mpxwidgets/spectrogramplot.hpp>
// #include <mpxcontrols/dataframe.hpp>
// #include <mpxcontrols/population_protocol.hpp>
// #include <mpxprocessor/processor.hpp>
#include <adcontrols/contoursmethod.hpp>
#include <adcontrols/mappedspectrum.hpp>
#include <adcontrols/mappedspectra.hpp>
#include <adcontrols/mappedimage.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adfs/sqlite.hpp>
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
#include <adprocessor/dataprocessor.hpp>
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
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <cmath>
#include <condition_variable>
#include <iostream>
#include <numeric>
#include <mutex>
#include <thread>

namespace cluster {

    class OpenCVWnd::Drawable {
        Drawable( const Drawable& ) = delete;
        Drawable& operator = ( const Drawable& ) = delete;
        std::shared_ptr< adfs::sqlite > db_;
        cv::Mat mat_;
    public:
        Drawable() {}
        void setData( const cv::Mat& );
        void setImage( ImageView *, const cv::Mat& );
        void draw( ImageView * );
        void zeroFilling( ImageView * );
        void raw( ImageView * );
        void dft( ImageView * );
        void logScale( ImageView * );
        void blur( ImageView * );
        void x2( ImageView * );
        void Contours( ImageView * );
        void handleProcessorChanged();
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

using namespace cluster;

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
                    if ( name == "Contours" && toggle )
                        drawable.Contours( view );

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
OpenCVWnd::handleProcessorChanged()
{
    for ( auto& d: impl_->drawable_ )
        d.handleProcessorChanged();
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
OpenCVWnd::handleContoursMethodChanged()
{
    for ( size_t i = 0; i < impl_->drawable_.size(); ++i ) {
        auto& view = impl_->map_.at( i );
        if ( view->isChecked( "Contours" ) ) {
            view->setChecked( "Logging", false ); // force disable to save results into _contours.db file
            impl_->drawable_.at( i ).Contours( view.get() );
        }
    }
}

void
OpenCVWnd::handleMappedImage()
{
    for ( size_t i = 0; i < impl_->drawable_.size(); ++i ) {
        auto& view = impl_->map_.at( i );

        if ( auto image = document::instance()->mappedImage() ) {

            impl_->drawable_.at( i ).setData( cvmat()( *image ) );

            if ( view->isChecked( "Contours" ) )
                impl_->drawable_.at( i ).Contours( impl_->map_.at( i ).get() );
            else
                impl_->drawable_.at( i ).raw( impl_->map_.at( i ).get() );
        }
        // impl_->map_.at( i )->resetButtons();
    }
}

void
OpenCVWnd::setImage( const cv::Mat& image )
{
    if ( image.rows > 256 || image.cols > 256 ) {
        int sf = std::max( image.rows / 256, image.cols / 256 );
    }
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

    if ( !mat_.empty() ) {
        mat_ = dft2d().zerofill( mat_, 4 );
        view->setImage( cvColor( gray )( mat_ ) );
    }
}

void
OpenCVWnd::Drawable::setData( const cv::Mat& m )
{
    mat_ = m;
}

void
OpenCVWnd::Drawable::raw( ImageView * view )
{
    bool gray = view->isChecked( "Gray" );

    if ( ! mat_.empty() ) {
        auto z = *std::max_element( mat_.begin< float >(), mat_.end< float >() );
        view->setImage( cvColor( gray )( mat_, 1.0 / z ) );
    }
}

void
OpenCVWnd::Drawable::dft( ImageView * view )
{
    bool gray = view->isChecked( "Gray" );

    if ( ! mat_.empty() ) {
        mat_ = dft2d().dft( mat_ );
        view->setImage( cvColor( gray )( mat_ ) );
    }
}

void
OpenCVWnd::Drawable::logScale( ImageView * view )
{
    const bool gray = view->isChecked( "Gray" );

    if ( !mat_.empty() ) {
        mat_ += cv::Scalar::all(1);                    // switch to logarithmic scale
        cv::log(mat_, mat_);
        view->setImage( cvColor( gray )( mat_ ) );
    }
}

void
OpenCVWnd::Drawable::blur( ImageView * view )
{
    const bool gray = view->isChecked( "Gray" );

    int szFactor = document::instance()->contoursMethod().sizeFactor();
    int blurSize = std::max( 1, document::instance()->contoursMethod().blurSize() );

    cv::Mat sized;
    cv::resize( mat_, sized, cv::Size(0,0), szFactor, szFactor, cv::INTER_LINEAR );

    cv::GaussianBlur( sized, sized, cv::Size( blurSize, blurSize ), 0, 0 );
    view->setImage( cvColor( gray )( sized ) );
}

void
OpenCVWnd::Drawable::x2( ImageView * view )
{
    const bool gray = view->isChecked( "Gray" );

    if ( !mat_.empty() ) {
        cv::resize( mat_, mat_, cv::Size(0,0), 2, 2, cv::INTER_LINEAR );
        view->setImage( cvColor( gray )( mat_ ) );
    }
}

void
OpenCVWnd::Drawable::handleProcessorChanged()
{
    db_.reset();
}

void
OpenCVWnd::Drawable::Contours( ImageView * view )
{
    if ( view->isChecked( "Logging" ) ) {
        boost::filesystem::path path = document::instance()->currentProcessor()->filename();
        path = path.parent_path() / ( path.stem().string() + "_contours.db" );

        if ( ! db_ ) {
            auto db = std::make_shared< adfs::sqlite >();
            if ( db->open( path.string().c_str() ) ) {
                ADDEBUG() << "-------------> Logging: " << path.string();
                auto sql = adfs::stmt( *db );
                if ( sql.exec( "PRAGMA synchronous = OFF" ) &&
                     sql.exec( "PRAGMA journal_mode = MEMORY" ) &&
                     sql.exec( "DROP TABLE IF EXISTS tof" ) &&
                     sql.exec( "DROP TABLE IF EXISTS contours" ) &&
                     sql.exec( "CREATE TABLE IF NOT EXISTS contours ("
                               " trigNumber INTEGER"
                               ",trigCounts INTEGER"
                               ",contourId INTEGER"
                               ",area REAL"
                               ",cx REAL"
                               ",cy REAL"
                               ",width REAL"
                               ",height REAL"
                               ",PRIMARY KEY(trigNumber,contourId)"
                               ");" ) &&
                     sql.exec( "CREATE TABLE IF NOT EXISTS tof ("
                               " trigNumber INTEGER"
                               ",contourId INTEGER"
                               ",time REAL"
                               ",FOREIGN KEY(trigNumber,contourId) REFERENCES contours(trigNumber,contourId)"
                               ");" ) )
                    db_ = db;
                else
                    ADDEBUG() << "sqlite error";
            }
        }
    } else {
        if ( db_ )
            db_.reset();
    }

    auto image = document::instance()->mappedImage();
    auto spectra = document::instance()->mappedSpectra();

    if ( ! mat_.empty() ) {
        cv::Mat canny;
        cv::Mat blur;
        cv::Mat gray;
        int thresh = document::instance()->contoursMethod().cannyThreshold();
        int szFactor = std::max( 1, document::instance()->contoursMethod().sizeFactor() );
        int blurSize = document::instance()->contoursMethod().blurSize();

        // ADDEBUG() << "cannyThreshold:" << thresh << ", size:" << szFactor << ", blur:" << blurSize;
        try {
            if ( szFactor > 1 ) {
                cv::resize( mat_, gray, cv::Size(0,0), szFactor, szFactor, cv::INTER_LINEAR );
                gray.convertTo( gray, CV_8UC1, 255 );
            } else {
                mat_.convertTo( gray, CV_8UC1, 255 );
            }
        } catch ( cv::Exception& e ) {
            ADDEBUG() << e.what();
        }

        try {
            if ( blurSize >= 1 )
                cv::blur( gray, blur, cv::Size( blurSize, blurSize ) );
            else
                blur = gray;
        } catch ( cv::Exception& e ) {
            ADDEBUG() << e.what();
        }

        // edge detection
        try {
            cv::Canny( blur, canny, thresh, thresh * 2, 3 );
        } catch ( cv::Exception& e ) {
            ADDEBUG() << e.what();
        }

        std::vector< std::vector< cv::Point > > contours;
        std::vector< cv::Vec4i > hierarchy;

        cv::findContours( canny, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0) );

        cv::Mat drawing = cv::Mat::zeros( canny.size(), CV_8UC3 );

        for( int i = 0; i< contours.size(); i++ )  {
            unsigned c = i + 1;
            cv::Scalar color = cv::Scalar( (c&01)*255, ((c&02)/2)*255, ((c&04)/4)*255 );
            drawContours( drawing, contours, i, color, 1, 8, hierarchy, 0, cv::Point() );

            cv::Moments mu = cv::moments( contours[i], false );
            double cx = ( mu.m10 / mu.m00 ) / szFactor;
            double cy = ( mu.m01 / mu.m00 ) / szFactor;
            double area = cv::contourArea( contours[i] ) / ( szFactor * szFactor );
            cv::Rect rc = boundingRect( contours[i] );
            size_t x = size_t( 0.5 + rc.x / szFactor );
            size_t y = size_t( 0.5 + rc.y / szFactor );
            double width = rc.width / szFactor;
            double height = rc.height / szFactor;

            if ( db_ &&
                 !std::isnan( cx ) &&
                 !std::isnan( cy ) &&
                 ( area < (width * height) ) &&
                 ( ( rc.width / rc.height > 0.5 ) && ( rc.width / rc.height < 2.0 ) ) &&
                 ( ( cx > ( width / 2 ) ) && ( cx < ( image->size1() - ( width / 2 ) ) ) ) &&
                 ( ( cy > ( height / 2) ) && ( cy < ( image->size2() - ( height / 2 ) ) ) ) ) {

                // ADDEBUG() << "include -- area=" << area << "\trect=" << (width*height);

                auto sql = adfs::stmt( *db_ );
                sql.prepare( "INSERT INTO contours (trigNumber,trigCounts,contourId,area,cx,cy,width,height)"
                             "VALUES(?,?,?,?,?,?,?,?)" );
                sql.bind( 1 ) = image->trigRange().first;
                sql.bind( 2 ) = image->trigRange().second - image->trigRange().first + 1;
                sql.bind( 3 ) = i;
                sql.bind( 4 ) = area;
                sql.bind( 5 ) = cx;
                sql.bind( 6 ) = cy;
                sql.bind( 7 ) = width;
                sql.bind( 8 ) = height;
                if ( sql.step() != adfs::sqlite_done )
                    ADDEBUG() << __FUNCTION__ << " : sqlite error.";

                if ( spectra && image->trigRange().second == image->trigRange().first ) { // single trigger data only

                    sql.prepare( "INSERT INTO tof (trigNumber,contourId,time) VALUES(?,?,?)" );
                    sql.bind( 1 ) = image->trigRange().first;
                    sql.bind( 2 ) = i;

                    for ( size_t j = y; j < y + height; ++j ) {
                        for ( size_t k = x; k < x + width; ++k ) {
                            const auto& sp = (*spectra)( j, k );
                            if ( sp.size() ) { // should be 1 or 0
                                // ADDEBUG() << std::make_pair( j, k ) << "size=" << sp.size() << ", " << (sp.time( 0 )*1e6) << ", " << sp.intensity( 0 );
                                sql.bind( 3 ) = sp.time( 0 );
                                if ( sql.step() != adfs::sqlite_done )
                                    ADDEBUG() << __FUNCTION__ << " : sqlite error.";
                                sql.reset();
                            }
                        }
                    }

                }

            } else {
                // ADDEBUG() << "## exclude -- area=" << area << "\trect=" << (width*height);
            }
        }
        view->setImage( drawing );
    }
}

void
OpenCVWnd::Drawable::setImage( ImageView * view, const cv::Mat& m )
{
    m.convertTo( mat_, CV_32F, 1.0/255.0 );
    mat_ = m;
    view->setImage( mat_ );
}
