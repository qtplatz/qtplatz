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

#include "dataprocessorfactory.hpp"
#include "sessionmanager.hpp"
#include "dataprocessor.hpp"
#include "dataproceditor.hpp"
#include "constants.hpp"

#include "msprocessingwnd.hpp"
#include <QTabWidget>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/idocumentfactory.h>
#include <coreplugin/icore.h>
#include <coreplugin/documentmanager.h>
#include <QStringList>
#include <adcontrols/datafile.hpp>
#include <qtwrapper/qstring.hpp>

using namespace dataproc;

DataprocessorFactory::~DataprocessorFactory()
{
}

DataprocessorFactory::DataprocessorFactory( QObject * owner, 
										    const QStringList& ) : Core::IEditorFactory( owner )
{
    setId( Constants::C_DATAPROCESSOR );

    setDisplayName( tr( "OpenWidth::Dataprocessor", "Data processor" ) );
    addMimeType( "application/adfs" );
    addMimeType( "application/csv" );
    addMimeType( "application/txt" );
    addMimeType( "application/octet-stream" );
}

// implementation for IEditorFactory
Core::IEditor *
DataprocessorFactory::createEditor()
{
    auto doc = std::make_shared< Dataprocessor >();
    doc->setId( Constants::C_DATAPROCESSOR );
    auto editor = new DataprocEditor( this );
    editor->setDataprocessor( doc.get() );
    return editor;
}

