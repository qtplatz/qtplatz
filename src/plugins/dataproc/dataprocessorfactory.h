// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef DATAPROCESSORFACTORY_H
#define DATAPROCESSORFACTORY_H

#include <coreplugin/editormanager/ieditorfactory.h>

namespace Core {
  class IEditor;
}

namespace dataproc {
  namespace internal {

    class DataprocessorFactory : public Core::IEditorFactory {
      Q_OBJECT
    public:
      ~DataprocessorFactory();
      explicit DataprocessorFactory(QObject *parent = 0);

      // implement IEditorFactory
      virtual Core::IEditor *createEditor(QWidget *parent);

    signals:

    public slots:

    private:

    };

  }
}

#endif // DATAPROCESSORFACTORY_H
