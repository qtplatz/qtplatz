// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef DATAPROCESSORFACTORY_H
#define DATAPROCESSORFACTORY_H

#include <coreplugin/editormanager/ieditorfactory.h>
#include <QStringList>

namespace Core {
  class IEditor;
}

namespace dataproc {

    namespace internal {

        class DataprocPlugin;

        // class DataprocessorFactory : public Core::IFileFactory {  // Core::IEditorFactory
        class DataprocessorFactory : public Core::IEditorFactory {
            Q_OBJECT
        public:
            ~DataprocessorFactory();
            explicit DataprocessorFactory( QObject * owner );

            void setEditor( QWidget * );

            // implement IEditorFactory
            virtual Core::IEditor *createEditor(QWidget *parent);

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
            QWidget * editorWidget_;
        };

    }
}

#endif // DATAPROCESSORFACTORY_H
