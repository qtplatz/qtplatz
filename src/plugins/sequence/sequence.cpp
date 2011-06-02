// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "sequence.h"

#if defined _DEBUG
#     pragma comment(lib, "adportabled.lib")
#     pragma comment(lib, "adplugind.lib")
#     pragma comment(lib, "adcontrolsd.lib")
#     pragma comment(lib, "acewrapperd.lib")
#     pragma comment(lib, "qtwrapperd.lib")
#else
#     pragma comment(lib, "adportable.lib")
#     pragma comment(lib, "adplugin.lib")
#     pragma comment(lib, "adcontrols.lib")
#     pragma comment(lib, "acewrapper.lib")
#     pragma comment(lib, "qtwrapper.lib")
#endif

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
