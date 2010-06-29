// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "dataset.h"

using namespace dataproc::internal;

Dataset::~Dataset()
{
}

Dataset::Dataset(QObject *parent) :  Core::IFile(parent)
				  , modified_(true)
{
}

void
Dataset::setModified( bool val )
{
  if ( modified_ == val )
    return;
  modified_ = val;
  emit changed();
}

bool
Dataset::isModified() const
{
  return modified_;
}

QString
Dataset::mimeType() const
{
  return mimeType_;
}

bool
Dataset::save( const QString& filename )
{
  Q_UNUSED(filename);
  return false;
}

/*
bool
Dataset::open( const QString& filename )
{
  return false;
}
*/

/*
void
Dataset::setFilename( const QString& filename )
{
  filename_ = filename;
}
*/

QString
Dataset::fileName() const
{
  return filename_;
}

QString
Dataset::defaultPath() const
{
  return QString();
}

QString
Dataset::suggestedFileName() const
{
  return QString();
}

/*
QString
Dataset::fileFilter() const
{
  return QString( "*.data" );
}

QString
Dataset::fileExtension() const
{
  return QString( ".data" );
}
*/

bool
Dataset::isReadOnly() const
{
  return false;
}

bool
Dataset::isSaveAsAllowed() const
{
  return true;
}

void
Dataset::modified( ReloadBehavior* behavior)
{
  Q_UNUSED(behavior);
}
