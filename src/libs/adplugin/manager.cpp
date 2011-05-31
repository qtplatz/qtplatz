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

#include "manager.hpp"
#include "adplugin.hpp"
#include "lifecycle.hpp"
#include "orbLoader.hpp"
#include <adportable/configuration.hpp>
#include <acewrapper/constants.hpp>
#include <adportable/configloader.hpp>
#include <adportable/debug.hpp>
#include <adportable/string.hpp>
#include <qtwrapper/qstring.hpp>
#include <QLibrary>
#include <QDir>
#include <QMessageBox>

#include <map>
#include <fstream>
#include <boost/smart_ptr.hpp>
#include <boost/filesystem.hpp>
#include <QWidget>

#if defined _MSC_VER
# pragma warning(disable:4996)
#endif
#include <ace/Singleton.h>

//----------------------------------

using namespace adplugin;

manager::manager(void)
{
}

manager::~manager(void)
{
}

////////////////

namespace adplugin {

    class manager_impl : public manager {
    public:
        ~manager_impl();
        manager_impl();

        // manager impl
        bool loadConfig( adportable::Configuration&, const std::wstring&, const wchar_t * );
        adplugin::ifactory * loadFactory( const std::wstring& );
        bool unloadFactory( const std::wstring& );

        virtual adplugin::orbLoader& orbLoader( const std::wstring& name );

        virtual void register_ior( const std::string& name, const std::string& ior );
        virtual const char * lookup_ior( const std::string& name );

    private:
        typedef std::map< std::wstring, adplugin::ifactory * > librariesType;
        typedef std::map< std::wstring, boost::shared_ptr<adplugin::orbLoader> > orbLoadersType;

        librariesType libraries_;
        orbLoadersType orbLoaders_;
		orbLoadersType failedLoaders_;
        std::map< std::string, std::string > iorMap_;
    };
}
////////////////////////////////////
manager *
manager::instance()
{
    typedef ACE_Singleton< manager_impl, ACE_Recursive_Thread_Mutex > impl;
    return impl::instance();
}

// static
QWidget *
manager::widget_factory( const adportable::Configuration& config, const wchar_t * path, QWidget * parent )
{
	if ( config.module().library_filename().empty() )
		return 0;

    std::wstring appbase( path );
	std::wstring loadfile = appbase + config.module().library_filename();

    adplugin::ifactory * pfactory = manager::instance()->loadFactory( loadfile );
	if ( pfactory ) {
        QWidget * pWidget = pfactory->create_widget( config.interface().c_str(), parent );
        adplugin::LifeCycle * pLifeCycle = dynamic_cast< adplugin::LifeCycle * > ( pWidget );
		if ( pLifeCycle )
            pLifeCycle->OnCreate( config );
		return pWidget;
	}
    return 0;
}

std::string
manager::iorBroker()
{
	return manager::instance()->lookup_ior( acewrapper::constants::adbroker::manager::_name() );
}

std::string
manager::ior( const char * name )
{
	return manager::instance()->lookup_ior( name );
}

///////////////////////////////////

manager_impl::manager_impl()
{
}

manager_impl::~manager_impl()
{
}

bool
manager_impl::loadConfig( adportable::Configuration& config, const std::wstring& filename, const wchar_t * query )
{
    return adportable::ConfigLoader::loadConfigFile( config, filename, query );
}

adplugin::ifactory *
manager_impl::loadFactory( const std::wstring& filename )
{
	librariesType::iterator it = libraries_.find( filename );
	if ( it == libraries_.end() ) {
        QLibrary lib( qtwrapper::qstring::copy(filename) );
        if ( lib.load() ) {
            typedef adplugin::ifactory * (*ad_plugin_instance_t)();
            ad_plugin_instance_t instance = reinterpret_cast<ad_plugin_instance_t>( lib.resolve( "ad_plugin_instance" ) );
            if ( instance ) {
                libraries_[ filename ] = instance();
            } else {
                QMessageBox::critical( 0, "adplugin::orbLoader", "some method(s) can not be assigned" );
            }
        }
    }
    if ( ( it = libraries_.find( filename ) ) != libraries_.end() )
        return it->second;

    return 0;
}

bool
manager_impl::unloadFactory( const std::wstring& filename )
{
	librariesType::iterator it = libraries_.find( filename );
	if ( it != libraries_.end() ) {
        if ( it->second )
            it->second->release();
        return true;
	}
    return false;
}

void
manager_impl::register_ior( const std::string& name, const std::string& ior )
{
	iorMap_[name] = ior;

	QDir dir = QDir::home();
	if ( ! dir.exists( ".ior" ) )
		dir.mkdir( ".ior" );
	dir.cd( ".ior" );
	std::string path = dir.absolutePath().toStdString();
	path += std::string("/") + name + ".ior";
	std::ofstream of( path.c_str() );
	of << ior;
}

const char *
manager_impl::lookup_ior( const std::string& name )
{
#if defined _DEBUG
	std::string path = QDir::home().absolutePath().toStdString();
	path += std::string( "/.ior/" ) + name + ".ior";
	std::ifstream inf( path.c_str() );

	if ( ! inf.fail() ) {
		std::string ior;
		inf >> ior;
	}
#endif

	std::map< std::string, std::string >::iterator it = iorMap_.find( name );
	if ( it != iorMap_.end() )
		return it->second.c_str();
	return 0;
}


//////////////////////////////////////
////////////////////////////////////////

class ORBLoaderError : public adplugin::orbLoader {
	std::string errmsg_;
public:
	ORBLoaderError( const std::string& errmsg ) : errmsg_( errmsg ) {}
	~ORBLoaderError() { }
	virtual operator bool() const { return false; }
	virtual bool initialize( CORBA::ORB *, PortableServer::POA * , PortableServer::POAManager * ) { return false; }
	virtual void initial_reference( const char * ) { }
	virtual const char * activate() { return ""; }
	virtual bool deactivate() { return false; }
	virtual const char * error_description() { return errmsg_.c_str(); }
};

//-------------------
adplugin::orbLoader&
manager_impl::orbLoader( const std::wstring& file )
{
    orbLoadersType::iterator it = orbLoaders_.find( file );
    
    if ( it != orbLoaders_.end() )
	return *it->second;

    boost::filesystem::path filepath( file );
    boost::system::error_code ec;
    if ( ! boost::filesystem::exists( file, ec ) ) {
	adportable::debug dbg;
	dbg << "error: " << ec.message() << " '" << ec.category().name() << "'";
	failedLoaders_[ file ].reset( new ORBLoaderError( dbg.str() ) );
	return *failedLoaders_[ file ];
    }
    
    QLibrary lib( qtwrapper::qstring::copy( file ) );
    if ( lib.load() ) {
	typedef adplugin::orbLoader * (*instance_t)();
	instance_t instance = reinterpret_cast<instance_t>( lib.resolve( "instance" ) );
	if ( instance ) {
	    boost::shared_ptr< adplugin::orbLoader > loader( instance() );
	    if ( loader )
		orbLoaders_[ file ] = loader;
	} else {
	    failedLoaders_[ file ].reset( new ORBLoaderError( "dll entry point 'instance' can not be found" ) );
	    return *failedLoaders_[ file ];
	}
    }
    if ( ( it = orbLoaders_.find( file ) ) != orbLoaders_.end() )
	return *it->second;
    
    failedLoaders_[ file ].reset( new ORBLoaderError( "dll load failed." ) );
    return *failedLoaders_[ file ];
}
