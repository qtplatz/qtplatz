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

#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <QObject>
#include <vector>
#include <memory>
#include <adextension/isessionmanager.hpp>

namespace Core { class IEditor; }

namespace portfolio { class Folium; }

namespace adprocessor { class dataprocessor; }

namespace adextension { class iSessionManager; }

namespace dataproc {

    class Dataprocessor;
    class DataprocPlugin;

    class Session {
    public:
        ~Session();
        Session();
        Session( const Session& );
        Session( std::shared_ptr<Dataprocessor>, Core::IEditor* );
		inline Dataprocessor * processor() const { return processor_.get(); }
		inline Core::IEditor * editor() { return editor_; }
    private:
        std::shared_ptr< Dataprocessor > processor_;  // holds a file
        Core::IEditor * editor_;
    };

    ////////////////////////////////
    ////////////////////////////////
    class SessionManager : public adextension::iSessionManager { // public QObject {

        Q_OBJECT

        explicit SessionManager(QObject *parent = 0);
        friend class DataprocPlugin;

        ~SessionManager();
    public:
        static SessionManager * instance();

        Dataprocessor * getActiveDataprocessor();
        void addDataprocessor( std::shared_ptr<Dataprocessor> );
        void updateDataprocessor( Dataprocessor *, portfolio::Folium& );
        void folderChanged( Dataprocessor *, const std::wstring& foldername );
        void checkStateChanged( Dataprocessor *, portfolio::Folium&, bool isChecked );
        void removeEditor( Core::IEditor * );
        void processed( Dataprocessor *, portfolio::Folium& );

        typedef std::vector< Session > vector_type;

        vector_type::iterator begin();
        vector_type::iterator end();
        vector_type::iterator find( const std::wstring& );

    signals:
		void signalAddSession( Dataprocessor * );
        void signalSelectionChanged( Dataprocessor *, portfolio::Folium& );
        void signalCheckStateChanged( Dataprocessor *, portfolio::Folium&, bool );
		void onSessionUpdated( Dataprocessor *, const QString& foliumId );
		void onFolderChanged( Dataprocessor *, const QString& folder );
        void onSessionAdded( Dataprocessor * );
        void onRemoveSession( Dataprocessor * );
        void onSessionRemoved( const QString& filename );
        void onProcessed( Dataprocessor *, portfolio::Folium& );
        void foliumChanged( Dataprocessor *, const portfolio::Folium& );
        void onDataprocessorChanged( Dataprocessor * );

    public slots:
        void selectionChanged( Dataprocessor *, portfolio::Folium& );

    private:
        // iSessionManager
        std::shared_ptr< adprocessor::dataprocessor > getDataprocessor( const QString& ) override;

    private:
        class impl;
        std::unique_ptr< impl > impl_;
    };

}

#endif // SESSIONMANAGER_H
