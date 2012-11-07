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
#include "sdfileview.hpp"
#include "constants.hpp"
#include "chemistrymainwindow.hpp"
#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/filemanager.h>
#include <coreplugin/icore.h>
#include <coreplugin/coreconstants.h>
#include <QWidget>

//#include <openbabel/obconversion.h>
//#include <openbabel/mol.h>

using namespace chemistry;

ChemEditor::ChemEditor( SDFileView * view
                        , Core::IEditorFactory * factory ) : Core::IEditor( view )
                                                           , sdfileView_( view )
                                                           , factory_( factory )
{
    Core::UniqueIDManager * uidm = Core::UniqueIDManager::instance();
    context_ << uidm->uniqueIdentifier( Constants::C_CHEM_MODE );
	connect( sdfileView_, SIGNAL( rawClicked( int, const SDFileModel* ) ), ChemistryMainWindow::instance(), SLOT( handleViewDetails( int, const SDFileModel* ) ) );
	connect( sdfileView_, SIGNAL( rawClicked( int, const SDFileModel* ) ), ChemistryMainWindow::instance(), SLOT( handleViewFragments( int, const SDFileModel* ) ) );
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
		sdfileView_->file( file_ );
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
	return sdfileView_;
}

QList<int>
ChemEditor::context() const
{
    return context_;
}
