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

#include "mode.hpp"
#include "constants.hpp"
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/coreconstants.h>
#if QTC_VERSION <= 0x03'02'81
# include <coreplugin/id.h>
#endif
#include <coreplugin/modemanager.h>
#include <coreplugin/editormanager/ieditor.h>

using namespace acquire;

Mode::~Mode()
{
    Core::EditorManager::instance()->setParent(0);
}

Mode::Mode(QObject *parent) :  Core::IMode(parent)
{
    setDisplayName( tr( "ACQUIRE" ) );
    setIcon(QIcon(":/acquire/images/Ecg-595b40b75ba036ed117d58e2.svg"));
    setPriority( 90 );
}


void
Mode::grabEditorManager(Core::IMode *mode)
{
    if (mode != this)
        return;

    Core::EditorManager * em = Core::EditorManager::instance();

    if ( em->currentEditor() )
        em->currentEditor()->widget()->setFocus();
}
