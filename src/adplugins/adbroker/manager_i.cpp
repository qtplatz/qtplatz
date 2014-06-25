// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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
#include "logger_i.hpp"
#include "objectdiscovery.hpp"
#include <acewrapper/orbservant.hpp>
#include <adlog/logger.hpp>
#include <adportable/string.hpp>
#include <regex>

using namespace adbroker;

namespace adbroker {
    namespace internal {

        struct object_receiver {
            inline bool operator == ( const object_receiver& t ) const {
                return sink_->_is_equivalent( t.sink_.in() );
            }
            inline bool operator == ( const Broker::ObjectReceiver_ptr t ) const {
                return sink_->_is_equivalent( t );
            }
            object_receiver() {
            }
            object_receiver( const object_receiver& t ) : sink_( t.sink_ ) {
            }
            Broker::ObjectReceiver_var sink_;
        };
    
    }
}

acewrapper::ORBServant< adbroker::manager_i > * manager_i::instance_ = 0;
std::mutex manager_i::mutex_;

manager_i::manager_i(void) : discovery_(0)
{
    ADTRACE() << "adbroker::manager_i ctor";
}

manager_i::~manager_i(void)
{
    ADTRACE() << "adbroker::~manager_i dtor";
    delete discovery_;
}

acewrapper::ORBServant< adbroker::manager_i > *
manager_i::instance()
{
    if ( instance_ == 0 ) {
        std::lock_guard< std::mutex > lock( mutex_ );
        if ( instance_ == 0 )
            instance_ = new acewrapper::ORBServant< adbroker::manager_i >;
    }
    return instance_;
}

void
manager_i::shutdown()
{
    ADTRACE() << "##### adbroker::manager::shutting down ... ######";

    if ( discovery_ ) {
        std::lock_guard< std::mutex > lock( mutex_ );
        if ( discovery_ )
            discovery_->close();
        delete discovery_;
        discovery_ = 0;
    }

    ADTRACE() << "##### adbroker::manager::shutdown complete ######";
}

Broker::Session_ptr
manager_i::getSession( const CORBA::WChar * token )
{
    PortableServer::POA_var poa = ::adbroker::manager_i::instance()->poa();

    if ( CORBA::is_nil( poa ) )
        return 0;

    session_map_type::iterator it = session_list_.find( token );
    if ( it == session_list_.end() )
        session_list_[ token ].reset( new adbroker::session_i( token ) );

    CORBA::Object_ptr obj = poa->servant_to_reference( session_list_[ token ].get() );
    try {
        return Broker::Session::_narrow( obj ); // return new object, refcount should be 1
    } catch ( CORBA::Exception& ) {
        return 0;
    }
}

Broker::Logger_ptr
manager_i::getLogger()
{
    PortableServer::POA_var poa = ::adbroker::manager_i::instance()->poa();
    if ( CORBA::is_nil( poa ) )
        return 0;

    if ( ! logger_i_ ) {
        logger_i_.reset( new broker::logger_i( poa ) );
        PortableServer::ObjectId * oid = poa->activate_object( logger_i_.get() );
        logger_i_->oid( *oid );
    }

    CORBA::Object_var obj = poa->servant_to_reference( logger_i_.get() );
    try {
        return Broker::Logger::_narrow( obj );
    } catch ( CORBA::Exception& ) {
        return 0;
    }
}

// CORBA interface
void
manager_i::register_ior( const char * name, const char * ior )
{
    internal_register_ior( name, ior );
}

// C++ direct interface
void
manager_i::internal_register_ior( const std::string& name, const std::string& ior )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    iorMap_[ name ] = ior;

	if ( objVec_.find( name ) == objVec_.end() ) {
		CORBA::ORB_var orb = manager_i::instance()->orb();
		CORBA::Object_var obj = orb->string_to_object( ior.c_str() );
        if ( ! CORBA::is_nil( obj.in() ) )
            register_object( name.c_str(), obj.in() );
	}

    for ( internal::object_receiver& cb: sink_vec_ )
        cb.sink_->object_discovered( name.c_str(), ior.c_str() );
}

char *
manager_i::ior( const char * name )
{
    std::lock_guard< std::mutex > lock( mutex_ );

    std::map< std::string, std::string >::iterator it = iorMap_.find( name );
    if ( it != iorMap_.end() )
        return CORBA::string_dup( it->second.c_str() );
    return 0;
}

void
manager_i::register_lookup( const char * name, const char * ident )
{
    if ( discovery_ == 0 ) {
        std::lock_guard< std::mutex > lock( mutex_ );
        if ( discovery_ == 0 ) {
            discovery_ = new ObjectDiscovery();
        }
    }
    if ( discovery_ ) {
        discovery_->open();
        discovery_->register_lookup( name, ident );

        std::lock_guard< std::mutex > lock( mutex_ );
        lookup_[ name ] = ident;
    } while(0);

#if defined DEBUG && 0
    adportable::debug() << "================================================================";
    adportable::debug() << "======== adbroker::manager_i::register_lookup(" 
                        << std::string(name) << ", " << std::string(ident) << ")";
#endif
}

bool
manager_i::register_object( const char * name, CORBA::Object_ptr obj )
{
    if ( CORBA::is_nil( obj ) ) {

        std::lock_guard< std::mutex > lock( mutex_ );

		auto it = objVec_.find( name );
        if ( it != objVec_.end() )
            objVec_.erase( it );
        return true;
    }
    
    objVec_[ name ] = CORBA::Object::_duplicate( obj );

    return true;
}

CORBA::Object_ptr
manager_i::find_object( const char * regex )
{
	std::regex re( regex );
	std::cmatch matches;
	auto itr = std::find_if( objVec_.begin(), objVec_.end(), [&](const std::map<std::string, CORBA::Object_var>::value_type& d) {
		return std::regex_match( d.first.c_str(), matches, re );
	} );
	if ( itr != objVec_.end() )
		return CORBA::Object::_duplicate( itr->second );
	return 0;
}

Broker::Objects *
manager_i::find_objects( const char * regex )
{
	Broker::Objects * results = 0;

	std::regex re( regex );
	std::cmatch matches;
	std::for_each( objVec_.begin(), objVec_.end(), [&](const std::map<std::string, CORBA::Object_var>::value_type& d) {
		if ( std::regex_match( d.first.c_str(), matches, re ) ) {
			if ( ! results ) 
				results = new Broker::Objects;
			results->length( results->length() + 1 );
			(*results)[ results->length() - 1 ] = CORBA::Object::_duplicate( d.second.in() );
		}
	});
	return results;
}

bool
manager_i::register_handler( Broker::ObjectReceiver_ptr cb )
{
    if ( ! CORBA::is_nil( cb ) ) {
        internal::object_receiver sink;
        sink.sink_ = Broker::ObjectReceiver::_duplicate( cb );

        std::lock_guard< std::mutex > lock( mutex_ );

        sink_vec_.push_back( sink );
        return true;
    }
    return false;
}

bool
manager_i::unregister_handler( Broker::ObjectReceiver_ptr cb )
{
    if ( ! CORBA::is_nil( cb ) ) {

        std::lock_guard< std::mutex > lock( mutex_ );

        auto it = std::find_if( sink_vec_.begin(), sink_vec_.end(), [&]( internal::object_receiver& t ){
                return t.sink_->_is_equivalent( cb );
            });
        if ( it != sink_vec_.end() ) {
            sink_vec_.erase( it );
            return true;
        }
    }
    return false;
}
