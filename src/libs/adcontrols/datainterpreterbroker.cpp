// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
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

#include "datainterpreterbroker.hpp"
#include "datainterpreter_factory.hpp"
#include "datainterpreter.hpp"
#include <adportable/string.hpp>
#include <adportable/debug.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <algorithm>
#include <map>
#include <string>

using namespace adcontrols;

namespace adcontrols {

    class DataInterpreterBroker::impl {
    public:
        static impl& instance() {
            static impl __impl;
            return __impl;
        }

        impl() {
        }

        typedef std::pair< std::string, std::shared_ptr< DataInterpreterFactory > > value_type;
        std::map< boost::uuids::uuid, value_type > factories_;
#if _MSC_VER < 1900
        static boost::uuids::uuid base_uuid;
#else
        static constexpr boost::uuids::uuid base_uuid = boost::uuids::uuid({ 0xD0, 0xE0, 0x5C, 0xF0, 0x7E, 0x81, 0x46, 0xD1, 0xA1, 0x43, 0x47, 0x7A, 0xB4, 0x1B, 0x73, 0x5F });
#endif
    };

#if _MSC_VER < 1900
    boost::uuids::uuid DataInterpreterBroker::impl::base_uuid = boost::uuids::uuid( { 0xD0, 0xE0, 0x5C, 0xF0, 0x7E, 0x81, 0x46, 0xD1, 0xA1, 0x43, 0x47, 0x7A, 0xB4, 0x1B, 0x73, 0x5F } );
#else
    boost::uuids::uuid constexpr DataInterpreterBroker::impl::base_uuid;// = boost::uuids::string_generator()( "{D0E05CF0-7E81-46D1-A143-477AB41B735F}" );
#endif

}

DataInterpreterBroker::DataInterpreterBroker()
{
}

DataInterpreterBroker::~DataInterpreterBroker()
{
}

bool
DataInterpreterBroker::register_factory( std::shared_ptr< DataInterpreterFactory > factory, const boost::uuids::uuid& uuid, const std::string& dataInterpreterClsid )
{
    if ( uuid == boost::uuids::uuid{ 0 } ) {
        impl::instance().factories_[ name_to_uuid( dataInterpreterClsid ) ] = std::make_pair( dataInterpreterClsid, factory );
    } else {
        impl::instance().factories_[ uuid ] = std::make_pair( dataInterpreterClsid, factory );
    }
    return true;
}

DataInterpreterFactory *
DataInterpreterBroker::find_factory( const std::string& dataInterpreterClsid )
{
    auto it = std::find_if( impl::instance().factories_.begin(), impl::instance().factories_.end()
                            , [&] ( const std::pair<boost::uuids::uuid, impl::value_type>& a ) { return dataInterpreterClsid == a.second.first; } );
    if ( it != impl::instance().factories_.end() ) {
        if ( auto ptr = it->second.second )
            return ptr.get();
    }
    return nullptr;
}

DataInterpreterFactory *
DataInterpreterBroker::find_factory( const boost::uuids::uuid& uuid )
{
    auto it = impl::instance().factories_.find( uuid );
    if ( it != impl::instance().factories_.end() ) {
        if ( auto ptr = it->second.second )
            return ptr.get();
    }
    return nullptr;
}

std::shared_ptr< adcontrols::DataInterpreter >
DataInterpreterBroker::make_datainterpreter( const boost::uuids::uuid& uuid )
{
    if ( auto factory = find_factory( uuid ) ) {
        return factory->create();
    }
    return nullptr;
}

std::shared_ptr< adcontrols::DataInterpreter >
DataInterpreterBroker::make_datainterpreter( const std::string& objtext )
{
    if ( auto factory = find_factory( objtext ) )
        return factory->create();

    if ( auto factory = find_factory( name_to_uuid( objtext ) ) )
        return factory->create();

    return nullptr;
}

//static
boost::uuids::uuid
DataInterpreterBroker::name_to_uuid( const std::string& dataInterpreterClsid )
{
    return boost::uuids::name_generator( impl::base_uuid )( dataInterpreterClsid );
}

std::vector< std::pair< boost::uuids::uuid, std::string > >
DataInterpreterBroker::installed_uuids()
{
    std::vector< std::pair< boost::uuids::uuid, std::string > > values;
    for ( auto& pair: impl::instance().factories_ )
        values.push_back( std::make_pair( pair.first, pair.second.first ) );
    return values;
}
