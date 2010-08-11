// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "adcontroller_global.h"

namespace CORBA {
    class ORB;
}

namespace acewrapper {
	class ORBServantManager;
}

class ADCONTROLLERSHARED_EXPORT adController {
public:
    adController();

	static bool initialize( CORBA::ORB * orb = 0 );
	static bool activate();
    static bool deactivate();
	static int run();
	static void abort_server();
private:

};

