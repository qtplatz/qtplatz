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

#include "dataprocmode.h"
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/coreconstants.h>

using namespace dataproc;
using namespace dataproc::internal;

DataprocMode::~DataprocMode()
{
    Core::EditorManager::instance()->setParent(0);
}

DataprocMode::DataprocMode(QObject *parent) :
    Core::BaseMode(parent)
{
    setName(tr("Data processing"));
    setUniqueModeName( "Edit" );  // pretending to "Editor" for Core
    setIcon(QIcon(":/fancyactionbar/images/mode_Debug.png"));
    setPriority( 97 );
}
