/**************************************************************************
** Copyright (C) 2013-2019 MS-Cheminformatics LLC, Toin, Mie Japan
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

#pragma once

#include <boost/uuid/uuid.hpp>
#define HAVE_DMAC_0 0

namespace socfpga {
    namespace dgmod {

        constexpr const char * const trace_observer_name    = "1.httpd-dg.ms-cheminfo.com";

        //dd141c3a-7b57-454c-9515-23242c0345a7
        constexpr boost::uuids::uuid trace_observer         = {{ 0xdd, 0x14, 0x1c, 0x3a, 0x7b, 0x57, 0x45, 0x4c, 0x95, 0x15, 0x23, 0x24, 0x2c, 0x03, 0x45, 0xa7 }};

        constexpr const char * const trace_datainterpreter  = "3c552747-5ef4-41ff-9074-aa8585cb1765";
    }
}
