// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

#include "dataprocessorfactory.hpp"
#include "sessionmanager.hpp"
#include "dataprocessor.hpp"
#include "dataproceditor.hpp"
#include "constants.hpp"

#include "msprocessingwnd.hpp"
#include <QTabWidget>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/ifilefactory.h>
#include <coreplugin/icore.h>
#include <coreplugin/filemanager.h>
#include <QStringList>
#include <adcontrols/datafile.hpp>
#include <qtwrapper/qstring.hpp>

using namespace dataproc::internal;

DataprocessorFactory::~DataprocessorFactory()
{
}

DataprocessorFactory::DataprocessorFactory( QObject * owner ) : Core::IEditorFactory( owner )
                                                              , kind_( "Dataprocessor" )
                                                              , editorWidget_(0) 
{
    mimeTypes_ 
        << Constants::C_DATA_MC4_MIMETYPE
        << Constants::C_DATA_TEXT_MIMETYPE
        << Constants::C_DATA_INFITOF_MIMETYPE
        << Constants::C_DATA_NATIVE_MIMETYPE
		<< "application/octet-stream";
}

void
DataprocessorFactory::setEditor( QWidget * p )
{
    editorWidget_ = p;
}

// implementation for IEditorFactory
Core::IEditor *
DataprocessorFactory::createEditor( QWidget * /* parent */)
{
    QTabWidget * pTab = new QTabWidget;
    editorWidget_ = pTab;

    return new DataprocEditor( editorWidget_, this );
    // return 0;
}

// implementation for IFileFactory

QStringList 
DataprocessorFactory::mimeTypes() const
{
    return mimeTypes_;
}

QString 
DataprocessorFactory::kind() const
{
    return kind_;
}

Core::IFile * 
DataprocessorFactory::open( const QString& filename )
{
    Core::EditorManager * em = Core::EditorManager::instance();
    Core::IEditor * iface = em->openEditor( filename, kind_ );
    return iface ? iface->file() : 0;
}

