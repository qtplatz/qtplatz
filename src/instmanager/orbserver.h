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
 public:
  
  static void * thread_entry( void * me ) {
    orbserver<T> * pThis = reinterpret_cast< orbserver<T> * >(me);
    if ( pThis )
      pThis->run();
    return 0;
  }
  
public:
  int init(int ac, ACE_TCHAR * av[]) {
    return orb_manager_.init(ac, av);
  }
  
  const std::string& activate() {
    CORBA::String_var str = orb_manager_.activate(&impl_);
    id_ = str.in();
    return id_;
  }
  
  void deactivate() {
    orb_manager_.deactivate( id_.c_str() );
    orb_manager_.fini();
  }
  
  int run() {
    return orb_manager_.run();	
  }
};
