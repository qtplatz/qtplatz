/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef ACQUIREDDATA_HPP
#define ACQUIREDDATA_HPP

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace adfs { class sqlite; }

namespace adutils {

    class AcquiredData  {
    public:
        AcquiredData();

        static bool insert( adfs::sqlite& db
                            , uint64_t objid
                            , int64_t time
                            , int32_t pos
                            , int32_t fcn
                            , uint32_t events
                            , const char * data
                            , size_t dsize
                            , const char * meta = 0
                            , size_t msize = 0 );

        static bool create_table( adfs::sqlite& db );
    };

}

#endif // ACQUIREDDATA_HPP
