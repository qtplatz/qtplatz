// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#pragma warning (disable: 4996)
# include <tao/Object.h>
# include <orbsvcs/CosNamingC.h>
#pragma warning (default: 4996)

namespace acewrapper {

	class NS {
		typedef CosNaming::NamingContext CTX;
	public:
		static CosNaming::NamingContext_ptr resolve_init( CORBA::ORB_ptr orb );
		// static bool bind( CTX::_ptr_type nc, const CosNaming::Name& name, CORBA::Object_ptr );
		static CORBA::Object_ptr resolve_name( CTX::_ptr_type nc, const CosNaming::Name& name );

		static CORBA::Object_ptr resolve_name( CORBA::ORB_ptr orb, const std::wstring& name );

		static bool register_name_service( CORBA::ORB_ptr orb, const CosNaming::Name& name, CORBA::Object_ptr obj );
		static bool unregister_name_service( CORBA::ORB_ptr orb, const CosNaming::Name& name );
   
	};


}
