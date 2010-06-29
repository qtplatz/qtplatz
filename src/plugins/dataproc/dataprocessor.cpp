//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "dataprocessor.h"
#include "dataset.h"
#include <coreplugin/uniqueidmanager.h>

#define C_DATAPROCESSOR "Dataprocessor"

using namespace dataproc::internal;

Dataprocessor::~Dataprocessor()
{
}

Dataprocessor::Dataprocessor(QObject *parent) : Core::IEditor(parent)
					      , widget_(0)
{
  Core::UniqueIDManager* uidm = Core::UniqueIDManager::instance();
  if ( uidm ) {
    context_ << uidm->uniqueIdentifier( C_DATAPROCESSOR );
  }
}

// Core::IEditor
bool
Dataprocessor::createNew(const QString &contents )
{
    Q_UNUSED( contents );
    return false;
}

bool
Dataprocessor::open(const QString &fileName )
{
    // todo
    return false;
}

Core::IFile *
Dataprocessor::file()
{
  return static_cast<Core::IFile *>( dataset_.get() );
}

const char *
Dataprocessor::kind() const
{
  return C_DATAPROCESSOR;
}

QString
Dataprocessor::displayName() const
{
  return displayName_;
}

void
Dataprocessor::setDisplayName(const QString &title)
{
  displayName_ = title;
}

bool
Dataprocessor::duplicateSupported() const
{
  return false;
}

Core::IEditor *
Dataprocessor::duplicate(QWidget *parent)
{
  Q_UNUSED( parent );
  return 0;
}

QByteArray
Dataprocessor::saveState() const
{
  return QByteArray();
}

bool
Dataprocessor::restoreState(const QByteArray &state)
{
  Q_UNUSED( state );
  return false;
}

bool
Dataprocessor::isTemporary() const
{
  return false;
}

QWidget *
Dataprocessor::toolBar()
{
  return 0;
}
// end Core::IEditor

void
Dataprocessor::slotTitleChanged( const QString& title )
{
    setDisplayName( title );
}
