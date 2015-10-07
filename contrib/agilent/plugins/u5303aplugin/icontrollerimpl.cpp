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

#include "icontrollerimpl.hpp"
#include <adplugin/plugin.hpp>
#include <adplugin_manager/loader.hpp>
#include <adicontroller/manager.hpp>

using namespace u5303a;

iControllerImpl::iControllerImpl() : adextension::iControllerImpl("u5303a")
{
}

iControllerImpl::~iControllerImpl()
{
}

bool
iControllerImpl::connect()
{
    if ( adplugin::plugin * plugin = adplugin::loader::loadLibrary( "u5303a", QStringList() ) ) {

        if ( auto manager = plugin->query_interface< adicontroller::manager >() ) {
            if ( auto session = manager->session( "u5303a::icontrollerimpl" ) ) {
                adextension::iControllerImpl::connect( session, "u5303a::iControllerImpl" );
                return true;
            }
        }

    }
    
    return false;
}

