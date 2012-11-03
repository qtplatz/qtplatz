/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
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

#include "chemeditorfactory.hpp"
#include "chemeditor.hpp"
#include "constants.hpp"
#include "sdfileview.hpp"

#include <QTabWidget>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/ifilefactory.h>
#include <coreplugin/icore.h>
#include <coreplugin/filemanager.h>
#include <QStringList>
#include <adcontrols/datafile.hpp>
#include <qtwrapper/qstring.hpp>

using namespace chemistry;

ChemEditorFactory::~ChemEditorFactory()
{
}

ChemEditorFactory::ChemEditorFactory( QObject * owner, 
                                      const QStringList& types ) : Core::IEditorFactory( owner )
																 , kind_( Constants::C_CHEM_EDITOR )
                                                                 , mimeTypes_ ( types ) 
                                                                 , tabWidget_(0) 
{
    mimeTypes_ 
        << Constants::C_SDF_MIMETYPE
        << Constants::C_MOL_MIMETYPE
		<< "application/octet-stream";
}

// implementation for IEditorFactory
Core::IEditor *
ChemEditorFactory::createEditor( QWidget * parent )
{
    (void)parent;
	SDFileView * view = new SDFileView;
	return new ChemEditor( view, this );
}

// implementation for IFileFactory

QStringList 
ChemEditorFactory::mimeTypes() const
{
    return mimeTypes_;
}

QString 
ChemEditorFactory::kind() const
{
    return kind_;
}

Core::IFile * 
ChemEditorFactory::open( const QString& filename )
{
    Core::EditorManager * em = Core::EditorManager::instance();
    Core::IEditor * iface = em->openEditor( filename, kind_ );
    return iface ? iface->file() : 0;
}
