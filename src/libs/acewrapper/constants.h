// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

namespace CosNaming {
	class Name;
}

namespace acewrapper {

    namespace constants {

        namespace adcontroller {
            struct session {
                static CosNaming::Name name();
            };
        }

        namespace adbroker {
            struct manager {
                static CosNaming::Name name();
            };
        }
    }

}
