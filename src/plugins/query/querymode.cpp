/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#include "querymode.hpp"
#include "queryconstants.hpp"
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/coreconstants.h>
#if QTC_VERSION <= 0x03'02'81
#include <coreplugin/id.h>
#else
#include <utils/id.h>
#endif
#include <coreplugin/modemanager.h>
#include <coreplugin/editormanager/ieditor.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/modemanager.h>

using namespace query;

QueryMode::QueryMode(QObject *parent) : Core::IMode(parent)
{
    setId( Constants::C_QUERY_MODE );
    setContext( Core::Context( Constants::C_QUERY_MODE, Core::Constants::MODE_EDIT ) );
    setDisplayName( tr( "Query" ) );
    setIcon(QIcon(":/query/images/Sqlite-square-icon.svg"));
    setPriority( 30 );
    connect(Core::ModeManager::instance(), &Core::ModeManager::currentModeChanged, this, &QueryMode::grabEditorManager);
}

QueryMode::~QueryMode()
{
    //will delete mainWindow at BaseMode baseclass DTOR
}

void
QueryMode::grabEditorManager(Utils::Id mode)
{
    if ( mode != id() )
        return;

    if ( auto cmd = Core::ActionManager::instance()->command( Core::Constants::OPEN ) )
        cmd->action()->setText( tr( "Open database..." ) );

    if ( Core::EditorManager::currentEditor() )
        Core::EditorManager::currentEditor()->widget()->setFocus();

    // if ( Core::EditorManager::instance()->currentEditor() )
    //     Core::EditorManager::instance()->currentEditor()->widget()->setFocus();
}
