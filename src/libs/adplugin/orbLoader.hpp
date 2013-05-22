// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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
#include <string>

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
        static std::wstring library_fullpath( const std::wstring& appdir, const std::wstring& library_filename );
        static std::wstring config_fullpath( const std::wstring& appdir, const std::wstring& config_filename );
    };

}
