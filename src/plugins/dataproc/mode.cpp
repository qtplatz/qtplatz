/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include "dataprocconstants.hpp"
#include <adportable/debug.hpp>
#include <coreplugin/coreconstants.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/editormanager/ieditor.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/modemanager.h>

namespace dataproc {

    Mode::Mode(QObject *parent) : Core::IMode(parent)
    {
        setDisplayName( tr( "Processing" ) );
        setIcon(QIcon(":/dataproc/image/ViewResults.png"));

        setPriority( 80 );

        setId( Constants::C_DATAPROCESSOR );

        connect(Core::ModeManager::instance(), &Core::ModeManager::currentModeChanged, this, &Mode::grabEditorManager);
    }

    void
    Mode::grabEditorManager( Utils::Id mode )
    {
        if ( mode == id() ) {
            // qDebug() << "\t-- currentModeChanged to " << mode << " == this mode(" << id() << ")";
        } else {
            // qDebug() << "\t-- currentModeChanged to " << mode << " != this mode(" << id() << ")";
        }
        if ( auto cmd = Core::ActionManager::instance()->command( Core::Constants::OPEN ) )
            cmd->action()->setText( tr( "Open data files..." ) );

        if ( Core::EditorManager::currentEditor() )
            Core::EditorManager::currentEditor()->widget()->setFocus();

    }
}
