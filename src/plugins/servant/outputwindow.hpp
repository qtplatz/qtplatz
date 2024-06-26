// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/

#pragma once

#include <coreplugin/ioutputpane.h>
#include <adextension/ilogger.hpp>

class QStackedWidget;
class QPlainTextEdit;
class QTextEdit;

namespace servant {

    class OutputWindow : public Core::IOutputPane {

        Q_OBJECT

    public:
        OutputWindow(void);
        ~OutputWindow(void);

        void appendLog( const std::wstring& );
        void appendLog( const QString& );

        // IOutputPane
        QWidget * outputWidget( QWidget * ) override;
        QList<QWidget *> toolBarWidgets() const override;
#if QTC_VERSION <= 0x08'00'02
        QString displayName() const override { return tr( "Servant Log" ); }
        int priorityInStatusBar() const override;
#endif
        void visibilityChanged(bool visible) override;
        //bool isEmpty() const;
        //int numberOfResults() const;
        bool hasFocus() const override { return false; }
        bool canFocus() const override { return false; }

        void setFocus() override;

        bool canNext() const override { return false; }
        bool canPrevious() const override { return false; }
        void goToNext() override;
        void goToPrev() override;
        bool canNavigate() const override { return false; }

    public slots:
        void clearContents() override;
        void handleLogging( const QString&, bool );

    private:
        QStackedWidget * widget_;
        QPlainTextEdit * textWidget_;
        // QTextEdit * textWidget_;
    };

}
