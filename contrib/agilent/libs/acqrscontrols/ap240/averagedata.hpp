/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#pragma once

#include <cstdint>
#include <cstdlib>
#include <memory>
#include <algorithm>
#include <adportable/waveform_averager.hpp>
#include "waveform.hpp"
#include "metadata.hpp"  // acqriscontrols::ap240::metadata
#include "method.hpp"    // acqriscontrols::ap240::method

namespace adcontrols { class TimeDigitalHistogram; }

namespace acqrscontrols {
    namespace ap240 {

        class histogram;
        class metadata;
        class method;
        
        typedef adportable::waveform_averager< int32_t > averager_type;
        typedef waveform waveform_type;

        class AverageData {
        public:
            
            AverageData();
            AverageData( const AverageData& t );

            size_t average_waveform( const acqrscontrols::ap240::waveform& );
            void reset();

            uint32_t protocolIndex_; // := this->method_.protocolIndex()
            uint32_t protocolCount_; // := this->method_.protocols().size()
            metadata meta_;
            method   method_;
            uint32_t serialnumber_origin_;
            uint32_t serialnumber_;
            uint32_t wellKnownEvents_;
            uint64_t timeSinceEpoch_;
            double timeSinceInject_;
            std::shared_ptr< const identify > ident_;
            std::shared_ptr< averager_type > waveform_register_;
            std::shared_ptr< histogram > histogram_register_;
        };

    }
}

