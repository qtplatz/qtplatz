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

namespace Core { class IEditorFactory; }

namespace dataproc {

    class datafileimpl;

    class DataprocEditor : public Core::IEditor {
        Q_OBJECT
    public:
        ~DataprocEditor();
        DataprocEditor( Core::IEditorFactory * );

        bool portfolio_create( const QString &token );
        // implement Core::IEditor
        virtual bool createNew( const QString &contents );
        virtual bool open( const QString &fileName );

        bool open( QString*, const QString&, const QString& ) override;
        Core::IDocument * document() override;

        virtual const char *kind() const;
        virtual QString displayName() const;
        virtual void setDisplayName(const QString &title);

        virtual bool duplicateSupported() const;
        virtual IEditor *duplicate(QWidget *parent);

        virtual QByteArray saveState() const;
        virtual bool restoreState(const QByteArray &state);
        virtual bool isTemporary() const;
        virtual QWidget *toolBar();
        virtual const char * uniqueModeName() const;

        // Core::IContext
        QWidget * widget();
        Core::Context context() const override;

    protected slots:
        void slotTitleChanged( const QString& title ) { setDisplayName( title ); }

    private:
		QWidget * widget_;  // dummy
        Core::IEditorFactory * factory_;
        Core::IDocument * file_;
        Core::Context context_;
        QString displayName_;
    };

}

#endif // DATAPROCEDITOR_H
