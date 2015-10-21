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

#include "adcontroller_global.h"
#include <adplugin/orbservant.hpp>
#include <adplugin/plugin.hpp>

namespace adplugin { class plugin; }

namespace CORBA {
    class ORB;
    class Object;
}

class ADCONTROLLERSHARED_EXPORT adController : public adplugin::orbServant 
                                             , public adplugin::plugin {
public:
    adController();
    virtual ~adController();

    // adplugin::orbServant
	bool initialize( CORBA::ORB *, PortableServer::POA * , PortableServer::POAManager * ) override;
	const char * activate() override;
	bool deactivate() override;
	const char * object_name() const override;
    CORBA::Object * _this() const override;

	int run();
	void abort_server();

	// adController
	static void _dispose();
	static bool _deactivate();
	static void _abort_server();

    // plugin
    const char * iid() const override { return "com.ms-cheminfo.lib.qtplatz.plugins.adcontroller"; }
    void accept( adplugin::visitor&, const char * ) override { /* do nothing */ }
private:

};

extern "C" {
    DECL_EXPORT adplugin::plugin * adplugin_plugin_instance();
}
