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

#include <map>
#include <string>
#include "adinterface/brokerS.h"

#include "logger_i.hpp"
#include "session_i.hpp"
#include "objectdiscovery.hpp"
#include <mutex>
#include <memory>

namespace acewrapper { template<class T> class ORBServant; }

namespace adbroker {

    namespace internal { struct object_receiver; }

    class manager_i : public virtual POA_Broker::Manager {
        manager_i(void);
        ~manager_i(void);
        friend class acewrapper::ORBServant< adbroker::manager_i >;
        static acewrapper::ORBServant< manager_i > * instance_;
        static std::mutex mutex_;
    public:
        static acewrapper::ORBServant< manager_i > * instance();

        void register_ior( const char * name, const char * ior );
        char * ior( const char * name );

        void register_lookup( const char * name, const char * ident );

        bool register_object( const char * name, CORBA::Object_ptr obj );
        CORBA::Object_ptr find_object( const char * name );
		Broker::Objects * find_objects( const char * name );

        bool register_handler( Broker::ObjectReceiver_ptr cb );
        bool unregister_handler( Broker::ObjectReceiver_ptr cb );

        Broker::Session_ptr getSession( const CORBA::WChar * );
        Broker::Logger_ptr getLogger();

        void shutdown();

        void internal_register_ior( const std::string& name, const std::string& ior );
    private:
        typedef std::map< std::wstring, std::shared_ptr< adbroker::session_i > > session_map_type;

        session_map_type session_list_;
        std::unique_ptr< broker::logger_i > logger_i_;
        std::map< std::string, std::string > iorMap_;
        std::map< std::string, std::string > lookup_;
        std::map< std::string, CORBA::Object_var > objVec_;

        std::vector< internal::object_receiver > sink_vec_;
        ObjectDiscovery * discovery_;
    };

}


