/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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

#include "settings.hpp"
#include <QString>
#include <QSettings>
#include <QFileInfo>

using namespace qtwrapper;
//using namespace qtwrapper::settings;

settings::settings( QSettings& settings ) : settings_( settings )
{
}

QString
settings::recentFile( const QString& group, const QString& key ) const
{
    QString value;

    settings_.beginGroup( group );

    if ( int size = settings_.beginReadArray( key ) ) {
        (void)size;
        settings_.setArrayIndex( 0 );
        value = settings_.value( "File" ).toString();
    }
    settings_.endArray();

    settings_.endGroup();

    return value;
}

void
settings::addRecentFiles( const QString& group, const QString& key, const QString& value )
{
    QFileInfo path( value );

    if ( ! path.exists() || value.isEmpty() )
        return;

    std::vector< QString > list;
    getRecentFiles( group, key, list );

    list.erase( std::remove_if( list.begin(), list.end()
                                , [path] ( const QString& a ){ return path == QFileInfo(a) || a.isEmpty(); } ), list.end() );

    settings_.beginGroup( group );

    settings_.beginWriteArray( key );

    size_t idx(0);
    settings_.setArrayIndex( idx++ );
    settings_.setValue( "File", path.canonicalFilePath() );

    for ( const auto& f: list ) {
        settings_.setArrayIndex( idx++ );
        settings_.setValue( "File", f );
        if ( idx >= 16 )
            break;
    }

    settings_.endArray();

    settings_.endGroup();
}

void
settings::getRecentFiles( const QString& group, const QString& key, std::vector<QString>& list ) const
{
    settings_.beginGroup( group );

    int size = settings_.beginReadArray( key );
    for ( int i = 0; i < size; ++i ) {
        settings_.setArrayIndex( i );
        list.push_back( settings_.value( "File" ).toString() );
    }
    settings_.endArray();

    settings_.endGroup();
}
