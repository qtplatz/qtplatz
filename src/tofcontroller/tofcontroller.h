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

	operator bool () const;
    bool initialize( CORBA::ORB * orb = 0 );
	bool activate();
    bool deactivate();
	int run();
	void abort_server();
    void dispose();
/*
	static bool initialize( CORBA::ORB * orb = 0 );
	static bool activate();
    static bool deactivate();
	static int run();
	static void abort_server();
/**/
};

extern "C" {
	__declspec(dllexport) adplugin::orbLoader * instance();
	__declspec(dllexport) bool initialize( CORBA::ORB * orb = 0 );
	__declspec(dllexport) bool activate();
	__declspec(dllexport) bool deactivate();
	__declspec(dllexport) int run();
	__declspec(dllexport) void abort_server();
}

#endif // TOFCONTROLLER_H
