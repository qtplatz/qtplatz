// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2014 MS-Cheminformatics LLC
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
#include "adbroker_global.h"
#include <adplugin/orbservant.hpp>
#include <adplugin/plugin.hpp>
#include <adplugin/orbfactory.hpp>

namespace CORBA {
    class ORB;
	class Object;
}

namespace PortableServer {
    class POA;
    class POAManager;
}

class ADBROKERSHARED_EXPORT adBroker : public adplugin::orbServant
                                     , public adplugin::plugin {
public:
    adBroker(void);
    virtual ~adBroker(void);

    // orbServant
    bool initialize( CORBA::ORB* orb, PortableServer::POA * poa, PortableServer::POAManager * mgr ) override;
    const char * activate() override;
    bool deactivate() override;
    //void initial_reference( const char * ) override;
	const char * object_name() const override;
	CORBA::Object * _this() const override;

    // plugin
    const char * iid() const { return "com.ms-cheminfo.lib.qtplatz.plugins.adborker"; }
    void accept( adplugin::visitor&, const char * ) { /* do nothing */ }
    virtual void * query_interface_workaround( const char * typenam );
};

extern "C" {
    Q_DECL_EXPORT adplugin::plugin * adplugin_plugin_instance();
}
