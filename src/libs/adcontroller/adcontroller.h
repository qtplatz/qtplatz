// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/

#pragma once

#include "adcontroller_global.h"

#include <adplugin/orbLoader.h>

namespace CORBA {
    class ORB;
}

namespace acewrapper {
    class ORBServantManager;
}

class ADCONTROLLERSHARED_EXPORT adController : public adplugin::orbLoader {
public:
    adController();
    virtual ~adController();

    // impriment adplugin::orbLoader
	virtual operator bool() const;
	// virtual bool initialize( CORBA::ORB * orb = 0 );
	virtual bool initialize( CORBA::ORB *, PortableServer::POA * , PortableServer::POAManager * );
    virtual void initial_reference( const char * iorBroker );
	virtual const char * activate();
	virtual bool deactivate();
	virtual int run();
	virtual void abort_server();

	// adController
	static void _dispose();
	static bool _deactivate();
	static void _abort_server();
private:

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
