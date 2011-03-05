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
** Software or, alternatively, in accordance with the terms contained in
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

#include "datafilebroker.h"
#include "datafile.h"
#include "datafile_factory.h"
#include <adportable/string.h>
#include <map>
#include <boost/smart_ptr.hpp>
#include <QLibrary>
# pragma warning(disable: 4996)
# include <ace/Singleton.h>
# pragma warning(default: 4996)

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
        factory_factory ffactory = static_cast<factory_factory>( lib.resolve( "datafile_factory" ) );
        if ( ffactory ) {
            datafile_factory * pfactory = ffactory();
            if ( pfactory )
                register_factory( pfactory, sharedlib );
            return true;
        }
    }
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