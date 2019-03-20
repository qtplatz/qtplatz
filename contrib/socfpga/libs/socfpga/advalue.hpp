/**************************************************************************
** Copyright (C) 2013-2019 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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

#pragma once

#include <adacquire/instrument.hpp>
#include <adacquire/signalobserver.hpp>

namespace socfpga {

    namespace dgmod {
        // Session class define here is psude singletion by a manager class
        // which is only the class make Session instance.

        struct advalue {
            uint64_t elapsed_time;
            uint64_t flags_time;
            uint64_t posix_time;
            uint32_t adc_counter; // 250kHz counter
            uint32_t nacc;
            uint32_t flags;
            std::array< double, 8 > ad;
            advalue() : elapsed_time(0), flags_time(0), posix_time(0), adc_counter(0), nacc(0), flags{0}, ad{{ 0 }} {}
             advalue( const advalue& t ) : elapsed_time( t.elapsed_time )
                                         , flags_time( t.flags_time )
                                         , posix_time( t.posix_time )
                                         , adc_counter( t.adc_counter )
                                         , nacc( t.nacc )
                                         , flags( t.flags )
                                         , ad( {t.ad} )
                {}
        };

    }
}
