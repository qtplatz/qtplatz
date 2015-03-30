// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
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

#include <adorbmgr/adorbmgr_global.h>
#include <adportable/asio/thread.hpp>
#include <atomic>
#include <mutex>
#include <thread>
#include <condition_variable>

namespace CORBA { class ORB; class Object; }
class TAO_ServantBase;
namespace PortableServer {
    class POA;
    class POAManager;
    typedef TAO_ServantBase ServantBase;
    typedef ServantBase *Servant;
}

namespace adorbmgr {

    class ADORBMGRSHARED_EXPORT orbmgr {
        ~orbmgr();
        orbmgr( const orbmgr& ); // noncopyable
        orbmgr( CORBA::ORB * orb = 0
                , PortableServer::POA * poa = 0
                , PortableServer::POAManager * mgr = 0 );
    public:
        int init( int ac, char * av[] );
		bool spawn();
        void shutdown();
        bool fini();
        bool wait();
        
        CORBA::ORB * orb();
        PortableServer::POA* root_poa();
        PortableServer::POA* child_poa();
        PortableServer::POAManager* poa_manager();

        static std::string activate( PortableServer::Servant );
        static void deactivate( const std::string& id );
        static void deactivate( CORBA::Object * );
        static void deactivate( PortableServer::ServantBase * );
        
        static orbmgr * instance();

    private:
        class impl;
        impl * impl_;
    };
}

