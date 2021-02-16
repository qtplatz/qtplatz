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

#include "document.hpp"
#include "constants.hpp"
#include "player.hpp"
#include "processor.hpp"
#include <adcontrols/processmethod.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/msreferences.hpp>
#include <adcontrols/msreference.hpp>
#include <adcontrols/datareader.hpp>
#include <adextension/isessionmanager.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/sqlite.hpp>
#include <adlog/logger.hpp>
#include <adportfolio/folium.hpp>
#include <adportable/profile.hpp>
#include <adportable/debug.hpp>
#include <adprocessor/dataprocessor.hpp>
#include <qtwrapper/settings.hpp>
#include <qtwrapper/waitcursor.hpp>
#include <qtwrapper/progresshandler.hpp>
#include <coreplugin/progressmanager/progressmanager.h>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem.hpp>
#include <boost/exception/all.hpp>
#include <boost/lexical_cast.hpp>
#include <app/app_version.h>
#include <QApplication>
#include <QByteArray>
#include <QFuture>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QMetaType>
#include <QSettings>
#include <algorithm>

Q_DECLARE_METATYPE( portfolio::Folium )

namespace video {
    struct user_preference {
        static boost::filesystem::path path( QSettings * settings ) {
            boost::filesystem::path dir( settings->fileName().toStdWString() );
            return dir.remove_filename() / "video";
        }
    };
}

namespace video {

    struct cannyValue {
        size_t blurSize;
        std::pair< int, int > cannyThreshold;
        size_t maxSizeThreshold;
        size_t minSizeThreshold;
        size_t resize;
        cannyValue() : blurSize( 1 )
                     , cannyThreshold{ 1, 3 }
                     , maxSizeThreshold( 2147483647 )
                     , minSizeThreshold( 1 )
                     , resize( 0 ) {
        }
    };

    struct topToolBarValues {
        int zScale;
        bool zScaleAutoEnabled;
        topToolBarValues() : zScale( 10 )
                           , zScaleAutoEnabled( true ) {
        }
    };

    class document::impl {
    public:
        impl() {}
        ~impl() {}
        std::shared_ptr< processor > processor_;
        cannyValue canny_value_;
        topToolBarValues topToolBarValues_;
    };
}


using namespace video;

std::mutex document::mutex_;

document::~document()
{
    delete impl_;
}

document::document() : settings_( std::make_unique< QSettings >( QSettings::IniFormat, QSettings::UserScope
                                                                 , QLatin1String( Core::Constants::IDE_SETTINGSVARIANT_STR )
                                                                 , QLatin1String( "video" ) ) )

                     , player_( std::make_unique< Player >() )
                     , camera_( std::make_unique< Player >() )
                     , impl_( new impl() )
{
}

document *
document::instance()
{
	static document __instance;
	return &__instance;
}

void
document::initialSetup()
{
    // auto obj = QJsonDocument::fromJson( settings_->value( "topToolBar", "{}" ).toByteArray() ).object();
    // ADDEBUG() << QJsonDocument( obj ).toJson().toStdString();
    impl_->topToolBarValues_.zScaleAutoEnabled = settings_->value( "topToolBar/zScaleAutoEnabled", true ).toBool();
    impl_->topToolBarValues_.zScale = settings_->value( "topToolBar/zScale", 10 ).toInt();
}

void
document::finalClose()
{
}

QSettings&
document::settings()
{
    return *settings_;
}

QString
document::lastDataDir() const
{
    return qtwrapper::settings( *settings_ ).recentFile( Constants::GRP_DATA_FILES, Constants::KEY_FILES );
}

bool
document::openFile( const QString& filename, QString& errorMessage )
{
    if ( player_->loadVideo( filename.toStdString() ) ) {
        emit fileChanged( filename );
        return true;
    } else {
        errorMessage = QString("File '%1' could not be opened" ).arg( filename );
        return false;
    }
}

Player *
document::player()
{
    return player_.get();
}

Player *
document::camera()
{
    return camera_.get();
}

void
document::captureCamera()
{
    camera_->loadCamera( 0 );
    emit cameraChanged();
}

std::shared_ptr< processor >
document::currentProcessor()
{
    if ( ! impl_->processor_ )
        impl_->processor_ = std::make_shared< processor >();
    return impl_->processor_;
}

void
document::setContoursMethod( QString&& json )
{
    auto obj = QJsonDocument::fromJson( json.toUtf8() ).object();
    impl_->canny_value_.blurSize = obj["blurSize"].toInt();
    impl_->canny_value_.cannyThreshold.first = obj["cannyThreshold"].toInt();
    impl_->canny_value_.cannyThreshold.second = obj["cannyThreshold_H"].toInt();
    impl_->canny_value_.maxSizeThreshold = obj["maxSizeThreshold"].toInt();
    impl_->canny_value_.minSizeThreshold = obj["minSizeThreshold"].toInt();
    impl_->canny_value_.resize  = obj["resize"].toInt();
#if 0
    ADDEBUG() << "blurSize: " << impl_->canny_value_.blurSize
              << ", cannyThreshold: " << impl_->canny_value_.cannyThreshold
              << ", maxSizeThreshold: " << impl_->canny_value_.maxSizeThreshold
              << ", minSizeThreshold: " << impl_->canny_value_.minSizeThreshold
              << ", resize: " << impl_->canny_value_.resize;
#endif
}

std::pair< int, int >
document::cannyThreshold() const
{
    return impl_->canny_value_.cannyThreshold;
}

int
document::sizeFactor() const
{
    return impl_->canny_value_.resize;
}

int
document::blurSize() const
{
    return impl_->canny_value_.blurSize;
}

int
document::minSizeThreshold() const
{
    return impl_->canny_value_.minSizeThreshold;
}

int
document::maxSizeThreshold() const
{
    return impl_->canny_value_.maxSizeThreshold;
}

void
document::setZScaleAutoEnabled( bool enable )
{
    impl_->topToolBarValues_.zScaleAutoEnabled = enable;
    settings_->setValue( "topToolBar/zScaleAutoEnabled", enable );
}

void
document::setZScale( int scale )
{
    impl_->topToolBarValues_.zScale = scale;
    settings_->setValue( "topToolBar/zScale", scale );
}

bool
document::zScaleAutoEnabled() const
{
    return impl_->topToolBarValues_.zScaleAutoEnabled;
}

int
document::zScale() const
{
    return impl_->topToolBarValues_.zScale;
}
