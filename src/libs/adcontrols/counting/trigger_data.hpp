// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2013-2019 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
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

#include <cstdint>

namespace adcontrols {

    namespace counting {

        struct trigger_data {
        public:
            uint32_t serialnumber;
            uint32_t protocolIndex;
            uint64_t timeSinceEpoch;
            double initialXTimeSeconds;
            uint32_t wellKnownEvents;
            double thresholdLevel;
            uint32_t algo;
            trigger_data() : serialnumber( 0 )
                           , protocolIndex( 0 )
                           , timeSinceEpoch( 0 )
                           , initialXTimeSeconds( 0 )
                           , wellKnownEvents( 0 )
                           , thresholdLevel( 0 )
                           , algo( 0 ) {
            }
        };
    }
}
