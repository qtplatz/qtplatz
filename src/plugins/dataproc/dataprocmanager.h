// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <QObject>
#include <boost/smart_ptr.hpp>

class QMainWindow;

namespace adportable {
    class Configuration;
}

namespace adcontrols {
    class datafile;
}

namespace dataproc {

    class Dataprocessor;

    namespace internal {

        class DataprocManagerImpl;

        class DataprocManager : public QObject {
            Q_OBJECT
        public:
            explicit DataprocManager(QObject *parent = 0);

            QMainWindow * mainWindow() const;
            void init( const adportable::Configuration&, const std::wstring& apppath );
            void setSimpleDockWidgetArrangement();
            void OnInitialUpdate();
            void OnFinalClose();
      
        signals:
            void signalUpdateFile( adcontrols::datafile * );
      
        public slots:
            void handleSessionAdded( Dataprocessor * );

        private slots:
            void handleApplyMethod();

        private:
            boost::shared_ptr<DataprocManagerImpl> pImpl_;
        };

    }
}

