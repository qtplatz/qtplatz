// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#ifndef SEQUENCEEDITORFACTORY_H
#define SEQUENCEEDITORFACTORY_H

#include <coreplugin/editormanager/ieditorfactory.h>
#include <QStringList>

namespace sequence {
  namespace internal {

    class SequenceEditorFactory : public Core::IEditorFactory {
      Q_OBJECT
    public:
    ~SequenceEditorFactory();
    explicit SequenceEditorFactory(QObject *parent = 0);

      // implement IEditorFactory
      virtual Core::IEditor *createEditor(QWidget *parent);
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

#endif // SEQUENCEEDITORFACTORY_H
