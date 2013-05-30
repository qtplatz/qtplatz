// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#include <tao/ORB.h>
#include <tao/PortableServer/PortableServer.h>
#include <string>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>

#pragma once

class TAO_ORB_Manager;

namespace servant {

    class ORBServantManager : boost::noncopyable {
        ORBServantManager( const ORBServantManager& ); // noncopyable
        ~ORBServantManager();
        ORBServantManager( CORBA::ORB_ptr orb = 0
                           , PortableServer::POA_ptr = 0
                           , PortableServer::POAManager_ptr = 0);
    public:
        int init( int ac, ACE_TCHAR * av[] );
		bool spawn( boost::barrier& );
        void shutdown();
        bool fini();
        bool wait();
		boost::mutex& mutex();
        
        CORBA::ORB_ptr orb();
        PortableServer::POA_ptr root_poa();
        PortableServer::POA_ptr child_poa();
        PortableServer::POAManager_ptr poa_manager();
        
        std::string activate( PortableServer::Servant );
        void deactivate( const std::string& id );
        
        
        static ORBServantManager * instance();

    private:

		void run( boost::barrier& );

        static ORBServantManager * instance_;
        boost::mutex mutex_;
        size_t init_count_;
        bool thread_running_;
        boost::thread * thread_;
        TAO_ORB_Manager * orbmgr_;
    };

}
