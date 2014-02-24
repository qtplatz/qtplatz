/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternativey, in accordance with the terms contained in
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

#include "datafilebroker.hpp"
#include "datafile.hpp"
#include "datafile_factory.hpp"
#include <adportable/string.hpp>
#include <adportable/debug.hpp>
#include "adcontrols.hpp"
#include <mutex>
#include <memory>
#include <map>

using namespace adcontrols;

namespace adcontrols {
    class datafileBrokerImpl;

    class datafileBrokerImpl : public datafileBroker {
		static datafileBrokerImpl * instance_;
    public:
        ~datafileBrokerImpl() {}
        bool register_factory( datafile_factory * factory, const std::wstring& name );
        datafile_factory * find( const std::wstring& name );
        void visit( adcontrols::datafile& );

        datafile * open( const std::wstring& filename, bool readonly );
        datafile * create( const std::wstring& filename );
        static datafileBrokerImpl * instance();
        const std::map< std::wstring, std::shared_ptr< datafile_factory > >& factories() const { return factories_; }

    private:
        std::map< std::wstring, std::shared_ptr< datafile_factory > > factories_;
    };
    
}

datafileBrokerImpl * datafileBrokerImpl::instance_ = 0;

datafileBrokerImpl *
datafileBrokerImpl::instance()
{
    if ( instance_ == 0 ) {
        std::lock_guard< std::mutex > lock( adcontrols::global_mutex::mutex() );
        if ( instance_ == 0 )
			instance_ = new datafileBrokerImpl;
	}
	return instance_;
}


datafileBroker::~datafileBroker()
{
}

datafileBroker::datafileBroker()
{
}

bool
datafileBroker::register_factory( datafile_factory * factory, const std::wstring& name )
{
    return datafileBrokerImpl::instance()->register_factory( factory, name );
}

datafile_factory*
datafileBroker::find( const std::wstring& name )
{
    return datafileBrokerImpl::instance()->find( name );
}

datafile *
datafileBroker::open( const std::wstring& filename, bool readonly )
{
    return datafileBrokerImpl::instance()->open( filename, readonly );
}

// static
bool
datafileBroker::access( const std::wstring& filename )
{
    for ( auto f: datafileBrokerImpl::instance()->factories() ) {
		if ( f.second && f.second->access( filename.c_str() ) )
            return true;
    }
    return false;
}

datafile *
datafileBroker::create( const std::wstring& filename )
{
    return datafileBrokerImpl::instance()->create( filename );
}

//////////////////////////

bool
datafileBrokerImpl::register_factory( datafile_factory * factory, const std::wstring& name )
{
    factories_[ name ].reset( factory );
    return true;
}

void
datafileBrokerImpl::visit( adcontrols::datafile& )
{
    // factory_type factory = impl.factory();
}

datafile_factory *
datafileBrokerImpl::find( const std::wstring& name )
{
    std::map< std::wstring, std::shared_ptr<datafile_factory> >::iterator it = factories_.find( name );
    if ( it != factories_.end() )
        return it->second.get();
    return 0;
}

datafile *
datafileBrokerImpl::open( const std::wstring& name, bool readonly )
{
    std::map< std::wstring, std::shared_ptr<datafile_factory> >::iterator it;
    for ( it = factories_.begin(); it != factories_.end(); ++it ) {
        if ( it->second && it->second->access( name.c_str() ) ) {
            return it->second->open( name.c_str(), readonly );
        }
    }
    return 0;
}

datafile *
datafileBrokerImpl::create( const std::wstring& name )
{
    std::map< std::wstring, std::shared_ptr<datafile_factory> >::iterator it;
    for ( it = factories_.begin(); it != factories_.end(); ++it ) {
        if ( it->second && it->second->access( name.c_str(), adcontrols::write_access ) )
            return it->second->open( name.c_str(), false );
    }
    return 0;
}
