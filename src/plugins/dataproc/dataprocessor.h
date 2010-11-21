// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef DATAPROCESSOR_H
#define DATAPROCESSOR_H

// #include <coreplugin/editormanager/ieditor.h>
#include <coreplugin/ifile.h>
#include <boost/smart_ptr.hpp>

namespace dataproc {

    namespace internal {

        class datafileimpl;

        class Dataprocessor : QObject { //public Core::IEditor {
            Q_OBJECT
        public:
            ~Dataprocessor();
            Dataprocessor();
            Dataprocessor( const Dataprocessor& );

            bool open( const QString& );
            Core::IFile * ifile();

        signals:

        public slots:
            // void slotTitleChanged( const QString& title );

        private:
            boost::shared_ptr< datafileimpl > datafileimpl_;
        };

    } // internal
} // dataproc

#endif // DATAPROCESSOR_H
