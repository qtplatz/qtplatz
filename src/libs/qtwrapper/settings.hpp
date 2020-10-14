// This is a -*- C++ -*- header.
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

#pragma once

#include <QString>
#include <QSettings>
#include "qtwrapper_global.h"
#include <tuple>

namespace qtwrapper {

    struct QTWRAPPERSHARED_EXPORT settings {
        QSettings& settings_;
        settings( QSettings& settings );

        QString recentFile( const QString& group, const QString& key ) const;
        void addRecentFiles( const QString& group, const QString& key, const QString& value );
        void getRecentFiles( const QString& group, const QString& key, std::vector<QString>& list ) const;

    };

    template< typename T > std::pair< QString, T > keyValue;

    struct setValue_t {
        QSettings& settings_;
        setValue_t( QSettings& settings, const QString& group ) : settings_( settings ) {
            settings_.beginGroup( group );
        }
        ~setValue_t() {
            settings_.endGroup();
        }
        template< typename T > setValue_t& operator << ( std::pair< QString, T>&& t ) {
            settings_.setValue( t.first, t.second );
            return *this;
        };
        template< typename T> setValue_t& operator()( std::pair< QString, T>&& t ) {
            settings_.setValue( t.first, t.second );
            return *this;
        };
    };
}
