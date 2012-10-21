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

#include "chemeditor.hpp"
#include "chemeditorfactory.hpp"
#include "chemfile.hpp"
#include "constants.hpp"
#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/filemanager.h>
#include <coreplugin/icore.h>
#include <coreplugin/coreconstants.h>
#include <QWidget>

#ifdef _MSC_VER
#pragma warning( disable: 4100 )
#endif
#include <openbabel/obconversion.h>
#include <openbabel/mol.h>
#ifdef _MSC_VER
#pragma warning( default: 4100 )
#endif

using namespace Chemistry::Internal;

ChemEditor::ChemEditor( QWidget * widget
                        , Core::IEditorFactory * factory ) : Core::IEditor( widget )
                                                           , editorWidget_( widget )
                                                           , factory_( factory )
                                                           , file_(0)
{
    Core::UniqueIDManager * uidm = Core::UniqueIDManager::instance();
    context_ << uidm->uniqueIdentifier( Constants::C_CHEM_EDITOR );
	//connect( editorWidget_, SIGNAL( titleChanged(QString) ), this, SLOT( slotTitleChanged(QString) ) );
	//connect( editorWidget_, SIGNAL( contentModified() ), this, SIGNAL( changed() ) );
}

ChemEditor::~ChemEditor()
{
}

// implement Core::IEditor
bool
ChemEditor::createNew( const QString &contents )
{
    Q_UNUSED( contents );
    //editorWidget_->setContent( QByteArray() );
    //file_->setFilename( QString() );
    return true;
}

bool
ChemEditor::open( const QString &qfilename )
{
	std::string filename( qfilename.toStdString() );

	file_.reset( new ChemFile );
    if ( file_->open( qfilename, 0 ) ) {
		Core::FileManager * filemgr = Core::ICore::instance()->fileManager();
        if ( filemgr->addFile( file_.get() ) )
			filemgr->addToRecentFiles( qfilename );
		return true;
	}
	return false;
}

Core::IFile *
ChemEditor::file()
{
    return file_.get();
}

const char *
ChemEditor::kind() const
{
    return Constants::C_CHEM_EDITOR;
}

QString
ChemEditor::displayName() const
{
    if ( file_ )
        return file_->fileName();
    return "ChemEditor::displayName()";
}

void
ChemEditor::setDisplayName(const QString & /* title */)
{
}

bool
ChemEditor::duplicateSupported() const
{
    return false;
}

Core::IEditor *
ChemEditor::duplicate(QWidget * /* parent */)
{
    return 0;
}

QByteArray
ChemEditor::saveState() const
{
    return QByteArray();
}

bool
ChemEditor::restoreState(const QByteArray & /* state */ )
{
    return true;
}

//virtual int currentLine() const { return 0; }
//virtual int currentColumn() const { return 0; }

bool
ChemEditor::isTemporary() const
{
    return false;
}

QWidget *
ChemEditor::toolBar()
{
    return 0;
}

// Core::IContext
QWidget *
ChemEditor::widget()
{
    return editorWidget_;
}

QList<int>
ChemEditor::context() const
{
    return context_;
}
