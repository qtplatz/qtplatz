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
#include <adportable/component.h>
#include <string>
#include <vector>
#include "ConfigLoader.h"
#include <qtwrapper/qstring.h>

using namespace adplugin;

QMutex mutex;
manager * manager::instance_ = 0;

namespace adplugin {
	namespace internal {
		class manager_impl : public manager {
		public:
			~manager_impl() {}
			manager_impl();

			// manager impl
            bool loadConfig( adportable::Configuration&, const QString&, const wchar_t * );
		private:
            // adportable::Configuration rootConfig_; // root is a place folder, which is always empty
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

////////////////////////////////////////////////////////
////////////////////////////////////////////////////////

manager_impl::manager_impl()
{
}

bool
manager_impl::loadConfig( adportable::Configuration& config, const QString& filename, const wchar_t * query )
{
    return ConfigLoader::loadConfiguration( config, qtwrapper::wstring( filename ), query );
}

/*
const adportable::Configuration *
manager_impl::getConfiguration( const wchar_t * name )
{
	using namespace adportable;

	for ( Configuration::vector_type::iterator it = rootConfig_.begin(); it != rootConfig_.end(); ++it ) {
		if ( name == it->name() )
			return &(*it);
	}
	return 0;
}

const adportable::Component *
manager_impl::findComponent( const wchar_t * name )
{
	static adportable::Component config;
    return &config;
}
*/
