// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef TOFCONTROLLER_H
#define TOFCONTROLLER_H

//#include "tofcontroller_global.h"
#include <adplugin/orbLoader.h>

namespace CORBA {
    class ORB;
}

class SHARED_EXPORT TofController : public adplugin::orbLoader {
public:
    virtual ~TofController();
    TofController();

	virtual operator bool () const;
    virtual bool initialize( CORBA::ORB * orb = 0 );
    virtual void initial_reference( const char * ior_broker_manager );
	virtual const char * activate();
	virtual bool deactivate();
	virtual int run();

	void abort_server();
    void dispose();
};

extern "C" {
	__declspec(dllexport) adplugin::orbLoader * instance();
/*
	__declspec(dllexport) bool initialize( CORBA::ORB * orb = 0 );
	__declspec(dllexport) void initial_reference( const char * iorBroker );
	__declspec(dllexport) const char * activate();
	__declspec(dllexport) bool deactivate();
	__declspec(dllexport) int run();
	__declspec(dllexport) void abort_server();
*/
}

#endif // TOFCONTROLLER_H
