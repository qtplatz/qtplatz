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

#include <boost/property_tree/ptree_fwd.hpp>
#include <string>
#include <vector>
#include <cstdint>

namespace adio {
    namespace io {
    
        class sample {
        public:
            sample();
            sample( const sample& );
            uint32_t id_;
            double runLength_;
            double injVolume_;
            std::string sampleId_;
            std::string description_;
            std::string methodId_;
        };

        class sequence {
        public:
            sequence();
            sequence( const sequence& );
            
            static bool read_json( std::istream&, sequence& );
            static bool write_json( std::ostream&, const sequence& );

            size_t replicates() const { return replicates_; };
            size_t& replicates() { return replicates_; };

            inline std::vector< sample >& samples() { return samples_; }
            inline const std::vector< sample >& samples() const { return samples_; }
            
        private:
            std::vector< sample > samples_;
            size_t replicates_;
            static bool read_json( const boost::property_tree::ptree&, sample& );
            static bool write_json( boost::property_tree::ptree&, const sample& );
        };
        
    }
}
