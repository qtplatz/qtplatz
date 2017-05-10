/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "mspeaktable.hpp"
#include "sessionmanager.hpp"
#include "dataprocessor.hpp"
#include <QMenu>

using namespace dataproc;

MSPeakTable::MSPeakTable(QWidget *parent) : adwidgets::MSPeakTable( parent )
{
}

void
MSPeakTable::addContextMenu( QMenu& menu
                             , const QPoint& pos
                             , std::shared_ptr< const adcontrols::MassSpectrum > ms ) const
{
    adwidgets::MSPeakTable::addActionsToContextMenu( menu, pos );

    menu.addSeparator();

    if ( ms ) {
        if ( auto dp = SessionManager::instance()->getActiveDataprocessor() ) {
            dp->addContextMenu( adprocessor::ContextMenuOnMSPeakTable, menu, ms, { 0, 0 }, false );
        }
    }
}

