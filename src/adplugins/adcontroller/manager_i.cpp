/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
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
#include "task.hpp"
#include <xmlparser/pugixml.hpp>
#include <cassert>

using namespace adcontroller;

manager_i * manager_i::instance_ = 0;
std::mutex manager_i::mutex_;

manager_i::manager_i(void) : orb_( 0 )
                           , poa_( 0 )
                           , poa_manager_( 0 )
{
}

manager_i::~manager_i(void)
{
}

// acewrapper::ORBServant< manager_i > *
// manager_i::instance()
// {
//     return singleton::manager_i::instance();
// }
manager_i *
manager_i::instance()
{
    if ( instance_ == 0 ) {
        std::lock_guard< std::mutex > lock( mutex_ );
        if ( instance_ == 0 )
            instance_ = new manager_i;
    }
    return instance_;
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
        adcontroller::iTask::instance()->open();

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

Broker::Manager_ptr
manager_i::getBrokerManager()
{
	return Broker::Manager::_duplicate( broker_mgr_.in() );
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
		(void)n;
		std::for_each( list.begin(), list.end(), [&]( pugi::xpath_node node ){
			std::string id = node.node().attribute( "id" ).value();
			std::string name = node.node().attribute( "name" ).value();
			std::string iid = node.node().attribute( "iid" ).value();
			broker_mgr_->register_lookup( iid.c_str(), id.c_str() );
		});
        iTask::instance()->setConfiguration( dom );
	}
    return static_cast<bool>( result );
}

CORBA::ORB * 
manager_i::orb()
{
    return CORBA::ORB::_duplicate( orb_ );
}

PortableServer::POA * 
manager_i::poa()
{
    return PortableServer::POA::_duplicate( poa_ );
}

PortableServer::POAManager *
manager_i::poa_manager()
{
    return PortableServer::POAManager::_duplicate( poa_manager_ );
}

void
manager_i::initialize( CORBA::ORB * orb, PortableServer::POA * poa, PortableServer::POAManager * mgr )
{
    orb_ = CORBA::ORB::_duplicate( orb );
    poa_ = PortableServer::POA::_duplicate( poa );
    poa_manager_ = PortableServer::POAManager::_duplicate( mgr );
}

const std::string&
manager_i::activate()
{
    PortableServer::ObjectId_var oid = poa_->activate_object( this );
    CORBA::Object_var obj = poa_->id_to_reference( oid.in() );
    CORBA::String_var str = orb_->object_to_string( obj.in() );
    ior_ = str;
    return ior_;
}

void
manager_i::deactivate()
{ 
	PortableServer::ObjectId_var oid = poa_->servant_to_id( this );
    this->poa_->deactivate_object ( oid );
}

const std::string&
manager_i::ior() const
{
    return ior_;
}
