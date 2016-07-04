/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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
#include "adextension_global.hpp"
#include <memory>

namespace adprocessor { class dataprocessor; }
namespace portfolio { class Folium; class Folder; }
    
namespace adextension {

    class ADEXTENSIONSHARED_EXPORT iSessionManager : public QObject {
        Q_OBJECT

    public:
        explicit iSessionManager(QObject *parent = 0);

        virtual std::shared_ptr< adprocessor::dataprocessor > getDataprocessor( const QString& ) { return nullptr; }
        
    signals:
        void addProcessor( iSessionManager *, const QString& );                                       // file open
        void processorSelectionChanged( iSessionManager *, const QString& file );                     // change file focus
        void folderSelectionChanged( iSessionManager *, const QString& file, const QString& folder ); // change focused folder
        void onDataChanged( iSessionManager *, const QString& file, const portfolio::Folium& );       // data contents changed
        void onCheckStateChanged( iSessionManager *, const QString& file, portfolio::Folium&, bool );

    public slots:

    };

}

