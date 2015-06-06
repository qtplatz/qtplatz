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

#ifndef QUERYDOCUMENT_HPP
#define QUERYDOCUMENT_HPP

#include <array>
#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <QObject>

namespace boost { namespace filesystem { class path; } }

class QSettings;

namespace query {
    
    namespace detail { class dataWriter; }

    class QueryConnection;

    class QueryDocument : public QObject {
        Q_OBJECT
    private:
        ~QueryDocument();
        QueryDocument();
        static std::atomic< QueryDocument * > instance_;
        static std::mutex mutex_;
    public:
        static QueryDocument * instance();

        void register_dataChanged( std::function< void( int, bool ) > );
        void setResultFile( const std::wstring& );
        void mslock_enabled( bool );

        void setConnection( QueryConnection * );
        QueryConnection * connection();

        void onInitialUpdate();
        void onFinalClose();

        //QSettings * settings() { return settings_.get(); }
        std::shared_ptr< QSettings > settings_ptr() { return settings_; }

    private:
        void addRecentFiles( const QString& group, const QString& key, const QString& value );
        void getRecentFiles( const QString& group, const QString& key, std::vector<QString>& list ) const;
        QString recentFile( const QString& group, const QString& key ) const;

        std::shared_ptr< QSettings > settings_;

    signals:
        void onConnectionChanged();
    };
}

#endif // QUERYDOCUMENT_HPP
