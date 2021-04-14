/**************************************************************************
** Copyright (C) 2021 Toshinobu Hondo, Ph.D.
** Copyright (C) 2021 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include <visa.h>
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
findResource::operator()( std::shared_ptr< AqMD3 > md3, bool configOnly ) const
{
    if ( findConfig_ ) {
        if ( auto res = configFile().loadResource() ) {
            if ( md3->clog( md3->initWithOptions( *res, VI_FALSE, VI_FALSE, initOptions_ ), __FILE__, __LINE__, [&]{ return *res; }) )
                return res;
        }
    }
    if ( !configOnly ) {
        for ( int num = 0; num < 200; ++num ) {
            std::string res = ( boost::format("PXI%d::0::0::INSTR") % num ).str();
            if ( md3->clog( md3->initWithOptions( res.c_str(), VI_FALSE, VI_FALSE, initOptions_ ), __FILE__, __LINE__, [&]{ return res; } ) ) {
                if ( saveConfig_ ) {
                    aqmd3::configFile().saveResource( res );
                }
                return res;
            }
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


std::vector< std::string >
findResource::lspxi()
{
    std::vector< std::string > res;
    ViSession rm( VI_NULL );
    viOpenDefaultRM( &rm );
    ViFindList find = VI_NULL;
    ViUInt32 count = 0;
    ViChar rsrc[256];
    auto status = viFindRsrc( rm, "PXI?*::INSTR", &find, &count, rsrc );
    if ( status == VI_SUCCESS && count > 0 ) {
        do {
            res.emplace_back( rsrc );
            status = viFindNext( find, rsrc );
        } while ( status == VI_SUCCESS );
        viClose( find );
    }
    viClose( rm );
    return res;
}
