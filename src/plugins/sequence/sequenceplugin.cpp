//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "sequenceplugin.h"

using namespace Sequence;
using namespace Sequence::internal;

SequencePlugin::~SequencePlugin()
{
}

SequencePlugin::SequencePlugin()
{
}

bool
SequencePlugin::initialize(const QStringList& arguments, QString* error_message)
{
  return false;
}

