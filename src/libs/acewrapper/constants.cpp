//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "constants.h"
#include <orbsvcs/CosNamingC.h>

namespace acewrapper {
    namespace constants {
        namespace adcontroller {

            CosNaming::Name
                manager::name() {
                    CosNaming::Name name;
                    name.length(1);
                    name[0].id = CORBA::string_dup( "adcontroller.manager" );
                    name[0].kind = CORBA::string_dup( "" );
                    return name;
            }

        }

        namespace adbroker {

            CosNaming::Name
                manager::name() {
                    CosNaming::Name name;
                    name.length(1);
                    name[0].id = CORBA::string_dup( "adbroker.manager" );
                    name[0].kind = CORBA::string_dup( "" );
                    return name;
            }

        }

    }
}

