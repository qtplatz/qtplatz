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

#include "adplugin_global.h"

namespace CORBA {
    class ORB;
}

namespace PortableServer {
	class POA;
	class POAManager;
}

namespace adplugin {

    class ADPLUGINSHARED_EXPORT orbLoader {
    public:
	virtual ~orbLoader() {};
	virtual operator bool() const = 0;
	
	virtual bool initialize( CORBA::ORB *, PortableServer::POA * , PortableServer::POAManager * ) = 0;
        virtual void initial_reference( const char * ior ) = 0;
        virtual const char * activate() = 0;
        virtual bool deactivate() = 0;
	
	virtual const char * error_description() { return 0; }
    };

}
