/**************************************************************************
** Copyright (C) 2020 Toshinobu Hondo, Ph.D.
** Copyright (C) 2020 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "findresource.hpp"
#include "configfile.hpp"
#include "aqmd3.hpp"
#include <adportable/debug.hpp>
#include <boost/format.hpp>

using namespace aqmd3;

findResource::~findResource()
{
}

findResource::findResource( bool findConfig
                            , bool saveConfig
                            , const char * const options ) : findConfig_( findConfig )
                                                           , saveConfig_( saveConfig )
                                                           , initOptions_( options )
{
}

boost::optional< std::string >
findResource::operator()( std::shared_ptr< AqMD3 > md3 ) const
{
    if ( findConfig_ ) {
        if ( auto res = configFile().loadResource() ) {
            if ( md3->clog( md3->initWithOptions( *res, VI_FALSE, VI_TRUE, initOptions_ ), __FILE__, __LINE__, [&]{ return *res; }) )
                return res;
        }
    }

    for ( int num = 0; num < 200; ++num ) {
        std::string res = ( boost::format("PXI%d::0::0::INSTR") % num ).str();
        if ( md3->clog( md3->initWithOptions( res.c_str(), VI_FALSE, VI_TRUE, initOptions_ ), __FILE__, __LINE__, [&]{ return res; } ) ) {
            if ( saveConfig_ ) {
                aqmd3::configFile().saveResource( res );
            }
            return res;
        }
    }
    return boost::none;
}

bool
findResource::operator()( std::shared_ptr< AqMD3 > md3, const std::string& res ) const
{
    if ( md3->clog( md3->initWithOptions( res, VI_FALSE, VI_TRUE, initOptions_ ), __FILE__, __LINE__, [&]{ return res; } ) ) {
        if ( saveConfig_ )
            aqmd3::configFile().saveResource( res );
        return true;
    }
    return false;
}
