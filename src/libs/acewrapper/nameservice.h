#pragma once

#include <tao/Object.h>
#include <orbsvcs/CosNamingC.h>

namespace acewrapper {

	class NS {
		typedef CosNaming::NamingContext CTX;
	public:
		static CosNaming::NamingContext_ptr resolve_init( CORBA::ORB_ptr orb );
		static bool bind( CTX::_ptr_type nc, const CosNaming::Name& name, CORBA::Object_ptr );
		static CORBA::Object_ptr resolve_name( CTX::_ptr_type nc, const CosNaming::Name& name );

		static bool register_name_service( CORBA::ORB_ptr orb, const CosNaming::Name& name, CORBA::Object_ptr obj );
   
		template<class T> static typename T::_ptr_type
			resolve_name_t( CTX::_ptr_type nc, const CosNaming::Name& name ) {
				CORBA::Object_var obj = resolve_name( nc, name );
				typename T::_var_type ref;
				try {
					ref = T::_narrow( obj.in() );
				} catch ( const CORBA::Exception& e ) {
					throw e;
				}
				return ref._retn();
		}

	};


}
