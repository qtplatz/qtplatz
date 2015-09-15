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

#ifndef MSCALIBIO_HPP
#define MSCALIBIO_HPP

#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>
#include <string>

namespace adfs { class sqlite; }
namespace adcontrols { class MSCalibrateResult; }
namespace boost { namespace uuids { struct uuid; } }

namespace adicontroller {

    namespace v3 {

        class mscalibio {
        public:
            mscalibio();
            static bool readCalibration( adfs::sqlite& db
                                         , const boost::uuids::uuid& objuuid
                                         , const std::string& dataClass
                                         , std::vector< char >& serailized
                                         , int64_t& revision );
            
            static bool writeCalibration( adfs::sqlite& db
                                          , const boost::uuids::uuid& objuuid
                                          , const std::string& calibId, const std::string& dataClass, const char * data, size_t size );
            static bool create_table_v3( adfs::sqlite& db );
        };
    }
    
}

#endif // MSCALIBIO_HPP
