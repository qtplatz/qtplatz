//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Copyright (C) MS-Cheminformatics LLC Project
//////////////////////////////////////////////

#include "tuneplugin.h"

using namespace Tune;
using namespace Tune::internal;

TunePlugin::~TunePlugin()
{
}

TunePlugin::TunePlugin()
{
}

bool
TunePlugin::initialize(const QStringList& arguments, QString * error_message)
{
  return false;
}
