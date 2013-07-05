// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2013 MS-Cheminformatics LLC
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
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

#include "adinterface/brokerC.h"
#include "adinterface/brokerclientS.h"

#include <vector>
#include <map>

namespace Broker { class Manager; }
namespace acewrapper { template<class T> class ORBServant; }

namespace tofservant {

    class tofServantPlugin;
	class tofSession_i;

    class tofmgr_i : public POA_BrokerClient::Accessor {
        tofmgr_i();
        ~tofmgr_i();
        friend class acewrapper::ORBServant< tofmgr_i >;

    public:
        bool setBrokerManager( Broker::Manager_ptr mgr );
        bool adpluginspec( const char * id, const char * spec );
        acewrapper::ORBServant< tofSession_i > * tofSession() { return tofSession_.get(); }
	private:
        std::unique_ptr< acewrapper::ORBServant< tofSession_i > > tofSession_;
		Broker::Manager_var broker_mgr_;
		std::string adplugin_id_;
		std::string adplugin_spec_;
        std::string configXml_;
    };
    
}
