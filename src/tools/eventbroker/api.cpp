/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "eventbroker.h"
#include "document.hpp"

bool
eventbroker_regiser_handler( event_handler h )
{
    return eventbroker::document::instance()->register_handler( h );
}

bool
eventbroker_unregiser_handler( event_handler h )
{
    return eventbroker::document::instance()->unregister_handler( h );
}

bool
eventbroker_bind( const char * host, const char * port )
{
    return eventbroker::document::instance()->bind( host, port );
}

bool
eventbroker_out( uint32_t value )
{
    return eventbroker::document::instance()->event_out( value );
}
