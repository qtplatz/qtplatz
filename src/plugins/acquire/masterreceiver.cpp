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

#include "masterreceiver.hpp"
#include "mastercontroller.hpp"

using namespace acquire;

MasterReceiver::MasterReceiver( MasterController * p ) : controller_( p->pThis() )
{
}

MasterReceiver::~MasterReceiver()
{
}

void
MasterReceiver::message( eINSTEVENT msg, uint32_t value )
{
    if ( auto p = controller_.lock() )
        emit p->message( p.get(), unsigned( msg ), unsigned( value ) );
}
            
void
MasterReceiver::log( const adacquire::EventLog::LogMessage& log )
{
}
            
void
MasterReceiver::shutdown()
{
}
            
void
MasterReceiver::debug_print( uint32_t priority, uint32_t category, const std::string& text )
{
}        
        
