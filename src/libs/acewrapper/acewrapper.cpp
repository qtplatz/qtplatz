#include "acewrapper.h"
#include "mutex.hpp"
#include <ace/Mutex.h>
#include <ace/Init_ACE.h>
#include <ace/OS_NS_unistd.h>
#include <ace/Object_Manager.h>

using namespace acewrapper;

instance_manager * instance_manager::instance_ = 0;

instance_manager::instance_manager()
{
  ACE::init();
}

instance_manager::~instance_manager()
{
  ACE::fini();
}

void
instance_manager::initialize()
{
  if ( instance_manager::instance_ == 0 ) {
	ACE_Recursive_Thread_Mutex *plock;
	if ( ACE_Object_Manager::get_singleton_lock(plock) == (-1) )
          throw std::exception("acewrapper: lock object couldn't acquired");
	scoped_mutex_t<> lock(*plock);
	instance_ = new acewrapper::instance_manager();
  }
}

void
instance_manager::dispose()
{
  delete instance_;
}

namespace acewrapper {
  class instance_disposer {
  public:
	~instance_disposer() {
	  instance_manager::dispose();
	}
  };
}

instance_disposer __disposer;

