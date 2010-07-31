// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef ORBSERVANT_H
#define ORBSERVANT_H

#include <boost/noncopyable.hpp>
#include <tao/Utils/ORB_Manager.h>
#include <string>

namespace acewrapper {

  template<class T> class ORBServant : boost::noncopyable {
  public:
	  ORBServant() {}

	  int init( int ac, ACE_TCHAR * av[] ) { return orbmgr_.init( ac, av ); }
	  int fini() { return orbmgr_.fini(); }
	  inline CORBA::ORB_ptr orb() { return orbmgr_.orb(); }
	  inline operator T* () { return &impl_; }
	  inline operator typename T::_stub_ptr_type () { return impl_._this(); }
	  inline void activate() { id_ = orbmgr_.activate( &impl_ ); }
	  void deactivate() { orbmgr_.deactivate( id_.c_str() ); }
      void run() { orbmgr_.run(); }
      inline const std::string& ior() const { return id_; }

	  static void * thread_entry( void * me ) {
		  ORBServant<T> * pThis = reinterpret_cast< ORBServant<T> * >(me);
		  if ( pThis )
			  pThis->orbmgr_.run();
		  return 0;
	  }

  private:
	  std::string id_;
	  T impl_;
      TAO_ORB_Manager orbmgr_;
  };

}

#endif // ORBSERVANT_H
