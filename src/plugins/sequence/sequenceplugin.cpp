//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "sequenceplugin.h"
#include <QtCore/qplugin.h>
#include <QStringList>

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
    Q_UNUSED( arguments );
    Q_UNUSED( error_message );
    return true;
}

void
SequencePlugin::extensionsInitialized()
{
}

Q_EXPORT_PLUGIN( SequencePlugin )
