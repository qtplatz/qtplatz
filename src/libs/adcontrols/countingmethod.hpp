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
#include "msfinder.hpp"
#include <boost/serialization/version.hpp>
#include <cstdint>
#include <string>
#include <vector>

namespace boost {
    namespace serialization {  class access;  }
    namespace uuids { struct uuid; }
}

namespace adcontrols {

    template< typename T > class CountingMethod_archive;

    class ADCONTROLSSHARED_EXPORT CountingMethod {
    public:
        const boost::uuids::uuid& clsid();
        
        CountingMethod();
        CountingMethod( const CountingMethod& );
        CountingMethod& operator = ( const CountingMethod& );

        typedef std::tuple< bool                          // enable
                            , std::string                 // formula
                            , std::pair< double, double > // (tof start, width in s)
                            , int                         // protocol
                            > value_type;
        enum {
            CountingEnable
            , CountingFormula
            , CountingRange
            , CountingProtocol
        };

        typedef std::vector< value_type >::iterator iterator;
        typedef std::vector< value_type >::const_iterator const_iterator;

        bool enable() const;
        void setEnable( bool );

        size_t size() const;

        void clear();
        
        value_type& operator [] ( size_t );
        const value_type& operator [] ( size_t ) const;
        iterator begin();
        iterator end();
        const_iterator begin() const;
        const_iterator end() const;

        CountingMethod& operator << ( value_type&& );

    private:
        bool enable_;
        std::vector< value_type > values_;

        friend class CountingMethod_archive< CountingMethod >;
        friend class CountingMethod_archive< const CountingMethod >;
        friend class boost::serialization::access;
        template<class Archive> void serialize(Archive& ar, const unsigned int version );
    };

}

BOOST_CLASS_VERSION( adcontrols::CountingMethod, 1 )


