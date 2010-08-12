// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef TOFCONTROLLER_H
#define TOFCONTROLLER_H

// #include "tofcontroller_global.h"
#include <adplugin/orbLoader.h>

namespace CORBA {
    class ORB;
}

class SHARED_EXPORT TofController : public adplugin::orbLoader {
protected:
    virtual ~TofController();
public:
    TofController();

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
*/
};

extern "C" {
    SHARED_EXPORT adplugin::orbLoader * instance();
}

#endif // TOFCONTROLLER_H
