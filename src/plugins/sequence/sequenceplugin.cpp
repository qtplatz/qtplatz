//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "sequenceplugin.h"
#include "sequencemode.h"
#include "sequenceeditorfactory.h"

#include <QtCore/qplugin.h>
#include <coreplugin/icore.h>
#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/mimedatabase.h>

#include <QStringList>

using namespace sequence;
using namespace sequence::internal;

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

    Core::ICore * core = Core::ICore::instance();
    
    QList<int> context;
    if ( core ) {
      Core::UniqueIDManager * uidm = core->uniqueIDManager();
      if ( uidm ) {
	context.append( uidm->uniqueIdentifier( QLatin1String("Dataproc.MainView") ) );
	context.append( uidm->uniqueIdentifier( Core::Constants::C_NAVIGATION_PANE ) );
      }
    } else
      return false;
    
    Core::MimeDatabase* mdb = core->mimeDatabase();
    if ( mdb ) {
      if ( !mdb->addMimeTypes(":/sequence/sequence-mimetype.xml", error_message) )
	return false;
      addAutoReleasedObject( new SequenceEditorFactory(this) );
    }
    
    return true;
}

void
SequencePlugin::extensionsInitialized()
{
}

Q_EXPORT_PLUGIN( SequencePlugin )
