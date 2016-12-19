// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC
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

#include <string>
#include <vector>
#include <cstdint>
#include <boost/property_tree/ptree_fwd.hpp>

namespace adportable {
    namespace dg {

        class ioState {
        public:
            ioState();
            ioState( const ioState& );
            ioState( bool enable
                      , uint32_t id
                      , ioMode mode
                      , uint32_t trig
                      , ioState initState
                      , const std::string& name
                      , const std::string& note = std::string() );
            bool        enable_;
            uint32_t    id_;
            ioMode      mode_;        // OUT|IN
            uint32_t    trigConfig_;  // Edge|Negative = INJ.IN
            ioState     initState_;   // initial state
            std::string name_;
            std::string note_;
        };


    }
}
