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
#include <QObject>
#include <QEvent>

class QEveng;

namespace Core { class IEditorFactory; }

namespace dataproc {

    class Dataprocessor;

    class DataprocEditor : public Core::IEditor {
        Q_OBJECT
    public:
        ~DataprocEditor();
        DataprocEditor();  // for QtCreator8

        bool eventFilter( QObject * object, QEvent * event ) override;

        ////////////// Qt6/QtCreator9 ///////////////////
    public:
        Core::IDocument * document() const override;
        QWidget *toolBar() override;
        Core::IEditor *duplicate() override;
    private:
        class impl;
        std::unique_ptr< impl > impl_;

    };

}

#endif // DATAPROCEDITOR_H
