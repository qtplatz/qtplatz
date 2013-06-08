/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
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

#include "manager_i.hpp"
#include "session_i.hpp"
#include "taskmanager.hpp"
#include <xmlparser/pugixml.hpp>

using namespace adcontroller;

namespace adcontroller {
    namespace singleton {
        typedef ACE_Singleton< acewrapper::ORBServant< ::adcontroller::manager_i >, ACE_Recursive_Thread_Mutex > manager_i;
    }
}

manager_i::manager_i(void) 
{
}

manager_i::~manager_i(void)
{
}

acewrapper::ORBServant< manager_i > *
manager_i::instance()
{
    return singleton::manager_i::instance();
}

void
manager_i::shutdown()
{
    // PortableServer::POA_var poa = singleton::manager::instance()->poa();
}

ControlServer::Session_ptr
manager_i::getSession( const CORBA::WChar * token )
{
    PortableServer::POA_var poa = manager_i::instance()->poa(); // getServantManager()->root_poa();

    if ( CORBA::is_nil( poa ) )
        return 0;

    if ( session_list_.empty() )
        adcontroller::iTaskManager::instance()->manager_initialize();

    session_map_type::iterator it = session_list_.find( token );
    if ( it == session_list_.end() ) 
        session_list_[ token ].reset( new adcontroller::session_i() );

    CORBA::Object_ptr obj = poa->servant_to_reference( session_list_[ token ].get() );
    return ControlServer::Session::_narrow( obj );
}

Broker::Logger_ptr
manager_i::getLogger()
{
    if ( ! CORBA::is_nil( broker_mgr_ ) ) {
        Broker::Logger_var logger = broker_mgr_->getLogger();
        return Broker::Logger::_duplicate( logger );
    }
    return 0;
}

bool
manager_i::setBrokerManager( Broker::Manager_ptr mgr )
{
    broker_mgr_ = Broker::Manager::_duplicate( mgr );
    return true;
}

bool
manager_i::adpluginspec( const char *id, const char * spec )
{
    adplugin_id = id;
    adplugin_spec = spec;

	if ( CORBA::is_nil( broker_mgr_ ) ) {
		assert( CORBA::is_nil( broker_mgr_ ) );
		return false;
	}

	pugi::xml_document dom;
	pugi::xml_parse_result result;
	if ( ( result = dom.load( spec ) ) ) {
		pugi::xpath_node_set list = dom.select_nodes( "//Module[@reference='lookup']" );
		size_t n = list.size();
		std::for_each( list.begin(), list.end(), [&]( pugi::xpath_node node ){
			std::string id = node.node().attribute( "id" ).value();
			std::string name = node.node().attribute( "name" ).value();
			std::string iid = node.node().attribute( "iid" ).value();
			broker_mgr_->register_lookup( iid.c_str(), id.c_str() );
		});
	}
    return true;
}
