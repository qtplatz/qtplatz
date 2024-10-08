/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "mainwindow.hpp"
#include <app/app_version.h>
#include <QApplication>
#include <QSettings>
#include <QDir>

int
main( int argc, char *argv[] )
{
    QApplication a(argc, argv);

    std::shared_ptr< QSettings > settings
        = std::make_shared< QSettings >( QSettings::IniFormat, QSettings::UserScope
                                        , QLatin1String( Core::Constants::IDE_SETTINGSVARIANT_STR )
                                        , QLatin1String( "publisher" ) );

    QDir appdir( QApplication::applicationDirPath() + QLatin1String( "/../share/qtplatz/xslt" ) );
    MainWindow::addRecentFiles( *settings, "Stylesheets", "DIRS", appdir.canonicalPath(), "DIR" );

    MainWindow w;
    w.resize( 800, 600 );
    w.show();
    w.onInitialUpdate( settings );
    return a.exec();
}
