/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "metadata.hpp"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>
#include <adportable/portable_binary_iarchive.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>

namespace acqiris {

    template<typename T = metadata>
    class metadata_archive {
    public:
        template<class Archive>
        void serialize( Archive& ar, T& _, const unsigned int version ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( _.actualPoints );
            ar & BOOST_SERIALIZATION_NVP( _.flags );
            ar & BOOST_SERIALIZATION_NVP( _.actualAverages );
            ar & BOOST_SERIALIZATION_NVP( _.indexFirstPoint );
            ar & BOOST_SERIALIZATION_NVP( _.channel );
            ar & BOOST_SERIALIZATION_NVP( _.dataType );
            ar & BOOST_SERIALIZATION_NVP( _.protocolIndex );
            ar & BOOST_SERIALIZATION_NVP( _.initialXTimeSeconds );
            ar & BOOST_SERIALIZATION_NVP( _.initialXOffset );
            ar & BOOST_SERIALIZATION_NVP( _.xIncrement );
            ar & BOOST_SERIALIZATION_NVP( _.scaleFactor );
            ar & BOOST_SERIALIZATION_NVP( _.scaleOffset );
            ar & BOOST_SERIALIZATION_NVP( _.horPos );
        }
    };

    template<> void metadata::serialize( boost::archive::xml_woarchive& ar, unsigned int version )
    {
        metadata_archive<>().serialize( ar, *this, version );
    }
        
    template<> void metadata::serialize( boost::archive::xml_wiarchive& ar, unsigned int version )
    {
        metadata_archive<>().serialize( ar, *this, version );
    }

    template<> void metadata::serialize( portable_binary_oarchive& ar, unsigned int version )
    {
        metadata_archive<>().serialize( ar, *this, version );
    }
        
    template<> void metadata::serialize( portable_binary_iarchive& ar, unsigned int version )
    {
        metadata_archive<>().serialize( ar, *this, version );
    }
}

using namespace acqiris;

metadata::metadata() : actualPoints( 0 )
                     , flags( 0 )
                     , actualAverages( 0 )
                     , indexFirstPoint( 0 )
                     , channel( 1 )
                     , dataType( 1 )
                     , protocolIndex( 0 )
                     , initialXTimeSeconds( 0 )
                     , initialXOffset( 0 )
                     , scaleFactor( 0 )
                     , xIncrement( 0 )
                     , scaleOffset( 0 )
                     , horPos( 0 )
{
}

