// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////


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

        void addDataprocessor( boost::shared_ptr<Dataprocessor>& );

        typedef std::vector< Session > vector_type;

        vector_type::iterator begin();
        vector_type::iterator end();

    signals:
        void signalSessionAdded( Dataprocessor * );
        void signalSelectionChanged( Dataprocessor *, portfolio::Folium& );

    public slots:
        void selectionChanged( Dataprocessor *, portfolio::Folium& );


    private:
        static SessionManager * instance_;
        std::vector< Session > sessions_;
    };

}

#endif // SESSIONMANAGER_H
