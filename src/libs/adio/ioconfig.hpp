// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2016-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016-2022 MS-Cheminformatics LLC
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

#include "adio_global.h"
#include <boost/json/value_from.hpp>
#include <boost/json/value_to.hpp>
#include <string>
#include <vector>
#include <cstdint>

namespace adio {
    namespace io {

        enum ioMode       { OUT, IN };
        enum ioState      { High, Low };
        enum trigConfig   { Edge = 0, Level = 1, Positive = 0x00, Negative = 0x10 };

        class ADIOSHARED_EXPORT ioConfig {
        public:
            ioConfig();
            ioConfig( const ioConfig& );
            ioConfig( bool enable
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

        class ADIOSHARED_EXPORT configuration {
        public:
            configuration();
            configuration( const configuration& );

            // static bool read_json( std::istream&, configuration& );
            // static bool read_json( std::istream&, ioConfig& );
            // static bool write_json( std::ostream&, const configuration& );

            typedef typename std::vector< ioConfig >::const_iterator const_iterator;
            typedef typename std::vector< ioConfig >::iterator iterator;

            void clear();

            inline const_iterator begin() const { return config_.begin(); }
            inline const_iterator end() const { return config_.end(); }
            inline iterator begin() { return config_.begin(); }
            inline iterator end() { return config_.end(); }

            inline std::vector< ioConfig >& config() { return config_; }
        private:

            std::vector< ioConfig > config_;
        };

        ADIOSHARED_EXPORT
        void tag_invoke( boost::json::value_from_tag, boost::json::value&, const ioConfig& );

        ADIOSHARED_EXPORT
        ioConfig tag_invoke( boost::json::value_to_tag< ioConfig >&, const boost::json::value& jv );

    }
}
