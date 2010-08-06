// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "adplugin.h"
#include <QMutex>
#include <stdlib.h>

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
			bool loadConfig( const wchar_t * );
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
manager_impl::loadConfig( const wchar_t * filename )
{
    Q_UNUSED( filename );
	return false;
}