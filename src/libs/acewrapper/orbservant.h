// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef ORBSERVANT_H
#define ORBSERVANT_H

#pragma warning ( disable: 4996 )
#include <tao/ORB.h>
#include <tao/PortableServer/PortableServer.h>
#pragma warning ( default: 4996 )
#include <string>

class TAO_ORB_Manager;

namespace acewrapper {

	template<class T> class ORBServant {
	public:
		~ORBServant() { 
		}

		ORBServant( ) {
		}

		ORBServant( CORBA::ORB_ptr orb, PortableServer::POA_ptr poa, PortableServer::POAManager_ptr mgr ) 
			: orb_(orb), root_poa_(poa), poa_manager_(mgr) {
		}

		void initialize( CORBA::ORB_ptr orb, PortableServer::POA_ptr poa, PortableServer::POAManager_ptr mgr ) {
			orb_ = CORBA::ORB::_duplicate(orb);
			poa_ = PortableServer::POA::_duplicate(poa);
			poa_manager_ = PortableServer::POAManager::_duplicate(mgr);
		}

		inline void activate() {
			PortableServer::ObjectId_var id = this->poa_->activate_object ( &impl_ );
			CORBA::Object_var obj = this->poa_->id_to_reference (id.in ());
			CORBA::String_var str = this->orb_->object_to_string (obj.in ());
            id_ = str;
        }

		void deactivate() { 
			CORBA::Object_var object = this->orb_->string_to_object ( id_.c_str() );
			PortableServer::ObjectId_var object_id = this->poa_->reference_to_id (object.in ());
			this->poa_->deactivate_object ( object_id.in () );
		}

		inline operator T* () { return &impl_; }

		inline CORBA::ORB_ptr orb() { return CORBA::ORB::_duplicate( orb_.in() ); }
		inline PortableServer::POA_ptr poa() { return PortableServer::POA::_duplicate( poa_.in() ); }

		inline operator typename T::_stub_ptr_type () { return impl_._this(); }
		inline const std::string& ior() const { return id_; }

        inline void broker_manager_ior( const std::string& ior ) { ior_broker_manager_ = ior; }
        inline const char * broker_manager_ior() const { return ior_broker_manager_.c_str(); }

	private:
		CORBA::ORB_var orb_;
		PortableServer::POA_var poa_;
		PortableServer::POAManager_var poa_manager_;
		std::string id_;
        std::string ior_broker_manager_;
		T impl_;
	};

}


#endif // ORBSERVANT_H
