// This is a -*- C++ -*- header.
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////
#ifndef OUTPUTWINDOW_H
#define OUTPUTWINDOW_H

#include <coreplugin/ioutputpane.h>

class QPlainTextEdit;

namespace App {
  namespace internal {

    class OutputWindow : public Core::IOutputPane {
      Q_OBJECT
    public:
      explicit OutputWindow(QWidget *parent = 0);

      QWidget * outputWidget( QWidget * );
      QList< QWidget *> toolBarWidgets() const { return QList<QWidget *>(); }
      QString name() const { return tr("Log book"); }
      int priorityInStatusBar() const;
      void clearContents();
      void visibilityChanged(bool visible);
      void appendText( const QString& text );
      bool canFocus();
      bool hasFocus();
      void setFocus();
      
      bool canNext();
      bool canPrevious();
      void goToNext();
      void goToPrev();
      bool canNavigate();

    signals:


    signals:

    public slots:

    private:
      QPlainTextEdit * textEdit_;

    };

    /***
     **/
  }
}

#endif // OUTPUTWINDOW_H
