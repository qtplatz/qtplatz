// This is a -*- C++ -*- header.
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#pragma once
#include "adbroker_global.h"

namespace CORBA {
    class ORB;
}

namespace PortableServer {
	class POA;
	class POAManager;
}

namespace acewrapper {
	class ORBServantManager;
}

class ADBROKERSHARED_EXPORT adBroker {
public:
    adBroker(void);
    ~adBroker(void);

	static bool initialize( CORBA::ORB* orb, PortableServer::POA * poa, PortableServer::POAManager * mgr );
	static const char * activate();
    static bool deactivate();

	static int run();
	static void abort_server();
};
