/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/modemanager.h>
#include <coreplugin/editormanager/ieditor.h>

using namespace sequence;

Mode::Mode(QObject *parent) :  Core::BaseMode(parent)
{
    setName( tr("Sequence" ) );
    setUniqueModeName( sequence::Constants::C_SEQUENCE_MODE );
    setIcon(QIcon(":/sequence/images/DefineControls.png"));
    setPriority( 90 );

    QList<int> contexts = QList<int>() 
        << Core::UniqueIDManager::instance()->uniqueIdentifier( sequence::Constants::C_SEQUENCE_MODE )
        << Core::UniqueIDManager::instance()->uniqueIdentifier( Core::Constants::C_EDIT_MODE )
        << Core::UniqueIDManager::instance()->uniqueIdentifier( Core::Constants::C_EDITORMANAGER )
        //<< Core::UniqueIDManager::instance()->uniqueIdentifier( Core::Constants::C_NAVIGATION_PANE )
        ;
        
    setContext( contexts );

    Core::ModeManager *modeManager = Core::ModeManager::instance();
    connect(modeManager, SIGNAL(currentModeChanged(Core::IMode*)), this, SLOT(grabEditorManager(Core::IMode*)));
}

Mode::~Mode()
{
    Core::EditorManager::instance()->setParent( 0 );
}

void
Mode::grabEditorManager( Core::IMode * mode )
{
    if ( mode != this )
        return;
    Core::EditorManager * em = Core::EditorManager::instance();
    if ( em && em->currentEditor() )
        em->currentEditor()->widget()->setFocus();
}
