/**************************************************************************
** Copyright (C) 2014-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2014-2015 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#include "iu5303afacade.hpp"
#include <adplugin/plugin.hpp>
#include <adplugin_manager/loader.hpp>
#include <adacquire/manager.hpp>

namespace accutof { namespace acquire {
        const QString iU5303AFacade::__module_name__ = "AccuTOF/U5303A";
    }
}

using namespace accutof::acquire;

iU5303AFacade::iU5303AFacade() : adextension::iControllerImpl("accutof")
{
}

iU5303AFacade::~iU5303AFacade()
{
}

bool
iU5303AFacade::connect()
{
    if ( adplugin::plugin * plugin = adplugin::loader::loadLibrary( "u5303a" ) ) {

        if ( auto manager = plugin->query_interface< adacquire::manager >() ) {
            if ( auto session = manager->session( "accutof" ) ) {
                adextension::iControllerImpl::connect( session, "accutof" );
                return true;
            }
        }

    }

    return false;
}
