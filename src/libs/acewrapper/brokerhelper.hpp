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

#include <string>

namespace CORBA {
   class ORB;
   class Object;
}

namespace Broker {
    class Manager;
}

namespace acewrapper {
    
    class brokerhelper {
    public:
        brokerhelper(void);
        ~brokerhelper(void);
        
        static Broker::Manager * getManager( CORBA::ORB * orb, const std::string& iorBroker );
        static std::string ior( Broker::Manager *, const char * name );
        static CORBA::Object * name_to_object( CORBA::ORB * orb, const std::string& name, const std::string& iorBroker );
        
    private:
        // static bool find_ior( const std::string& name, std::string& ior );
    };
};
