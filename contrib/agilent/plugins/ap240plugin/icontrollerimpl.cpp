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
#include <adacquire/manager.hpp>

using namespace ap240;

iControllerImpl::iControllerImpl() : adextension::iControllerImpl("ap240")
{
}

iControllerImpl::~iControllerImpl()
{
}

bool
iControllerImpl::connect()
{
    if ( adplugin::plugin * plugin = adplugin::loader::loadLibrary( "ap240controller" ) ) {

        if ( auto manager = plugin->query_interface< adacquire::manager >() ) {
            if ( auto session = manager->session( "ap240::icontrollerimpl" ) ) {
                adextension::iControllerImpl::connect( session, "ap240::iControllerImpl" );
                return true;
            }
        }

    }
    
    return false;
}

