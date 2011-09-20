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

#include "brokerhelper.hpp"
#pragma warning (disable: 4996)
# include <tao/Object.h>
# include <adinterface/brokerC.h>
#pragma warning (default: 4996)

using namespace acewrapper;

brokerhelper::brokerhelper()
{
}

brokerhelper::~brokerhelper(void)
{
}

//static
Broker::Manager_ptr
brokerhelper::getManager( CORBA::ORB_ptr orb, const std::string& iorBrokerMgr )
{
    CORBA::Object_var obj = orb->string_to_object( iorBrokerMgr.c_str() );
    return Broker::Manager::_narrow( obj );
}

//static
CORBA::Object * 
brokerhelper::name_to_object( CORBA::ORB_ptr orb, const std::string& name, const std::string& iorBroker  )
{
    Broker::Manager_var mgr = getManager( orb, iorBroker );
    if ( ! CORBA::is_nil( mgr ) ) {
        CORBA::Object_var obj = orb->string_to_object( mgr->ior( name.c_str() ) );
        return obj._retn();
    }
    return 0;
}

//static
std::string
brokerhelper::ior( Broker::Manager * mgr, const char * name )
{
    if ( mgr ) {
        CORBA::String_var str = mgr->ior( name );
        return std::string( str );
    }
    return "";
}


