/**************************************************************************
** Copyright (C) 2014-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2014-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "masterobserverevents.hpp" 
#include "mastercontroller.hpp"
#include <adportable/debug.hpp>

#if defined _DEBUG
# include <chrono>
#endif

using namespace acquire;

/*
 *  ObserverEvents -- implementation helper
 */

MasterObserverEvents::MasterObserverEvents(MasterController * p) : controller_(p->pThis())
{
}

MasterObserverEvents::~MasterObserverEvents()
{
}


void
MasterObserverEvents::OnConfigChanged( uint32_t objId, adicontroller::SignalObserver::eConfigStatus status )
{
}
    
void
MasterObserverEvents::OnUpdateData( uint32_t objId, long pos )
{
}
    
void
MasterObserverEvents::OnMethodChanged( uint32_t objId, long pos )
{
}
    
void
MasterObserverEvents::OnEvent( uint32_t objId, uint32_t event, long pos )
{
}
    
void
MasterObserverEvents::onDataChanged( adicontroller::SignalObserver::Observer * so, uint32_t pos )
{
#if defined _DEBUG
    static std::map< std::string, std::chrono::steady_clock::time_point > __last;
    std::chrono::steady_clock::time_point tp = std::chrono::steady_clock::now();
    if ( std::chrono::duration_cast<std::chrono::milliseconds>( tp - __last[ so->objtext() ] ).count() > 3000 ) {
        ADDEBUG() << so->objtext() << " pos = " << pos;
        __last[ so->objtext() ] = tp;
    }
#endif

    if ( auto p = controller_.lock() )
        p->invokeDataChanged( so, pos );

}
    
