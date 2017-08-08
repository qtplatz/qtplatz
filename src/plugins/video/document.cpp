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

#include "document.hpp"
#include "constants.hpp"
#include "player.hpp"
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

using namespace video;

std::mutex document::mutex_;

document::~document()
{
}

document::document() : settings_( std::make_unique< QSettings >() )
                     , player_( std::make_unique< Player >() )
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
        emit playerChanged( filename );
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










