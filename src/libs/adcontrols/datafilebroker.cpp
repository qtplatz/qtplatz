/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternativey, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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
#include <map>
#include <boost/smart_ptr.hpp>
#include <QLibrary>
#include <adportable/debug.hpp>

# include <ace/Singleton.h>

using namespace adcontrols;

namespace adcontrols {
    class datafileBrokerImpl;

    namespace singleton {
        typedef ACE_Singleton< datafileBrokerImpl, ACE_Recursive_Thread_Mutex> datafileBrokerImpl;
    }

    class datafileBrokerImpl : public datafileBroker {
    public:
        ~datafileBrokerImpl() {}
        bool register_library( const std::wstring& sharedlib_name );
        bool register_factory( datafile_factory * factory, const std::wstring& name );
        datafile_factory * find( const std::wstring& name );
        void visit( adcontrols::datafile& );

        datafile * open( const std::wstring& filename, bool readonly );
        datafile * create( const std::wstring& filename );

    private:
        std::map< std::wstring, boost::shared_ptr< datafile_factory > > factories_;
    };
    
    typedef ACE_Singleton< datafileBrokerImpl, ACE_Recursive_Thread_Mutex > impl_type;
}

datafileBroker::~datafileBroker()
{
}

datafileBroker::datafileBroker()
{
}

bool
datafileBroker::register_library( const std::wstring& sharedlib )
{
    return singleton::datafileBrokerImpl::instance()->register_library( sharedlib );
}

bool
datafileBroker::register_factory( datafile_factory * factory, const std::wstring& name )
{
    return singleton::datafileBrokerImpl::instance()->register_factory( factory, name );
}

datafile_factory*
datafileBroker::find( const std::wstring& name )
{
    return singleton::datafileBrokerImpl::instance()->find( name );
}

datafile *
datafileBroker::open( const std::wstring& filename, bool readonly )
{
    return singleton::datafileBrokerImpl::instance()->open( filename, readonly );
}

datafile *
datafileBroker::create( const std::wstring& filename )
{
    return singleton::datafileBrokerImpl::instance()->create( filename );
}

//////////////////////////

bool
datafileBrokerImpl::register_factory( datafile_factory * factory, const std::wstring& name )
{
    factories_[ name ].reset( factory );
    return true;
}

bool
datafileBrokerImpl::register_library( const std::wstring& sharedlib )
{
    std::string mbs = adportable::string::convert( sharedlib );
    QLibrary lib( mbs.c_str() );
    if ( lib.load() ) {
        typedef adcontrols::datafile_factory * (*factory_factory)();
        factory_factory ffactory = reinterpret_cast<factory_factory>( lib.resolve( "datafile_factory" ) );
        if ( ffactory ) {
            datafile_factory * pfactory = ffactory();
            if ( pfactory )
                register_factory( pfactory, sharedlib );
            return true;
        }
    }
    adportable::debug(__FILE__, __LINE__) << lib.errorString().toStdString();
    return false;
}

void
datafileBrokerImpl::visit( adcontrols::datafile& )
{
    // factory_type factory = impl.factory();
}

datafile_factory *
datafileBrokerImpl::find( const std::wstring& name )
{
    std::map< std::wstring, boost::shared_ptr<datafile_factory> >::iterator it = factories_.find( name );
    if ( it != factories_.end() )
        return it->second.get();
    return 0;
}

datafile *
datafileBrokerImpl::open( const std::wstring& name, bool readonly )
{
    std::map< std::wstring, boost::shared_ptr<datafile_factory> >::iterator it;
    for ( it = factories_.begin(); it != factories_.end(); ++it ) {
        if ( it->second && it->second->access( name ) ) {
            return it->second->open( name, readonly );
        }
    }
    return 0;
}

datafile *
datafileBrokerImpl::create( const std::wstring& name )
{
    std::map< std::wstring, boost::shared_ptr<datafile_factory> >::iterator it;
    for ( it = factories_.begin(); it != factories_.end(); ++it ) {
        if ( it->second && it->second->access( name, adcontrols::write_access ) )
            return it->second->open( name, false );
    }
    return 0;
}
