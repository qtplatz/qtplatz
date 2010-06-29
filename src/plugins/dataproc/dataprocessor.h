// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef DATAPROCESSOR_H
#define DATAPROCESSOR_H

#include <coreplugin/editormanager/ieditor.h>
#include <boost/smart_ptr.hpp>

namespace dataproc {
  namespace internal {

    class Dataset;

    class Dataprocessor : public Core::IEditor {
      Q_OBJECT
    public:
      ~Dataprocessor();
      explicit Dataprocessor(QObject *parent = 0);

      // implement Core::IEditor
      virtual bool createNew(const QString &contents = QString());
      virtual bool open(const QString &fileName = QString());
      virtual Core::IFile *file();
      virtual const char *kind() const;
      virtual QString displayName() const;
      virtual void setDisplayName(const QString &title);
      virtual bool duplicateSupported() const;
      virtual IEditor *duplicate(QWidget *parent);
      virtual QByteArray saveState() const;
      virtual bool restoreState(const QByteArray &state);
      virtual int currentLine() const { return 0; }
      virtual int currentColumn() const { return 0; }
      virtual bool isTemporary() const;
      virtual QWidget *toolBar();
      // end Core::IEditor

      // implement IContext
      virtual QList<int> context() const { return context_; }
      virtual QWidget * widget() { return widget_; }
      // 
    signals:

    public slots:
      void slotTitleChanged( const QString& title );

    private:
      QList<int> context_;
      boost::shared_ptr< Dataset > dataset_;
      QString displayName_;
      QWidget * widget_;
    };

  } // internal
} // dataproc

#endif // DATAPROCESSOR_H
