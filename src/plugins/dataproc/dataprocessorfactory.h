// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef DATAPROCESSORFACTORY_H
#define DATAPROCESSORFACTORY_H

#include <coreplugin/editormanager/ieditorfactory.h>
#include <QStringList>

namespace Core {
  class IEditor;
}

namespace dataproc {

    namespace internal {

        class DataprocPlugin;

        class DataprocessorFactory : public Core::IFileFactory {  // Core::IEditorFactory
            Q_OBJECT
        public:
            ~DataprocessorFactory();
            explicit DataprocessorFactory( DataprocPlugin * owner );

            // implement IEditorFactory
            // virtual Core::IEditor *createEditor(QWidget *parent);
            // <---
            // implement IFileFactory
            virtual QStringList mimeTypes() const;
            virtual QString kind() const;
            virtual Core::IFile * open(const QString& filename );
            // <---
        signals:

        public slots:

        private:
            QString kind_;
            QStringList mimeTypes_;
        };

    }
}

#endif // DATAPROCESSORFACTORY_H
