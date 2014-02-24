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

#include "dataproceditor.hpp"
#include "dataprocessorfactory.hpp"
#include "constants.hpp"
#include "msprocessingwnd.hpp"
#include "ifileimpl.hpp"
#include "dataprocessor.hpp"
#include "sessionmanager.hpp"
#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/filemanager.h>
#include <coreplugin/icore.h>
#include <coreplugin/coreconstants.h>
#include <qtwrapper/waitcursor.hpp>

using namespace dataproc;

DataprocEditor::DataprocEditor( Core::IEditorFactory * factory ) : Core::IEditor( 0 )
                                                                   , widget_( new QWidget ) // dummy for Core::EditorManager
                                                                   , factory_(factory)
                                                                   , file_(0)
{
    Core::UniqueIDManager * uidm = Core::UniqueIDManager::instance();
    context_ << uidm->uniqueIdentifier( Constants::C_DATAPROCESSOR )
             << uidm->uniqueIdentifier( Core::Constants::C_EDITORMANAGER );
}

DataprocEditor::~DataprocEditor()
{
    SessionManager::instance()->removeEditor( this );
	delete widget_;
}

// implement Core::IEditor
bool
DataprocEditor::createNew( const QString &contents )
{
    Q_UNUSED( contents );
    return true;
}

bool
DataprocEditor::portfolio_create( const QString& token )
{
    std::shared_ptr<Dataprocessor> processor( new Dataprocessor );
    if ( processor->create( token ) ) {
        SessionManager::instance()->addDataprocessor( processor, this );
        file_ = processor->ifile();
        return file_;
    }
    return false;
}

bool
DataprocEditor::open( const QString &filename )
{
	qtwrapper::waitCursor wait;

    std::shared_ptr<Dataprocessor> processor( new Dataprocessor );

    if ( processor->open( filename ) ) {
        SessionManager::instance()->addDataprocessor( processor, this );

        Core::FileManager * filemgr = Core::ICore::instance()->fileManager();
        if ( filemgr->addFile( processor->ifile() ) )
            filemgr->addToRecentFiles( filename );

        file_ = processor->ifile();

        return file_; // processor->ifile();
    }
    return false;
}

Core::IFile *
DataprocEditor::file()
{
    return file_;
}

const char *
DataprocEditor::kind() const
{
    return Constants::C_DATAPROCESSOR;
}

QString
DataprocEditor::displayName() const
{
    if ( file_ )
        return file_->fileName();
    return "DataprocEditor::displayName()";
}

void
DataprocEditor::setDisplayName(const QString & /* title */)
{
}

bool
DataprocEditor::duplicateSupported() const
{
    return false;
}

Core::IEditor *
DataprocEditor::duplicate(QWidget * /* parent */)
{
    return 0;
}

QByteArray
DataprocEditor::saveState() const
{
    return QByteArray();
}

bool
DataprocEditor::restoreState(const QByteArray & /* state */ )
{
    return true;
}

//virtual int currentLine() const { return 0; }
//virtual int currentColumn() const { return 0; }

bool
DataprocEditor::isTemporary() const
{
    return false;
}

QWidget *
DataprocEditor::toolBar()
{
    return 0;
}

const char * 
DataprocEditor::uniqueModeName() const
{
    return dataproc::Constants::C_DATAPROC_MODE;
}

// Core::IContext
QWidget *
DataprocEditor::widget()
{
    return widget_;
}

QList<int>
DataprocEditor::context() const
{
    return context_;
}


