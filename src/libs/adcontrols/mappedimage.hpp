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
#include <boost/numeric/ublas/matrix.hpp>
#include <limits>
#include <vector>
#include <cstdint>

namespace boost { namespace serialization { class access; } }
namespace adcontrols { class idAudit; }

namespace adcontrols {

    template<typename> class imageFrame;
    
    /** \class MappedImage
     * 
     * \brief An sliced image of MappedSpectra.
     *
     */
    
    class ADCONTROLSSHARED_EXPORT MappedImage {
    public:
        ~MappedImage( void );
        MappedImage( void );
        MappedImage( size_t i, size_t j );
        MappedImage( const MappedImage & );
        MappedImage & operator = ( const MappedImage & rhs );
        
        typedef std::pair< uint32_t, uint32_t > datum_type;
        typedef std::vector< datum_type >::iterator iterator;
        typedef std::vector< datum_type >::const_iterator const_iterator;

        const adcontrols::idAudit& ident() const;

        size_t size1() const;  // horizontal pixel count
        size_t size2() const;  // vertical pixel count

        double operator ()( size_t i, size_t j );
        const double operator ()( size_t i, size_t j ) const;

        bool merge( const boost::numeric::ublas::matrix<uint16_t>&
                    , unsigned int low = 0
                    , unsigned int high = std::numeric_limits<unsigned int>::max() );

        operator const boost::numeric::ublas::matrix< double >& () const;
        double max_z() const;
        size_t mergeCount() const;

    private:
        class impl;
        impl * impl_;

        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int version );
    };
}


