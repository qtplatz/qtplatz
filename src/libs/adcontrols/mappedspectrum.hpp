/**************************************************************************
** Copyright (C) 2014-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2014-2015 MS-Cheminformatics LLC
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
#include <compiler/disable_dll_interface.h>
#include <boost/serialization/version.hpp>
#include <vector>
#include <cstdint>

namespace boost { namespace serialization { class access; } }

namespace adcontrols {

    class MassSpectrum;
        
    class ADCONTROLSSHARED_EXPORT MappedSpectrum {
    public:
        ~MappedSpectrum(void);
        MappedSpectrum(void);
        MappedSpectrum(const MappedSpectrum &);
        MappedSpectrum & operator = ( const MappedSpectrum & rhs );

        typedef std::pair< double, uint32_t > datum_type; // time(s), intensity
        typedef std::vector< datum_type >::iterator iterator;
        typedef std::vector< datum_type >::const_iterator const_iterator;

        size_t size() const;
        const datum_type& operator []( size_t idx ) const;
        // MappedSpectrum& operator << ( const datum_type& );
        MappedSpectrum& operator << ( datum_type&& );
        MappedSpectrum& operator += ( const MappedSpectrum& );

        inline double time( size_t idx ) const { return data_[ idx ].first; }
        inline uint32_t intensity( size_t idx ) const { return data_[ idx ].second; }
            
        iterator begin();
        iterator end();
        const_iterator begin() const;
        const_iterator end() const;
        iterator erase( iterator first, iterator last );

        double tic() const;
        uint32_t numAverage() const;
        void setNumAverage( uint32_t );
        void setTrigNumber( uint32_t, uint32_t origin = 0 );
        uint32_t trigNumber( bool sinceOrigin = true ) const;
        uint32_t trigNumberOrigin() const;
        std::pair<uint64_t, uint64_t>& timeSinceEpoch();
        const std::pair<uint64_t, uint64_t>& timeSinceEpoch() const;
        void setSamplingInfo( double samplingInterval, double delay, uint32_t nSamples );

        bool transform( adcontrols::MassSpectrum& );

        double acqDelay() const;
        double samplingInterval() const;
        uint32_t acqSamples() const;
        std::pair< double, double > acqTimeRange() const;
            
    private:

        std::vector< datum_type > data_;
        uint32_t num_average_;
        uint32_t trig_number_;
        uint32_t trig_number_origin_;
        std::pair<uint64_t, uint64_t> timeSinceEpoch_;
        double sampInterval_;
        double delay_;       // seconds to first sample
        uint32_t nSamples_;  // number of samples per trigger (width(s)/sampInterval)

        template<typename T> class serializer;
        friend class serializer<MappedSpectrum>;
        friend class serializer<const MappedSpectrum>;
        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int version );
    };

#if defined _MSC_VER
    template class ADCONTROLSSHARED_EXPORT std::vector < MappedSpectrum::datum_type > ;
#endif
}


BOOST_CLASS_VERSION( adcontrols::MappedSpectrum, 3 )


