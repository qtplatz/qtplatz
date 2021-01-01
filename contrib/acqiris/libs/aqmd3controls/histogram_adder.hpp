/**************************************************************************
** Copyright (C) 2010-2020 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2020 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "aqmd3controls_global.hpp"
#include "waveform.hpp"
#include "meta_data.hpp"
#include <adportable/float.hpp>
#include <atomic>
#include <vector>
#include <mutex>
#include <compiler/pragma_warning.hpp>

namespace adcontrols { class TimeDigitalHistogram; }

namespace aqmd3controls {

    class AQMD3CONTROLSSHARED_EXPORT pkd_result;

    class AQMD3CONTROLSSHARED_EXPORT histogram_adder;

    class histogram_adder {
        histogram_adder( const histogram_adder & ) = delete;
        histogram_adder& operator = ( const histogram_adder& ) = delete;

    public:
        histogram_adder();

        void clear();
        void reset();
        size_t append( const pkd_result& result );
        size_t append( const waveform& pkd );

        size_t actualAverage() const;
        double triggers_per_sec() const;

        uint32_t trigNumber( bool first = false ) const { return first ? serialnumber_0_ : serialnumber_; }
        uint64_t timeSinceEpoch( bool first = false ) const { return first ? timeSinceEpoch_0_ : timeSinceEpoch_; }

        std::shared_ptr< adcontrols::TimeDigitalHistogram > fetch( bool reset = true );
        // void move( adcontrols::TimeDigitalHistogram&, bool reset = true );

        const meta_data& meta() const { return meta_; }

        static void translate( adcontrols::TimeDigitalHistogram&, const waveform& );

    private:
        // metadata for initial trigger in this histogram
        meta_data meta_;
        uint32_t serialnumber_0_;             // first waveform trigger#
        uint32_t serialnumber_;               // last waveform trigger#
        uint64_t timeSinceEpoch_0_;           // first waveform acquired time
        uint64_t timeSinceEpoch_;             // last waveform acquired time
        uint32_t wellKnownEvents_;

        std::mutex mutex_;
        std::vector< uint32_t > data_;
        std::atomic< bool > reset_requested_;
    };

}
