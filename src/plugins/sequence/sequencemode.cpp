//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "sequencemode.h"
#include <coreplugin/editormanager/editormanager.h>

using namespace sequence::internal;

SequenceMode::~SequenceMode()
{
  Core::EditorManager::instance()->setParent(0);
}

SequenceMode::SequenceMode(QObject *parent) :
    Core::BaseMode(parent)
{
  setName(tr("Sequence"));
  setUniqueModeName( "Sequence.Mode" );
  setIcon(QIcon(":/fancyactionbar/images/mode_Edit.png"));
  setPriority( 96 );
}
