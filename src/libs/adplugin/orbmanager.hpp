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

# include <tao/ORB.h>
# include <tao/PortableServer/PortableServer.h>
# include <ace/Recursive_Thread_Mutex.h>

#include <string>

template<class T, class M> class ACE_Singleton;

class TAO_ORB_Manager;

namespace adplugin {

    class ADPLUGINSHARED_EXPORT ORBManager {
	~ORBManager();
	ORBManager();
	
    public:
	// int init( int argc, char * argv[] );
        void initialize( CORBA::ORB_ptr orb, PortableServer::POA_ptr poa );
	
	CORBA::ORB_ptr orb();
        PortableServer::POA_ptr poa();
	CORBA::Object_ptr string_to_object( const std::string& ior );
        bool deactivate( CORBA::Object_ptr );
        bool deactivate( PortableServer::ServantBase * );
	
        static ORBManager * instance();
	
    private:
	CORBA::ORB_var orb_;
        PortableServer::POA_var poa_;
	
	ACE_Recursive_Thread_Mutex mutex_;
        friend class ACE_Singleton< ORBManager, ACE_Recursive_Thread_Mutex >;
    };
    
}

