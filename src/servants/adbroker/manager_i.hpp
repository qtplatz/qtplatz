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

#pragma once

#include <acewrapper/orbservant.hpp>
#include <map>
#include <string>
#include "adinterface/brokerS.h"

#include "logger_i.hpp"
#include "session_i.hpp"
#include "objectdiscovery.hpp"
#include <boost/smart_ptr.hpp>
#include <ace/Recursive_Thread_Mutex.h>

namespace adbroker {

    namespace internal { struct object_receiver; }

    class manager_i : public virtual POA_Broker::Manager {

    public:
        manager_i(void);
        ~manager_i(void);

        void shutdown();
        bool register_handler( Broker::ObjectReceiver_ptr cb );
        bool unregister_handler( Broker::ObjectReceiver_ptr cb );
        Broker::Session_ptr getSession( const CORBA::WChar * );
        Broker::Logger_ptr getLogger();
        void register_ior( const char * name, const char * ior );
        char * ior( const char * name );
        void register_lookup( const char * name, const char * ident );
        inline ACE_Recursive_Thread_Mutex& mutex() { return mutex_; }
        static manager_i * instance();
        void internal_register_ior( const std::string& name, const std::string& ior );
    private:
        typedef std::map< std::wstring, boost::shared_ptr< adbroker::session_i > > session_map_type;
        session_map_type session_list_;
        boost::scoped_ptr< broker::logger_i > logger_i_;
        std::map< std::string, std::string > iorMap_;
        std::map< std::string, std::string > lookup_;
        std::vector< internal::object_receiver > sink_vec_;
        ObjectDiscovery * discovery_;
        ACE_Recursive_Thread_Mutex mutex_;
    };

    namespace singleton {
        typedef ACE_Singleton< acewrapper::ORBServant< manager_i >, ACE_Recursive_Thread_Mutex > manager;
    }

}


