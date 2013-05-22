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
#include "adbroker_global.h"
#include <adplugin/orbLoader.hpp>

namespace CORBA {
    class ORB;
}

namespace PortableServer {
    class POA;
    class POAManager;
}

namespace acewrapper {
    class ORBServantManager;
}

class ADBROKERSHARED_EXPORT adBroker : public adplugin::orbLoader {
public:
    adBroker(void);
    virtual ~adBroker(void);
    
    virtual bool initialize( CORBA::ORB* orb, PortableServer::POA * poa, PortableServer::POAManager * mgr );
    virtual const char * activate();
    virtual bool deactivate();
    virtual void initial_reference( const char * );
    virtual operator bool() const;
};

extern "C" {
    Q_DECL_EXPORT adplugin::orbLoader * instance();
}
