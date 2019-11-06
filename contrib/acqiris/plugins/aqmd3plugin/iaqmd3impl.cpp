/**************************************************************************
** Copyright (C) 2014-2020 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "iaqmd3impl.hpp"
#include <aqmd3/session.hpp>
#include <adplugin/plugin.hpp>
#include <adplugin_manager/loader.hpp>
#include <adportable/debug.hpp>
#include <adacquire/manager.hpp>

using namespace aqmd3;

iAQMD3Impl::iAQMD3Impl() : adextension::iControllerImpl("aqmd3")
{
    ADDEBUG() << "iAQMD3Impl ctor";
}

iAQMD3Impl::~iAQMD3Impl()
{
    ADDEBUG() << "iAQMD3Impl dtor";
}

bool
iAQMD3Impl::connect()
{
    ADDEBUG() << __FUNCTION__;

    if ( auto aqmd3 = std::make_shared< aqmd3::session >() ) {
        adextension::iControllerImpl::connect( aqmd3.get(), "iAQMD3Impl" );
        return true;
    }

    return false;
}
