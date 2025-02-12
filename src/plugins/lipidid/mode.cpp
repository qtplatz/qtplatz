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

#include "mode.hpp"
#include "constants.hpp"
#include <coreplugin/coreconstants.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/editormanager/ieditor.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/modemanager.h>

using namespace lipidid;

Mode::Mode(QObject *parent) : Core::IMode(parent)
{
    setDisplayName( tr( "LipidID" ) );

    // <a href="https://www.flaticon.com/free-icons/cell" title="cell icons">Cell icons created by Freepik - Flaticon</a>
    setIcon(QIcon(":/lipidid/image/cell.png"));

    setPriority( 60 );

    setId( Constants::C_LIPIDID );
    // setContextHelpId( QLatin1String( "QtPlatz Manual " ) );
    setContext( Core::Context( Constants::C_LIPIDID, Core::Constants::MODE_EDIT ) );

    connect(Core::ModeManager::instance(), &Core::ModeManager::currentModeChanged, this, &Mode::grabEditorManager);
}

void
Mode::grabEditorManager( Utils::Id mode)
{
    if (mode != id())
        return;

    if ( auto cmd = Core::ActionManager::instance()->command( Core::Constants::OPEN ) )
        cmd->action()->setText( tr( "Open data files..." ) );
}
