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
#include <boost/serialization/version.hpp>
#include <vector>
#include <cstdint>

namespace boost { namespace serialization { class access; } }

namespace adcontrols {
        
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
        MappedSpectrum& operator << ( const datum_type& );

        inline double time( size_t idx ) const { return data_[ idx ].first; }
        inline uint32_t intensity( size_t idx ) const { return data_[ idx ].second; }
            
        iterator begin();
        iterator end();
        const_iterator begin() const;
        const_iterator end() const;
        iterator erase( iterator first, iterator last );

        double tic() const;
            
    private:

# if _MSC_VER
# pragma warning( disable:4251 )            
# endif
        std::vector< datum_type > data_;

        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int version );
    };
}

BOOST_CLASS_VERSION( adcontrols::MappedSpectrum, 1 )


