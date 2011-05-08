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

#include "dataprocmode.hpp"
#include "constants.hpp"
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/modemanager.h>
#include <coreplugin/editormanager/ieditor.h>

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
    setUniqueModeName( Core::Constants::MODE_EDIT );  // pretending to "Editor" for Core
    setIcon(QIcon(":/fancyactionbar/images/mode_Debug.png"));
    setPriority( 97 );

    QList<int> contexts = QList<int>() <<
        Core::UniqueIDManager::instance()->uniqueIdentifier( Constants::C_DATAPROCESSOR ) <<
        Core::UniqueIDManager::instance()->uniqueIdentifier(Core::Constants::C_EDIT_MODE) <<
        Core::UniqueIDManager::instance()->uniqueIdentifier(Core::Constants::C_EDITORMANAGER) <<
        Core::UniqueIDManager::instance()->uniqueIdentifier(Core::Constants::C_NAVIGATION_PANE);
    setContext( contexts );

    Core::ModeManager *modeManager = Core::ModeManager::instance();
    connect(modeManager, SIGNAL(currentModeChanged(Core::IMode*)), this, SLOT(grabEditorManager(Core::IMode*)));
}

void
DataprocMode::grabEditorManager(Core::IMode *mode)
{
    if (mode != this)
        return;

    Core::EditorManager * em = Core::EditorManager::instance();
    
    if ( em->currentEditor() )
        em->currentEditor()->widget()->setFocus();
}
