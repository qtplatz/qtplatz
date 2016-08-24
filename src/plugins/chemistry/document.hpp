/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <atomic>
#include <memory>
#include <mutex>

class QSettings;
class QSqlDatabase;
class QTextEdit;

namespace chemistry {

    class ChemConnection;
    class ChemQuery;

    class document : public QObject
    {
        Q_OBJECT

        explicit document(QObject *parent = 0);
        static std::atomic< document * > instance_;
        static std::mutex mutex_;
    public:
        static document * instance();
        
        void initialSetup();
        void finalClose();

        QSettings * settings();

        void setQuery( ChemQuery * );
        ChemQuery * query();
        //

        QSqlDatabase sqlDatabase();

        void setChemSpiderToken( const QString& );
        QString chemSpiderToken() const;

    private:
        struct impl;

        void dbInit( ChemConnection * );
        void dbUpdate( ChemConnection * );

    signals:
        void onConnectionChanged();
        void databaseModified();
                                  
    public slots:
        void ChemSpiderSearch( const QString&, QTextEdit * );
    };

}


