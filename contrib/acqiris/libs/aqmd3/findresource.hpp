/**************************************************************************
** Copyright (C) 2020 Toshinobu Hondo, Ph.D.
** Copyright (C) 2020 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License AgreeAQMD3SHARED_EXPORTment provided with the
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

#pragma once

#include "aqmd3_global.hpp"
#include <boost/optional.hpp>
#include <string>
#include <memory>


namespace aqmd3 {

    class AqMD3;

    struct AQMD3SHARED_EXPORT  findResource;

    struct findResource {
        const bool findConfig_;
        const bool saveConfig_;
        const std::string initOptions_;
        findResource( bool findConfig = true
                      , bool saveConfig = true
                      , const char * const options =
                      "Simulate=false, DriverSetup= Model=SA230P" );
                      // "Cache=true, InterchangeCheck=false, QueryInstrStatus=true, RangeCheck=true, RecordCoercions=false, Simulate=false" );
        ~findResource();
        boost::optional< std::string > operator()( std::shared_ptr< AqMD3 >, bool useConfigOnly = true ) const;
        bool operator()( std::shared_ptr< AqMD3 >, const std::string& res ) const;
        static std::vector< std::string > lspxi();
    };

}
