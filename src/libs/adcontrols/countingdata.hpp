// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
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

#include "adcontrols_global.h"
#include <string>
#include <tuple>
#include <vector>

namespace adcontrols {

    struct ADCONTROLSSHARED_EXPORT CountingPeak {
        std::pair< double, double > apex;
        std::pair< double, double > front;
        std::pair< double, double > back;
        CountingPeak();
        CountingPeak( const CountingPeak& t );
        double area() const;
        double height() const;
        double width() const;
    };
    
    class ADCONTROLSSHARED_EXPORT CountingData {
    public:
        CountingData();
        CountingData( const CountingData& );
        CountingData& operator = ( const CountingData& );

        uint32_t triggerNumber() const;
        uint32_t protocolIndex() const;
        uint64_t timeSinceEpoch() const;
        double elapsedTime() const;
        uint32_t events();
        double threshold();
        uint32_t algo(); // 0:Absolute, 1:Average, 2:Deferential

        void setTriggerNumber( uint32_t );
        void setProtocolIndex( uint32_t );
        void setTimeSinceEpoch( uint64_t );
        void setElapsedTime( double );
        void setEvents( uint32_t );
        void setThreshold( double );
        void setAlgo( uint32_t );

        std::vector< CountingPeak >& peaks() { return peaks_; }
        const std::vector< CountingPeak >& peaks() const { return peaks_; }

    private:
        uint32_t triggerNumber_;
        uint32_t protocolIndex_;
        uint64_t timeSinceEpoch_;
        double elapsedTime_;
        uint32_t events_;
        double threshold_;
        uint32_t algo_;
        std::vector< CountingPeak > peaks_;
    };

}



