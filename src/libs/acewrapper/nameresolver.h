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

namespace acewrapper {

	class nameresolver {
	public:
		nameresolver(void);
		~nameresolver(void);

		static bool register_name_service( const std::string& name, const std::string& ior );
		static bool unregister_name_service( const std::string& name );
		static CORBA::Object * resolve_name( CORBA::ORB * orb, const std::string& name );
	private:
		static bool find_ior( const std::string& name, std::string& ior );
	};
};
