/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "../acqrscontrols_global.hpp"
#include "waveform.hpp"
#include <adportable/float.hpp>  
#include <atomic>
#include <vector>
#include <mutex>

namespace adcontrols { class TimeDigitalHistogram; }

namespace acqrscontrols {

    template< typename T > class threshold_result_;
    
    namespace ap240 {

        class method;

        class ACQRSCONTROLSSHARED_EXPORT histogram {

            histogram( const histogram & ) = delete;
            histogram& operator = ( const histogram& ) = delete;

        public:
            histogram();

            void clear();
            void reset();
            size_t append( const acqrscontrols::threshold_result_< waveform >& result ); 
            size_t trigger_count() const;
            double triggers_per_sec() const;

            uint32_t trigNumber( bool first = false ) const { return first ? serialnumber_0_ : serialnumber_; }
            uint64_t timeSinceEpoch( bool first = false ) const { return first ? timeSinceEpoch_0_ : timeSinceEpoch_; }

            size_t getHistogram( std::vector< std::pair<double, uint32_t> >& histogram
                                 , acqrscontrols::ap240::metadata& meta
                                 , acqrscontrols::ap240::method& method
                                 , std::pair<uint32_t, uint32_t>& serialnumber
                                 , std::pair<uint64_t, uint64_t>& timeSinceEpoch );

            static bool average( const std::vector< std::pair< double, uint32_t > >&
                                 , double resolution, std::vector< double >& times, std::vector< double >& intens );

            static bool average( const std::vector< std::pair< double, uint32_t > >&, double resolution, std::vector < std::pair< double, uint32_t > >& output );

            void move( adcontrols::TimeDigitalHistogram&, bool reset = true );
            const acqrscontrols::ap240::method& method() const { return method_; }

        private:
            // metadata for initial trigger in this histogram
            acqrscontrols::ap240::metadata meta_;
            acqrscontrols::ap240::method   method_;
            uint32_t serialnumber_0_;             // first waveform trigger#
            uint32_t serialnumber_;               // last waveform trigger#
            uint64_t timeSinceEpoch_0_;           // first waveform acquired time
            uint64_t timeSinceEpoch_;             // last waveform acquired time
            uint32_t wellKnownEvents_;

            std::mutex mutex_;
            std::atomic< size_t > trigger_count_; // number of triggers
            std::vector< uint32_t > data_;
            std::atomic< bool > reset_requested_;

        };

    }
}

