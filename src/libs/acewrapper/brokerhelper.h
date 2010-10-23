// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

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
