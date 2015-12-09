/**************************************************************************
** Copyright (C) 2015-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2015-2016 MS-Cheminformatics LLC
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
#include <boost/serialization/version.hpp>
#include <vector>
#include <cstdint>
#include <iostream>
#include <compiler/pragma_warning.hpp>

namespace boost { namespace serialization { class access; } }

namespace adcontrols {

    class MassSpectrum;

    template< typename T > class TimeDigitalHistogram_archive;

	class ADCONTROLSSHARED_EXPORT TimeDigitalHistogram {
	public:
        typedef std::pair< double, uint32_t > value_type; // time(s), intensity
        typedef std::vector< value_type >::iterator iterator;
        typedef std::vector< value_type >::const_iterator const_iterator;

        TimeDigitalHistogram();
        TimeDigitalHistogram( const TimeDigitalHistogram& );

        size_t size() const;
        const value_type& operator [] ( size_t idx ) const;
        iterator begin();
        iterator end();
        const_iterator begin() const;
        const_iterator end() const;
        
        double& initialXTimeSeconds();                    // digitizer time stamp, since acquisition started
        double& initialXOffset();                         // digitizer acquisition start offset ( delay )
        double& xIncrement();                             // digitizer sampling interval
        uint64_t& actualPoints();                         // digitizer waveform length (for spectrum display)
        uint64_t& trigger_count();
        uint32_t& wellKnownEvents();
        std::pair< uint64_t, uint64_t >& serialnumber();
        std::pair< uint64_t, uint64_t >& timeSinceEpoch();
        double initialXTimeSeconds() const;
        double initialXOffset() const;
        double xIncrement() const;
        uint64_t actualPoints() const;
        uint64_t trigger_count() const;
        uint32_t wellKnownEvents() const;

        const std::pair< uint64_t, uint64_t >& serialnumber() const;
        const std::pair< uint64_t, uint64_t >& timeSinceEpoch() const;
        std::vector< std::pair< double, uint32_t > >& histogram();

        uint32_t accumulate( double tof, double window ) const;

        double triggers_per_second() const;

        static bool translate( adcontrols::MassSpectrum&, const TimeDigitalHistogram& );

        static bool archive( std::ostream&, const TimeDigitalHistogram& );
        static bool restore( std::istream&, TimeDigitalHistogram& );

    private:
        double initialXTimeSeconds_;                       // digitizer time stamp, since acquisition started
        double initialXOffset_;                            // digitizer acquisition start offset ( delay )
        double xIncrement_;                                // digitizer sampling interval
        uint64_t actualPoints_;                            // digitizer waveform length (for spectrum display)
        uint64_t trigger_count_;
        uint32_t wellKnownEvents_;

        pragma_msvc_warning_push_disable_4251

        std::pair< uint64_t, uint64_t > serialnumber_;     // first, last waveform trigger#
        std::pair< uint64_t, uint64_t > timeSinceEpoch_;   // first waveform acquired time
        std::vector< std::pair< double, uint32_t > > histogram_;

        pragma_msvc_warning_pop

        friend class TimeDigitalHistogram_archive< TimeDigitalHistogram >;
        friend class TimeDigitalHistogram_archive< const TimeDigitalHistogram >;

        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int version );
	};

}

