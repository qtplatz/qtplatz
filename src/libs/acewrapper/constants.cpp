// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include "constants.hpp"

namespace acewrapper {
    namespace constants {
        namespace adcontroller {
#if 0
            CosNaming::Name
                manager::name() {
                    CosNaming::Name name;
                    name.length(1);
                    name[0].id = CORBA::string_dup( "adcontroller.manager" );
                    name[0].kind = CORBA::string_dup( "" );
                    return name;
            }
#endif
			const char * manager::_name() { return "adcontroller.manager"; }
        }

        namespace adbroker {
#if 0
            CosNaming::Name
                manager::name() {
                    CosNaming::Name name;
                    name.length(1);
                    name[0].id = CORBA::string_dup( "adbroker.manager" );
                    name[0].kind = CORBA::string_dup( "" );
                    return name;
            }
#endif
			const char * manager::_name() { return "adbroker.manager"; }
        }
    }
}

