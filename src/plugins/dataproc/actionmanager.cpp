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

#include "actionmanager.hpp"
#include <coreplugin/icore.h>
#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/editormanager/ieditor.h>

#include <coreplugin/coreconstants.h>
#include <QIcon>
//#include <coreplugin/mimedatabase.h>
#include <coreplugin/coreconstants.h>
//#include <QStringList>
//#include "dataprocplugin.h"

using namespace dataproc;

ActionManager::ActionManager(QObject *parent) : QObject(parent)
                                              , saveAction_( 0 )
                                              , saveAsAction_( 0 )
                                              , closeCurrentEditorAction_( 0 )
                                              , closeAllEditorsAction_( 0 )
                                              , closeOtherEditorsAction_( 0 )
                                              , importFile_( 0 )
{
}

bool
ActionManager::initialize_actions( const QList<int>& context )
{
    (void)context;
    //context_ = context;
#if 0
    Core::ICore * core = Core::ICore::instance();
    Core::ActionManager *am = core->actionManager();

	saveAsAction_ = new QAction( QIcon( ":/dataproc/image/apply_small.png" ), tr("SaveAs" ), this );
	am->registerAction( saveAsAction_, Core::Constants::SAVEAS, context );
    connect( saveAsAction_, SIGNAL( triggered() ), this, SLOT( saveFileAs() ) );
#endif
    return true;
}

/////////////////////////////////////////////////////////////////
/// copy from editormanager.cpp
bool
ActionManager::saveFileAs() // Core::IEditor *editor )
{
    return false;
}

bool
ActionManager::importFile()
{
    return true;
}
