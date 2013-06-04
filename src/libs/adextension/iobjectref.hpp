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

#include <QObject>
#include "adextension_global.hpp"

namespace CORBA { class ORB; class Object; }
namespace Broker { class Manager; }
class TAO_ServantBase;
namespace PortableServer { class POA; typedef TAO_ServantBase ServantBase; }

namespace adextension {

    // this object may be instanciated in ServantPlugin
    class ADEXTENSIONSHARED_EXPORT iObjectRef : public QObject {
        Q_OBJECT
    public:
        virtual ~iObjectRef();
        iObjectRef();

		virtual PortableServer::POA * poa() = 0;
		virtual CORBA::ORB * orb() = 0;
        virtual Broker::Manager * getBrokerManager() = 0;
        virtual bool deactivate( CORBA::Object * ) = 0;
        virtual bool deactivate( PortableServer::ServantBase * ) = 0;
    };

}
