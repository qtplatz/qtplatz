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
#include "error_code.hpp"
#include <adportable/string.hpp>
#include <adportable/debug.hpp>
#include "adcontrols.hpp"
#include <atomic>
#include <mutex>
#include <memory>
#include <map>
#include <stdexcept>

using namespace adcontrols;

namespace adcontrols {

    class datafileBrokerImpl;

    class datafileBrokerImpl : public datafileBroker {
        static std::atomic<datafileBrokerImpl *> instance_;
        static std::once_flag flag;
    public:
        ~datafileBrokerImpl() {
            ADDEBUG() << "###### datafileBrokerImpl -- dtor ######### " << this;
        }

        bool register_factory( datafile_factory * factory, const std::string& uniqname );

        void visit( adcontrols::datafile& );

        // [[deprecated]] datafile * open( const std::wstring& filename, bool readonly, error_code * );
        // [[deprecated]] datafile * create( const std::wstring& filename, error_code * );

        datafile * open( const std::filesystem::path& filename, bool readonly, error_code * );
        datafile * create( const std::filesystem::path& filename, error_code * );

        static datafileBrokerImpl * instance();

        const std::map< std::string, std::shared_ptr< datafile_factory > >& factories() const { return factories_; }

        void clear() {
            // don't delete since factory is allocated in each shared library
        }
    private:
        std::map < std::string, std::shared_ptr< datafile_factory > > factories_;
    };

}

std::atomic<datafileBrokerImpl * > datafileBrokerImpl::instance_( 0 );
std::once_flag datafileBrokerImpl::flag;

datafileBrokerImpl *
datafileBrokerImpl::instance()
{
    std::call_once( flag, [] () { instance_ = new datafileBrokerImpl(); } );
	return instance_;
}


datafileBroker::~datafileBroker()
{
    ADDEBUG() << "###### datafileBroker -- dtor ######### " << this;
}

datafileBroker::datafileBroker()
{
}

bool
datafileBroker::register_factory( datafile_factory * factory, const std::string& uniqname )
{
    return datafileBrokerImpl::instance()->register_factory( factory, uniqname );
}

datafile *
datafileBroker::open( const std::filesystem::path& path, bool readonly, error_code * ec )
{
    return datafileBrokerImpl::instance()->open( path, readonly, ec );
}

// static
bool
datafileBroker::access( const std::filesystem::path& path )
{
    for ( auto f: datafileBrokerImpl::instance()->factories() ) {
		if ( f.second && f.second->access( path ) )
            return true;
    }
    return false;
}

datafile *
datafileBroker::create( const std::filesystem::path& path, error_code * ec )
{
    return datafileBrokerImpl::instance()->create( path, ec );
}

//static
void
datafileBroker::clear_factories()
{
    datafileBrokerImpl::instance()->clear();
}

//////////////////////////

bool
datafileBrokerImpl::register_factory( datafile_factory * factory, const std::string& uniqname )
{
    if ( uniqname.empty() )
        throw std::invalid_argument( "uniqname" );

    factories_[ uniqname ].reset( factory );

    return true;
}

void
datafileBrokerImpl::visit( adcontrols::datafile& )
{
    // factory_type factory = impl.factory();
}


datafile *
datafileBrokerImpl::open( const std::filesystem::path& path, bool readonly, error_code * ec )
{
    if ( ec )
        *ec = {};
    for ( auto it = factories_.begin(); it != factories_.end(); ++it ) {
        if ( it->second && it->second->access( path ) )
            return it->second->open( path, readonly );
    }
    if ( ec ) {
        ec->assign( -1, "No data factory installed" );
    }
    return 0;
}

datafile *
datafileBrokerImpl::create( const std::filesystem::path& path, error_code * ec )
{
    for ( auto it = factories_.begin(); it != factories_.end(); ++it ) {
        if ( it->second && it->second->access( path, adcontrols::write_access ) )
            return it->second->open( path, false );
    }

    if ( factories_.empty() && ec )
        ec->assign( -1, "No data factory installed" );

    return 0;
}
