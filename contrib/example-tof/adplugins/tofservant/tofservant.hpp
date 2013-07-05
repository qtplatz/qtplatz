// This is a -*- C++ -*- header.
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

#pragma once

#include "tofservant_global.h"
#include <adplugin/orbservant.hpp>
#include <adplugin/plugin.hpp>
#include <adplugin/orbfactory.hpp>
#include <memory>

namespace acewrapper { template<class T> class ORBServant; }

namespace tofservant {

	class tofmgr_i;

    class TOFSERVANTSHARED_EXPORT tofServantPlugin : public adplugin::plugin
                                                   , public adplugin::orbFactory
                                                   , public adplugin::orbServant {
        static tofServantPlugin * instance_;
        tofServantPlugin();
        virtual ~tofServantPlugin();
        std::unique_ptr< acewrapper::ORBServant< tofmgr_i > > tofMgr_;

    public:
        static tofServantPlugin * instance();
        PortableServer::POA* poa();
        
        int run();
        void abort_server();

        // adplugin::orbServant
        bool initialize( CORBA::ORB *, PortableServer::POA * , PortableServer::POAManager * ) override;
        const char * activate() override;
        bool deactivate() override;
        const char * object_name() const override;
        // orbFactory
        adplugin::orbServant * create_instance() override { return this; }

        // plugin
        const char * iid() const override;
        void accept( adplugin::visitor&, const char * ) override;
        void * query_interface_workaround( const char * typenam ) override;
    };

}

extern "C" {
    DECL_EXPORT adplugin::plugin * adplugin_plugin_instance();
}
