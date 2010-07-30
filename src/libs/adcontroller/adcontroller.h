// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "adcontroller_global.h"

namespace CosNaming {
	class Name;
}

class ADCONTROLLERSHARED_EXPORT adcontroller {
public:
    adcontroller();
	static int run( int argc, char * argv[] );
	static void abort_server();
	static CosNaming::Name name();
};

