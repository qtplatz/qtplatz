//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "sequence.h"

using namespace sequence::internal;

Sequence::~Sequence()
{
}

Sequence::Sequence(QObject *parent) :
    Core::IFile(parent)
    , modified_(true)
{
}

void
Sequence::setModified( bool val )
{
  if ( modified_ == val )
    return;
  modified_ = val;
  emit changed();
}

bool
Sequence::isModified() const
{
  return modified_;
}

QString
Sequence::mimeType() const
{
  return mimeType_;
}

bool
Sequence::save( const QString& filename )
{
  Q_UNUSED(filename);
  return false;
}

QString
Sequence::fileName() const
{
  return filename_;
}

QString
Sequence::defaultPath() const
{
  return QString();
}

QString
Sequence::suggestedFileName() const
{
  return QString();
}

bool
Sequence::isReadOnly() const
{
  return false;
}

bool
Sequence::isSaveAsAllowed() const
{
  return true;
}

void
Sequence::modified( ReloadBehavior* behavior)
{
  Q_UNUSED(behavior);
}
