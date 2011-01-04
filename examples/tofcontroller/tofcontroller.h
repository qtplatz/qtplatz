// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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
// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
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
