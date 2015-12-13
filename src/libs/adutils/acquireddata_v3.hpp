/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
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

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace boost { namespace uuids { struct uuid; } }
namespace adfs { class sqlite; }

namespace adutils {

    namespace v3 {
    
        class AcquiredData  {
        public:
            AcquiredData();
            
            static bool insert( adfs::sqlite&
                                , const boost::uuids::uuid& objid
                                , uint64_t elapsed_time
                                , uint64_t epoch_time
                                , uint64_t pos
                                , uint32_t fcn
                                , uint32_t ndata 
                                , uint32_t events
                                , const std::string& data
                                , const std::string& meta );

            static bool create_table_v3( adfs::sqlite& db );
        
        };
    }
}

