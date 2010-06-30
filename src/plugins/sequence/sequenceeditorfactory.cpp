//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#include "sequenceeditorfactory.h"
#include "sequenceeditor.h"

#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/editormanager/ieditor.h>
#include <coreplugin/ifilefactory.h>

using namespace sequence::internal;

SequenceEditorFactory::~SequenceEditorFactory()
{
}

SequenceEditorFactory::SequenceEditorFactory(QObject *parent) :
    Core::IEditorFactory(parent)
    , kind_( "Sequence" )
{
  mimeTypes_ << "Sequece";
}

// implementation for IEditorFactory
Core::IEditor *
SequenceEditorFactory::createEditor( QWidget * parent )
{
  return new SequenceEditor();
  /*
    DataAnalysisWindow * editorWidget = new DataAnalysisWindow( parent );
    DataEditor * editor = new DataEditor( editorWidget );
    return editor;
  */
}

// implementation for IFileFactory

QStringList 
SequenceEditorFactory::mimeTypes() const
{
  return mimeTypes_;
}

QString 
SequenceEditorFactory::kind() const
{
  return kind_;
}

Core::IFile * 
SequenceEditorFactory::open(const QString& filename )
{
  Core::EditorManager * em = Core::EditorManager::instance();
  Core::IEditor * iface = em->openEditor( filename, kind_ );
  return iface ? iface->file() : 0;
}

