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

#include "peptidemode.hpp"
#include "peptideconstants.hpp"
#include <coreplugin/coreconstants.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/editormanager/ieditor.h>
#include <coreplugin/modemanager.h>
#include <utils/id.h>

using namespace peptide;

PeptideMode::~PeptideMode()
{
    Core::EditorManager::instance()->setParent(0);
}

PeptideMode::PeptideMode(QObject *parent) :  Core::IMode(parent)
{
    setDisplayName( tr( "Peptide" ) );

    setId( peptide::Constants::C_PEPTIDE_MODE );
    setContext( Core::Context( "Peptide.MainView" ) );

    setIcon(QIcon(":/peptide/images/fingerprint.png"));
    setPriority( 40 );
}


void
PeptideMode::grabEditorManager(Core::IMode *mode)
{
    if (mode != this)
        return;
    // if ( Core::EditorManager::instance()->currentEditor() )
    //     Core::EditorManager::instance()->currentEditor()->widget()->setFocus();
}
