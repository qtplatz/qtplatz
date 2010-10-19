// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <string>

namespace CORBA {
   class ORB;
   class Object;
}

namespace Broker {
	class Manager;
}

namespace acewrapper {

	class nameresolver {
	public:
		nameresolver(void);
		~nameresolver(void);

		static Broker::Manager * getManager( CORBA::ORB * orb, const std::string& ior );
		static std::string ior( Broker::Manager *, const char * name );
		// static CORBA::Object * string_to_object( const std::string& ior );

	private:
		// static bool find_ior( const std::string& name, std::string& ior );
	};
};
