// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <coreplugin/ifile.h>

namespace sequence {
  namespace internal {

    class Sequence : public Core::IFile {
      Q_OBJECT
    public:
      ~Sequence();
      explicit Sequence(QObject *parent = 0);
      
            void setModified( bool val = true );

      // implement Core::IFile
      virtual bool save(const QString &fileName);
      virtual QString fileName() const;
      
      virtual QString defaultPath() const;
      virtual QString suggestedFileName() const;
      virtual QString mimeType() const;
      
      virtual bool isModified() const;
      virtual bool isReadOnly() const;
      virtual bool isSaveAsAllowed() const;
      
      virtual void modified(ReloadBehavior *behavior);
      virtual void checkPermissions() {}

    signals:

    public slots:
      // void modified() { setModified( true ); }

    private:
      const QString mimeType_;
      const QString filename_;
      bool modified_;

    };

  }
}

#endif // SEQUENCE_H
