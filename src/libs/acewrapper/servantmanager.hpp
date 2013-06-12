/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
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

#ifndef SERVANTMANAGER_HPP
#define SERVANTMANAGER_HPP

#include <tao/ORB.h>
#include <tao/PortableServer/PortableServer.h>
#include <boost/noncopyable.hpp>
#include <string>
#include <mutex>

class TAO_ORB_Manager;

namespace acewrapper {

    class ServantManager : boost::noncopyable {
	~ServantManager();
	ServantManager( CORBA::ORB_ptr orb = 0
			, PortableServer::POA_ptr = 0
			, PortableServer::POAManager_ptr = 0 );
    public:
	int init( int ac, ACE_TCHAR * av [] );
	int fini();

        CORBA::ORB_ptr orb();
        PortableServer::POA_ptr root_poa();
        PortableServer::POA_ptr child_poa();
        PortableServer::POAManager_ptr poa_manager();
	
        std::string activate( PortableServer::Servant );
        void deactivate( const std::string& id );
	
        bool spawn();
        void shutdown();
	static ServantManager * instance();
	
    private:
	void run();
        static void * thread_entry( void * me );
	
        size_t init_count_;
        bool thread_running_;
        std::mutex mutex_;
        TAO_ORB_Manager * orbmgr_;
        ACE_thread_t threadid_;
        friend class ACE_Singleton< ServantManager, ACE_Recursive_Thread_Mutex >;
    };

    namespace singleton {
	typedef ACE_Singleton< ServantManager, ACE_Recursive_Thread_Mutex > ServantManager;
    }
}

#endif // SERVANTMANAGER_HPP
