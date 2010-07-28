// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include <boost/utility.hpp>
#include <tao/Utils/ORB_Manager.h>

template<class T>
class orbserver : boost::noncopyable {

	TAO_ORB_Manager orb_manager_;
	std::string id_;
	T impl_;
	bool isactive_;

 public:
	 static void * thread_entry( void * me ) {
		 orbserver<T> * pThis = reinterpret_cast< orbserver<T> * >(me);
		 if ( pThis )
			 pThis->run();
		 return 0;
	 }
  
public:
	orbserver() : isactive_(false) {}

	int init(int ac, ACE_TCHAR * av[]) {
		return orb_manager_.init(ac, av);
	}

	int fini() { return orb_manager_.fini(); }
	CORBA::ORB_ptr orb() { return orb_manager_.orb(); }
	operator T* () { return &impl_; }
  
	const std::string& activate() {
		isactive_ = true;
		CORBA::String_var str = orb_manager_.activate(&impl_);
		id_ = str.in();
		return id_;
	}
  
	void deactivate( bool fin = true ) {
		orb_manager_.deactivate( id_.c_str() );
        if ( fin )
			orb_manager_.fini();
	}
  
	int run() {
		return orb_manager_.run();	
	}
};
