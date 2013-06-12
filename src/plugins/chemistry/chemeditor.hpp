/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#ifndef CHEMEDITOR_HPP
#define CHEMEDITOR_HPP

#include <coreplugin/editormanager/ieditor.h>
#include <memory>

namespace Core { class IEditorFactory; }

namespace chemistry {

	class ChemFile;
	class SDFileView;

	class ChemEditor : public Core::IEditor {
		Q_OBJECT
	public:
        ~ChemEditor();
		explicit ChemEditor( SDFileView *, Core::IEditorFactory * );
  
            // implement Core::IEditor
        virtual bool createNew( const QString &contents );
        virtual bool open( const QString &fileName );
        virtual Core::IFile *file();
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
        QList<int> context() const;

    protected slots:
        void slotTitleChanged( const QString& title ) { setDisplayName( title ); }

    private:
        SDFileView * sdfileView_;
        Core::IEditorFactory * factory_;
		std::shared_ptr< ChemFile > file_;
        QList<int> context_;
        QString displayName_;
   };

}

#endif // CHEMEDITOR_HPP
