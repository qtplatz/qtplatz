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

class ADBROKERSHARED_EXPORT adbroker {
public:
    adbroker(void);
    ~adbroker(void);

	static int run( int argc, char * argv[] );
	static void abort_server();

    static CORBA::ORB * orb();
    static const char * ior();
};
