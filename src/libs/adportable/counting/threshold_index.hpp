/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <cstdint>

namespace adportable {

    namespace counting {

        struct threshold_index {

            uint32_t first;
            uint32_t second;
            uint32_t apex;   // index for max(min) element for threshold find; peak-apex element for differential mode
            int32_t value;   // intensity for apex
            int32_t level;   // baseline intensity (binary value) for AverageRelative algorithm
        
            threshold_index( uint32_t _first = 0
                             , uint32_t _second = 0
                             , uint32_t _apex = 0
                             , int16_t _value = 0
                             , int32_t _level = 0 )
                : first( _first ), second( _second ), apex( _apex )
                , value( _value ), level( _level ) {
            }
        private:
            friend class boost::serialization::access;
            template<class Archive>
            void serialize( Archive& ar, const unsigned int version ) {
                ar & BOOST_SERIALIZATION_NVP( first );
                ar & BOOST_SERIALIZATION_NVP( second );
                ar & BOOST_SERIALIZATION_NVP( apex );
                ar & BOOST_SERIALIZATION_NVP( value );
                ar & BOOST_SERIALIZATION_NVP( level );
            }
        };
    }
}

BOOST_CLASS_VERSION( adportable::counting::threshold_index, 1 )
