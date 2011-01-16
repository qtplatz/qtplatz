// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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
#include <boost/smart_ptr.hpp>

namespace portfolio {
    class Folium;
}

namespace dataproc {

    class Dataprocessor;
    class DataprocPlugin;
    
    namespace internal {
        class Session;
        class DataprocPlugin;
    }

    class Session {
    public:
        ~Session();
        Session();
        Session( const Session& );
        Session( boost::shared_ptr<Dataprocessor>& );
        Dataprocessor& getDataprocessor();
    private:
        boost::shared_ptr< Dataprocessor > processor_;  // holds a file
    };

    class SessionManager : public QObject {
        Q_OBJECT
        explicit SessionManager(QObject *parent = 0);
        friend internal::DataprocPlugin;
    public:
        ~SessionManager();

        static SessionManager * instance();

        Dataprocessor * getActiveDataprocessor();
        void addDataprocessor( boost::shared_ptr<Dataprocessor>& );
        void updateDataprocessor( Dataprocessor *, portfolio::Folium& );

        typedef std::vector< Session > vector_type;

        vector_type::iterator begin();
        vector_type::iterator end();

    signals:
        void signalSessionAdded( Dataprocessor * );
        void signalSessionUpdated( Dataprocessor * );
        void signalSelectionChanged( Dataprocessor *, portfolio::Folium& );

    public slots:
        void selectionChanged( Dataprocessor *, portfolio::Folium& );

    private:
        static SessionManager * instance_;
        std::vector< Session > sessions_;
        Dataprocessor * activeDataprocessor_;
    };

}

#endif // SESSIONMANAGER_H
