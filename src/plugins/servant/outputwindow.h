// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <coreplugin/ioutputpane.h>

class QStackedWidget;
class QPlainTextEdit;

namespace servant {

    class OutputWindow : public Core::IOutputPane {

        Q_OBJECT

    public:
        OutputWindow(void);
        ~OutputWindow(void);

        void appendLog( const std::wstring& );
        void appendLog( const QString& );

    // IOutputPane
        QWidget * outputWidget( QWidget * );
        QList<QWidget *> toolBarWidgets() const;
        QString name() const { return tr("Servant Log"); }
        int priorityInStatusBar() const;
        void visibilityChanged(bool visible);
        //bool isEmpty() const;
        //int numberOfResults() const;
        bool hasFocus();
        bool canFocus();
        void setFocus();

        bool canNext();
        bool canPrevious();
        void goToNext();
        void goToPrev();
        bool canNavigate();

        public slots:
            void clearContents();

    private:
        QStackedWidget * widget_;
        QPlainTextEdit * textWidget_;
    };

}
