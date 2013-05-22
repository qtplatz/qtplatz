// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Copyright (C) MS-Cheminformatics LLC / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef TOFCONTROLLER_H
#define TOFCONTROLLER_H

//#include "tofcontroller_global.h"
#include <adplugin/orbLoader.h>

class SHARED_EXPORT TofController : public adplugin::orbLoader {
public:
    virtual ~TofController();
    TofController();

	virtual operator bool () const;
	virtual bool initialize( CORBA::ORB *, PortableServer::POA * , PortableServer::POAManager * );
    virtual void initial_reference( const char * ior_broker_manager );
	virtual const char * activate();
	virtual bool deactivate();
};

extern "C" {
	__declspec(dllexport) adplugin::orbLoader * instance();
}

#endif // TOFCONTROLLER_H
