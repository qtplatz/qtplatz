//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "dataprocessorfactory.h"
#include "dataprocessor.h"

using namespace dataproc::internal;

DataprocessorFactory::~DataprocessorFactory()
{
}

DataprocessorFactory::DataprocessorFactory(QObject *parent) :
    Core::IEditorFactory(parent)
{
}

Core::IEditor *
DataprocessorFactory::createEditor( QWidget * parent )
{
  return new Dataprocessor();
}
