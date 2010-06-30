//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "dataprocessorfactory.h"
#include "dataprocessor.h"
#include "constants.h"
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/editormanager/ieditor.h>
#include <coreplugin/ifilefactory.h>
#include <QStringList>

using namespace dataproc::internal;

DataprocessorFactory::~DataprocessorFactory()
{
}

DataprocessorFactory::DataprocessorFactory(QObject *parent) :
  Core::IEditorFactory(parent)
  , kind_( "Dataprocessor" )
{
  mimeTypes_ << Constants::C_DATAPROCESSOR_MIMETYPE;
}

// implementation for IEditorFactory
Core::IEditor *
DataprocessorFactory::createEditor( QWidget * parent )
{
  return new Dataprocessor();
  /*
    DataAnalysisWindow * editorWidget = new DataAnalysisWindow( parent );
    DataEditor * editor = new DataEditor( editorWidget );
    return editor;
  */
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
DataprocessorFactory::open(const QString& filename )
{
  Core::EditorManager * em = Core::EditorManager::instance();
  Core::IEditor * iface = em->openEditor( filename, kind_ );
  return iface ? iface->file() : 0;
}
