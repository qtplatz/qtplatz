// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "adplugin.h"
#include <QMutex>
#include <QString>
#include <stdlib.h>
#include <adportable/configuration.h>
#include <adportable/configloader.h>
#include <adportable/debug.h>
#include <string>
#include <vector>
#include <qtwrapper/qstring.h>
#include <QPluginLoader>
#include <QLibrary>
#include <QDir>
#include "ifactory.h"
#include "imonitor.h"
#include "orbLoader.h"
#include <boost/smart_ptr.hpp>
#include <fstream>

#pragma warning (disable: 4996)
#include <ace/Init_ACE.h>
#include <ace/Singleton.h>
#pragma warning (default: 4996)

#if defined ACE_WIN32
#  if defined _DEBUG
#     pragma comment(lib, "TAO_Utilsd.lib")
#     pragma comment(lib, "TAO_PId.lib")
#     pragma comment(lib, "TAO_PortableServerd.lib")
#     pragma comment(lib, "TAO_AnyTypeCoded.lib")
#     pragma comment(lib, "TAO_CosNamingd.lib")
#     pragma comment(lib, "TAOd.lib")
#     pragma comment(lib, "ACEd.lib")
#  else
#     pragma comment(lib, "TAO_Utils.lib")
#     pragma comment(lib, "TAO_PI.lib")
#     pragma comment(lib, "TAO_PortableServer.lib")
#     pragma comment(lib, "TAO_AnyTypeCode.lib")
#     pragma comment(lib, "TAO_CosNaming.lib")
#     pragma comment(lib, "TAO.lib")
#     pragma comment(lib, "ACE.lib")
#  endif
#endif

#if defined WIN32
# if defined _DEBUG
#     pragma comment(lib, "adinterfaced.lib")
#     pragma comment(lib, "adportabled.lib")
#     pragma comment(lib, "acewrapperd.lib")
#     pragma comment(lib, "qtwrapperd.lib")
#     pragma comment(lib, "xmlwrapperd.lib")
# else
#     pragma comment(lib, "adinterface.lib")
#     pragma comment(lib, "adportable.lib")
#     pragma comment(lib, "acewrapper.lib")
#     pragma comment(lib, "qtwrapper.lib")
#     pragma comment(lib, "xmlwrapper.lib")
# endif
#endif


using namespace adplugin;

namespace adplugin {
	namespace internal {

		class manager_impl : public manager {
		public:
			~manager_impl();
			manager_impl();

			// manager impl
			bool loadConfig( adportable::Configuration&, const std::wstring&, const wchar_t * );
			QObject * loadLibrary( const std::wstring& );
			bool unloadLibrary( const std::wstring& );

			virtual adplugin::orbLoader& orbLoader( const std::wstring& name );

			virtual void register_ior( const std::string& name, const std::string& ior );
			virtual const char * lookup_ior( const std::string& name );

		private:
			typedef std::map< std::wstring, QObject * > librariesType;
			typedef std::map< std::wstring, boost::shared_ptr<adplugin::orbLoader> > orbLoadersType;

			librariesType libraries_;
			orbLoadersType orbLoaders_;
			std::map< std::string, std::string > iorMap_;
		};
	}
}
////////////////////////////////////

using namespace adplugin::internal;


manager::~manager()
{
}

manager::manager()
{
}

manager *
manager::instance()
{
    typedef ACE_Singleton< internal::manager_impl, ACE_Recursive_Thread_Mutex > impl;
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

	IFactory * piFactory = qobject_cast< IFactory *> ( manager::instance()->loadLibrary( loadfile ) );
	if ( piFactory ) {
		QWidget * pWidget = piFactory->create_widget( config.interface().c_str(), parent );
        adplugin::LifeCycle * pLifeCycle = dynamic_cast< adplugin::LifeCycle * > ( pWidget );
		if ( pLifeCycle )
            pLifeCycle->OnCreate( config );
		return pWidget;
	}
    return 0;
}


////////////////////////////////////////////////////////
////////////////////////////////////////////////////////

manager_impl::manager_impl()
{
}

manager_impl::~manager_impl()
{
	for ( librariesType::iterator it = libraries_.begin(); it != libraries_.end(); ++it ) {
        if ( it->second ) {
			QPluginLoader loader( qtwrapper::qstring(it->first) );
            loader.unload();
        }
    }
}

bool
manager_impl::loadConfig( adportable::Configuration& config, const std::wstring& filename, const wchar_t * query )
{
    return adportable::ConfigLoader::loadConfigFile( config, filename, query );
}

QObject *
manager_impl::loadLibrary( const std::wstring& filename )
{
	librariesType::iterator it = libraries_.find( filename );
	if ( it == libraries_.end() ) {
		QPluginLoader loader ( qtwrapper::qstring::copy(filename) );
		if ( loader.load() )
            libraries_[ filename ] = loader.instance();
	}
    if ( ( it = libraries_.find( filename ) ) != libraries_.end() )
        return it->second;
    return 0;
}

bool
manager_impl::unloadLibrary( const std::wstring& filename )
{
	librariesType::iterator it = libraries_.find( filename );
	if ( it != libraries_.end() ) {
		QPluginLoader loader ( qtwrapper::qstring::copy(filename) );
        if ( loader.unload() ) {
            libraries_.erase( it );
            return true;
        }
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
	path += std::string( "/.ior" ) + name + ".ior";
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

class orbLoaderImpl : public adplugin::orbLoader {
public:
	~orbLoaderImpl() {}
	orbLoaderImpl() : initialize_(0)
                    , activate_(0)
					, deactivate_(0)
					, run_(0)
					, abort_server_(0) {
	}

	orbLoaderImpl( QLibrary& lib ) : initialize_(0)
                                   , activate_(0)
								   , deactivate_(0)
								   , run_(0)
								   , abort_server_(0)
	{
		initialize_   = static_cast<initialize_t>( lib.resolve( "initialize" ) );
		activate_     = static_cast<activate_t>( lib.resolve( "activate" ) );
		deactivate_   = static_cast<deactivate_t>( lib.resolve( "deactivate" ) );
		run_          = static_cast<run_t>( lib.resolve( "run" ) );
		abort_server_ = static_cast<abort_server_t>( lib.resolve( "abort_server" ) );
	}

	virtual operator bool () const {
		return initialize_ && activate_ && deactivate_ && run_ && abort_server_;
	}

private:
	typedef bool (*initialize_t)( CORBA::ORB * );
	typedef const char * (*activate_t)();
	typedef bool (*deactivate_t)();
	typedef bool (*run_t)();
	typedef bool (*abort_server_t)();
public:
	virtual bool initialize( CORBA::ORB * orb ) { return initialize_   ? initialize_( orb ) : false; }
	virtual const char * activate()             { return activate_     ? activate_()        : false; }
	virtual bool deactivate()                   { return deactivate_   ? deactivate_()      : false; }
	virtual int run()                           { return run_          ? run_()             : false; }
	virtual void abort_server()                 { return abort_server_ ? abort_server()     : false; }
private:
	initialize_t   initialize_;
	activate_t     activate_;
	deactivate_t   deactivate_;
	run_t          run_;
	abort_server_t abort_server_;
};

////////////////////////////////////////
adplugin::orbLoader&
manager_impl::orbLoader( const std::wstring& file )
{
	orbLoadersType::iterator it = orbLoaders_.find( file );

	if ( it != orbLoaders_.end() )
		return *it->second;

	static orbLoaderImpl empty;

    int rcode = _waccess( file.c_str(), 0 );
	if ( rcode ) {
		adportable::debug out;
		const char * reason = 0;
        if ( errno == EACCES ) reason = "access denied";
		else if ( errno == ENOENT ) reason = "file name or path not found";
		else if ( errno == EINVAL ) reason = "invalid parameter";
		else reason = "n/a";
		out << file << " access filed by " << reason;
		return empty;
	}
    
	QLibrary lib( qtwrapper::qstring::copy( file ) );
	if ( lib.load() ) {
		typedef adplugin::orbLoader * (*instance_t)();
		instance_t instance = static_cast<instance_t>( lib.resolve( "instance" ) );
		if ( instance ) {
			boost::shared_ptr< adplugin::orbLoader > loader( instance() );
			if ( loader )
				orbLoaders_[ file ] = loader;
		} else {
			boost::shared_ptr< adplugin::orbLoader > loader( new orbLoaderImpl( lib ) );
			if ( loader )
				orbLoaders_[ file ] = loader;
		}
	}
	if ( ( it = orbLoaders_.find( file ) ) != orbLoaders_.end() )
		return *it->second;
	return empty;
}
