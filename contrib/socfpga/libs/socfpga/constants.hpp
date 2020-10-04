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

    // Hamamatsu Photonics K.K.
    namespace hpk {
        // MIGHTION control                                   b7d2b693-b3cf-4b7d-a5a6-5bf7fc71d6f0
        constexpr boost::uuids::uuid dgmod_mcpad            = {{ 0xb7, 0xd2, 0xb6, 0x93, 0xb3, 0xcf, 0x4b, 0x7d, 0xa5, 0xa6, 0x5b, 0xf7, 0xfc, 0x71, 0xd6, 0xf0 }};
    }

    namespace infitof {
        // referenced in infitof2/constants.hpp

        // original msi hv control                            522A83E8-B1B9-4341-8B0F-CAC66D6D1E67
        constexpr boost::uuids::uuid dgmod_hvctl            = {{ 0x52, 0x2a, 0x83, 0xe8, 0xb1, 0xb9, 0x43, 0x41, 0x8b, 0x0f, 0xca, 0xc6, 0x6d, 0x6d, 0x1e, 0x67 }};

        // dgmod dgctl                                        34945677-4AD7-4D0F-B51D-4743F53514D4
        constexpr boost::uuids::uuid dgmod_dgctl            = {{ 0x34, 0x94, 0x56, 0x77, 0x4a, 0xd7, 0x4d, 0x0f, 0xb5, 0x1d, 0x47, 0x43, 0xf5, 0x35, 0x14, 0xd4 }};
        //                                                    381D7D88-1BF5-4BFA-9D27-16E702B74640
        constexpr boost::uuids::uuid dgmod_protocol         = {{ 0x38, 0x1d, 0x7d, 0x88, 0x1b, 0xf5, 0x4b, 0xfa, 0x9d, 0x27, 0x16, 0xe7, 0x02, 0xb7, 0x46, 0x40 }};

    }
}
