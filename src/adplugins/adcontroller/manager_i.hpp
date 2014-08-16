// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#include <compiler/diagnostic_push.h>
#include <compiler/disable_deprecated.h>
#include <compiler/disable_4250.h> // inherits via dominance

#include <adinterface/brokerS.h>
#include <adinterface/brokerclientS.h>

#include <compiler/diagnostic_pop.h>

#include "session_i.hpp"
#include <map>
#include <string>
#include <mutex>

namespace acewrapper { template<class T> class ORBServant; }

namespace adcontroller {

    class manager_i : public virtual POA_ControlServer::Manager
                    , public virtual POA_BrokerClient::Accessor {
        manager_i(void);
        ~manager_i(void);
        friend class acewrapper::ORBServant< ::adcontroller::manager_i >;
        static acewrapper::ORBServant< manager_i > * instance_;
        static std::mutex mutex_;
    public:
        static acewrapper::ORBServant< manager_i > * instance();

        void shutdown();
		Broker::Manager_ptr getBrokerManager();

        // ControlServer::Manager
        ControlServer::Session_ptr getSession( const CORBA::WChar * ) override;
        Broker::Logger_ptr getLogger();

        // BrokerClient::Accessor
        bool setBrokerManager( Broker::Manager_ptr mgr ) override;
        bool adpluginspec( const char * id, const char * adpluginspec ) override;
        
    private:
        typedef std::map< std::wstring, std::shared_ptr< adcontroller::session_i > > session_map_type;
        session_map_type session_list_;
        Broker::Manager_var broker_mgr_;
        Broker::Logger_var logger_;
        std::string adplugin_id;
        std::string adplugin_spec;
    };

}
