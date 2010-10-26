//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "MassSpectrometerBroker.h"
#include <ace/Singleton.h>
#include <ace/Recursive_Thread_Mutex.h>
#include <map>
#include <string>

using namespace adcontrols;

namespace adcontrols {

	class MassSpectrometerBrokerImpl;

	//-------------------------
	namespace singleton {
		typedef ACE_Singleton<MassSpectrometerBrokerImpl, ACE_Recursive_Thread_Mutex> MassSpectrometerBrokerImpl;
	}

	//-------------------------
	class MassSpectrometerBrokerImpl : public MassSpectrometerBroker {
	public:
		~MassSpectrometerBrokerImpl() {}

		bool register_factory( factory_type factory, const std::wstring& name ) {
			factories_[name] = factory;
			return true;
		}

		factory_type find( const std::wstring& name ) {
			std::map< std::wstring, factory_type >::iterator it = factories_.find( name );
			if ( it != factories_.end() )
				return it->second;
			return 0;
		}

	private:
		std::map< std::wstring, factory_type > factories_;
	};

	typedef ACE_Singleton< MassSpectrometerBrokerImpl, ACE_Recursive_Thread_Mutex > impl_type;
}

/////////////////////////////////////////////

MassSpectrometerBroker::MassSpectrometerBroker(void)
{
}

MassSpectrometerBroker::~MassSpectrometerBroker(void)
{
}

bool
MassSpectrometerBroker::register_factory( factory_type factory, const std::wstring& name )
{
	return singleton::MassSpectrometerBrokerImpl::instance()->register_factory( factory, name );
}

MassSpectrometerBroker::factory_type
MassSpectrometerBroker::find( const std::wstring& name )
{
	return singleton::MassSpectrometerBrokerImpl::instance()->find( name );
}