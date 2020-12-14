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

#include "massspectrometerbroker.hpp"
#include "massspectrometer.hpp"
#include "massspectrometer_factory.hpp"
#include "adcontrols.hpp"
#include "constants.hpp"
#include <adportable/utf.hpp>
#include <adportable/debug.hpp>
#include <compiler/boost/workaround.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <algorithm>
#include <map>
#include <string>

using namespace adcontrols;


////////////////////////////////////////////////////////////////
/////// new generation
///////////////////////////////////////////////////////////////

namespace adcontrols {

    class MassSpectrometerBroker::impl {
    public:
        static impl& instance() {
            static impl __impl;
            return __impl;
        }

        ~impl() {
            if ( !factories_.empty() )
                clear();
        }

        void clear() {
#ifndef NDEBUG
            adportable::debug dbg( __FILE__, __LINE__ );
            size_t count(0);
            for ( const auto& a: factories_ )
                dbg << ( count++ == 0 ? "deleting factories: " : ", ") << a.second.first;
#endif
            for ( auto& a: factories_ )
                a.second.second.reset();

            factories_.clear();
        }

        typedef std::pair< std::string, std::shared_ptr< massspectrometer_factory > > value_type;

        std::map< boost::uuids::uuid, value_type > factories_;
    };

}

MassSpectrometerBroker::MassSpectrometerBroker()
{
}

MassSpectrometerBroker::~MassSpectrometerBroker()
{
}

//static
void
MassSpectrometerBroker::clear_factories()
{
    impl::instance().clear();
}

//static
bool
MassSpectrometerBroker::register_factory( std::shared_ptr< massspectrometer_factory >&& ptr )
{
    if ( ptr ) {
        auto& uuid = ptr->objclsid();
        auto text = ptr->objtext();
        impl::instance().factories_[ uuid ] = { text, ptr };
        return true;
    }
    return false;
}

//static
bool
MassSpectrometerBroker::register_factory( massspectrometer_factory* f )
{
    if ( f ) {
        if ( auto ptr = f->shared_from_this() )  {
            auto& uuid = f->objclsid();
            impl::instance().factories_ [ uuid ] = { f->objtext(), ptr };
            return true;
        }
    }
    return false;
}

//static
massspectrometer_factory*
MassSpectrometerBroker::find_factory( const boost::uuids::uuid& uuid )
{
    auto it = impl::instance().factories_.find( uuid );
    if ( it != impl::instance().factories_.end() ) {
        if ( auto ptr = it->second.second )
            return ptr.get();
    }
    return nullptr;
}

massspectrometer_factory*
MassSpectrometerBroker::find_factory( const std::string& objtext )
{
    auto it = std::find_if( impl::instance().factories_.begin(), impl::instance().factories_.end()
                            , [&] ( const std::pair<boost::uuids::uuid, impl::value_type>& a ) { return objtext == a.second.first; } );
    if ( it != impl::instance().factories_.end() ) {
        if ( auto ptr = it->second.second )
            return ptr.get();
    }
    return nullptr;
}

std::shared_ptr< adcontrols::MassSpectrometer >
MassSpectrometerBroker::make_massspectrometer( const boost::uuids::uuid& uuid )
{
    if ( auto factory = find_factory( uuid ) ) {
        return factory->create();
    }
    return nullptr;
}

std::shared_ptr< adcontrols::MassSpectrometer >
MassSpectrometerBroker::make_massspectrometer( const std::string& objtext )
{
    if ( auto factory = find_factory( objtext ) )
        return factory->create();

    return nullptr;
}

//static
std::vector< std::pair< boost::uuids::uuid, std::string > >
MassSpectrometerBroker::installed_uuids()
{
    std::vector< std::pair< boost::uuids::uuid, std::string > > values;
    for ( auto& pair: impl::instance().factories_ )
        values.emplace_back( pair.first, pair.second.first );
    return values;
}
