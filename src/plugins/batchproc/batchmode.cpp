/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "batchmode.hpp"
#include "batchprocconstants.hpp"
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/id.h>
#include <coreplugin/modemanager.h>
#include <coreplugin/editormanager/ieditor.h>

using namespace batchproc;

BatchMode::~BatchMode()
{
    Core::EditorManager::instance()->setParent(0);
}

BatchMode::BatchMode(QObject *parent) :  Core::IMode(parent)
{
    setDisplayName( tr( "Batch" ) );
    // setUniqueModeName( batchproc::Constants::C_BATCHPROC_MODE );
    setIcon(QIcon(":/batchproc/images/file_batch.png"));
    setPriority( 30 );
    Core::Context contexts( (Core::Id( Constants::C_BATCHPROC_MODE )) );
    setContext( contexts );

    // Core::ModeManager *modeManager = Core::ModeManager::instance();
    // connect(modeManager, SIGNAL(currentModeChanged(Core::IMode*)), this, SLOT(grabEditorManager(Core::IMode*)));
}


void
BatchMode::grabEditorManager(Core::IMode *mode)
{
    if (mode != this)
        return;
}

