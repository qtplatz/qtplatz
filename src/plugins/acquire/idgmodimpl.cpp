/**************************************************************************
** Copyright (C) 2014-2019 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "idgmodimpl.hpp"
#include "dgmod/session.hpp"
#include <adplugin/plugin.hpp>
#include <adplugin_manager/loader.hpp>
#include <adportable/debug.hpp>
#include <adacquire/manager.hpp>

using namespace acquire;

iDGMODImpl::iDGMODImpl() : adextension::iControllerImpl("DGMOD")
{
    ADDEBUG() << "iDGMODImpl ctor";
}

iDGMODImpl::~iDGMODImpl()
{
    ADDEBUG() << "iDGMODImpl dtor";
}

bool
iDGMODImpl::connect()
{
    ADDEBUG() << "############################### " << __FUNCTION__;

    if ( auto session = std::make_shared< acquire::dgmod::session >() ) {
        adextension::iControllerImpl::connect( session.get(), "iDGMODImpl" );
        return true;
    }

    return false;
}
