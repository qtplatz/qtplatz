//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "session_i.h"
#include "manager_i.h"
#include "chemicalformula_i.h"
#include "brokermanager.h"

using namespace adbroker;

session_i::session_i(void)
{
}

session_i::~session_i(void)
{
}

bool
session_i::connect( const char * user, const char * pass, const char * token )
{
    ACE_UNUSED_ARG( user );
    ACE_UNUSED_ARG( pass );
    ACE_UNUSED_ARG( token );

    adbroker::Task * pTask = adbroker::singleton::BrokerManager::instance()->get<adbroker::Task>();
    if ( pTask )
		return true;
    return false;
}

Broker::ChemicalFormula_ptr
session_i::getChemicalFormula()
{
    PortableServer::POA_var poa = ::adbroker::singleton::manager::instance()->poa();

    if ( CORBA::is_nil( poa ) )
        return 0;

    ChemicalFormula_i * p = new ChemicalFormula_i();
    if ( p ) {
        CORBA::Object_ptr obj = poa->servant_to_reference( p );
        try {
            Broker::ChemicalFormula_var var = Broker::ChemicalFormula::_narrow( obj );
            return var._retn();
        } catch ( CORBA::Exception& ) {
        }
    }
    return 0;
}