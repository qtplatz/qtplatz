//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "analysisplugin.h"

using namespace Analysis;
using namespace Analysis::internal;

AnalysisPlugin::~AnalysisPlugin()
{
}

AnalysisPlugin::AnalysisPlugin()
{
}

bool
AnalysisPlugin::initialize(const QStringList& arguments, QString* error_message)
{
  return false;
}
