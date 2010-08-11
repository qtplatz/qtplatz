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
#include <string>
#include <vector>
#include "ConfigLoader.h"
#include <qtwrapper/qstring.h>
#include <QPluginLoader>
#include <QLibrary>
#include "ifactory.h"
#include "imonitor.h"

using namespace adplugin;

QMutex mutex;
manager * manager::instance_ = 0;

namespace adplugin {
	namespace internal {

		class manager_impl : public manager {
		public:
			~manager_impl();
			manager_impl();

			// manager impl
            bool loadConfig( adportable::Configuration&, const QString&, const wchar_t * );
            QObject * loadLibrary( const QString& );
			bool unloadLibrary( const QString& );

		private:
			std::map< QString, QObject * > libraries_;
		};
	}
}
////////////////////////////////////

using namespace adplugin::internal;

static void dispose_manager()
{
	manager::dispose();
}

manager::~manager()
{
}

manager::manager()
{
	atexit( &dispose_manager );
}

void
manager::dispose()
{
	if ( instance_ ) {
		mutex.lock();
		if ( instance_ ) {
			delete manager::instance_;
			manager::instance_ = 0;
		}
		mutex.unlock();
	}
}

manager *
manager::instance()
{
	if ( instance_ == 0 ) {
         mutex.lock();
		 if ( instance_ == 0 )
			 instance_ = new internal::manager_impl();
		 mutex.unlock();
	}
	return instance_;
}

// static
QWidget *
manager::widget_factory( const adportable::Configuration& config, const wchar_t * path, QWidget * parent )
{
	if ( config.module().library_filename().empty() )
		return 0;
	QString loadfile = qtwrapper::qstring( path ) + QString("/") + qtwrapper::qstring( config.module().library_filename() );

	IFactory * piFactory = qobject_cast< IFactory *> ( manager::instance()->loadLibrary( loadfile ) );
	if ( piFactory ) {
		QWidget * pWidget = piFactory->create_widget( config.component().c_str(), parent );
		adplugin::ui::IMonitor * pMonitor = qobject_cast< adplugin::ui::IMonitor *> ( pWidget );
		if ( pMonitor )
			pMonitor->OnInitialUpdate( config.xml().c_str() );
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
    for ( std::map<QString, QObject *>::iterator it = libraries_.begin(); it != libraries_.end(); ++it ) {
        if ( it->second ) {
            QPluginLoader loader( it->first );
            loader.unload();
        }
    }
}

bool
manager_impl::loadConfig( adportable::Configuration& config, const QString& filename, const wchar_t * query )
{
    return ConfigLoader::loadConfiguration( config, qtwrapper::wstring( filename ), query );
}

QObject *
manager_impl::loadLibrary( const QString& filename )
{
	std::map<QString, QObject *>::iterator it = libraries_.find( filename );
	if ( it == libraries_.end() ) {
        QPluginLoader loader ( filename );
        if ( loader.load() )
            libraries_[ filename ] = loader.instance();
	}
    if ( ( it = libraries_.find( filename ) ) != libraries_.end() )
        return it->second;
    return 0;
}

bool
manager_impl::unloadLibrary( const QString& filename )
{
	std::map<QString, QObject *>::iterator it = libraries_.find( filename );
	if ( it != libraries_.end() ) {
        QPluginLoader loader ( filename );
        if ( loader.unload() ) {
            libraries_.erase( it );
            return true;
        }
	}
    return false;
}
