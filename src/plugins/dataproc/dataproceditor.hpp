// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#ifndef DATAPROCEDITOR_H
#define DATAPROCEDITOR_H

#include <coreplugin/editormanager/ieditor.h>
#include <memory>

class QEveng;

namespace Core { class IEditorFactory; }

namespace dataproc {

    class Dataprocessor;

    class DataprocEditor : public Core::IEditor {
        Q_OBJECT
    public:
        ~DataprocEditor();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        DataprocEditor( Core::IEditorFactory * ); // for Qt5/QtCreator4
#else
        DataprocEditor();  // for Qt6/QtCreator9
#endif

#if QTC_VERSION < 0x090000 //QT_VERSION_CHECK(6, 0, 0)
        void setDataprocessor( Dataprocessor * );

        bool portfolio_create( const QString &token );
        // implement Core::IEditor

        bool open( QString*, const QString&, const QString& ) override;
        Core::IDocument * document() override;

        QByteArray saveState() const override;
        bool restoreState( const QByteArray &state ) override;
        // bool isTemporary() const override;
        QWidget *toolBar() override;
        // const char * uniqueModeName() const override;

        Core::Context context() const override;

    protected slots:
        void handleTitleChanged( const QString& title );

    private:
        std::shared_ptr< Dataprocessor > processor_; // IDocument
        QWidget * widget_;  // dummy widget (fake display for EditorManager)
        Core::IEditorFactory * factory_;
        Core::Context context_;
        QString displayName_;
#endif
        bool eventFilter( QObject * object, QEvent * event ) override;
#if QTC_VERSION >= 0x09'00'00
        ////////////// Qt6/QtCreator9 ///////////////////
    public:
        Core::IDocument * document() const override;
        QWidget *toolBar() override;
        Core::IEditor *duplicate() override;
    private:
        class impl;
        std::unique_ptr< impl > impl_;
#endif
    };

}

#endif // DATAPROCEDITOR_H
