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
#include <QMessageBox>
#include <QSettings>
#include <QFuture>
#include <QMetaType>
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
    class document::impl {
    public:
        impl() {}
        ~impl() {}
        std::shared_ptr< processor > processor_;
    };
}


using namespace video;

std::mutex document::mutex_;

document::~document()
{
    delete impl_;
}

document::document() : settings_( std::make_unique< QSettings >() )
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
