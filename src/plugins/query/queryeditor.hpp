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

#include <coreplugin/idocument.h>
#include <coreplugin/editormanager/ieditor.h>
#include <QStringList>

class QEvent;

namespace query {

    class QueryDocProxy : public Core::IDocument {
        Q_OBJECT
    public:
        inline Core::IDocument * document() { return this; };

        // Core::IDocument
        bool save( QString* errorString, const QString& filename = QString(), bool autoSave = false ) override;
        bool reload( QString *, Core::IDocument::ReloadFlag, Core::IDocument::ChangeType ) override;

        QString defaultPath() const override;
        QString suggestedFileName() const override;
        bool isModified() const override;
        bool isSaveAsAllowed() const override;
        bool isFileReadOnly() const override;
    };

    class QueryEditor : public Core::IEditor {
        Q_OBJECT
    public:
        ~QueryEditor();
        QueryEditor( QObject * parent = 0 );
        bool eventFilter( QObject * object, QEvent * event ) override;

        // Core::IEditor
        bool open( QString*, const QString&, const QString& ) override;
        Core::IDocument * document() override;

        QByteArray saveState() const override;
        bool restoreState( const QByteArray &state ) override;

        QWidget *toolBar() override;
        Core::Context context() const override;

    private:
        QWidget * widget_;
        QueryDocProxy * proxy_;
    };


}


