/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "method.hpp"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>
#include <adportable/portable_binary_iarchive.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>


namespace acqrscontrols {
    namespace u5303a {

        template<typename T = method>
        class method_archive {
        public:
            template<class Archive>
            void serialize( Archive& ar, T& _, const unsigned int version ) {
                // Since V4
                using namespace boost::serialization;
                ar & BOOST_SERIALIZATION_NVP( _.channels_ );
                ar & BOOST_SERIALIZATION_NVP( _.mode_ );
                ar & BOOST_SERIALIZATION_NVP( _.method_ );
                ar & BOOST_SERIALIZATION_NVP( _.threshold_ );
            }

        };

        template<> ACQRSCONTROLSSHARED_EXPORT void method::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
        {
            method_archive<>().serialize( ar, *this, version );
        }

        template<> ACQRSCONTROLSSHARED_EXPORT void method::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
        {
            method_archive<>().serialize( ar, *this, version );
        }

        template<> void method::serialize( portable_binary_oarchive& ar, const unsigned int version )
        {
            method_archive<>().serialize( ar, *this, version );
        }

        template<> void method::serialize( portable_binary_iarchive& ar, const unsigned int version )
        {
            method_archive<>().serialize( ar, *this, version );
        }

        bool method::archive( std::ostream& os, const method& t )
        {
            try {
                portable_binary_oarchive ar( os );
                ar & boost::serialization::make_nvp( "m", t );
                return true;
            } catch ( std::exception& ex ) {
                BOOST_THROW_EXCEPTION( ex );
            }
            return false;
        }

        bool method::restore( std::istream& is, method& t )
        {
            try {
                portable_binary_iarchive ar( is );
                ar & boost::serialization::make_nvp( "m", t );
                return true;
            } catch ( std::exception& ex ) {
                BOOST_THROW_EXCEPTION( ex );
            }
            return false;
        }

        bool method::xml_archive( std::wostream& os, const method& t )
        {
            try {
                boost::archive::xml_woarchive ar( os );
                ar & boost::serialization::make_nvp( "m", t );
                return true;
            } catch ( std::exception& ex ) {
                BOOST_THROW_EXCEPTION( ex );
            }
            return false;
        }

        bool method::xml_restore( std::wistream& is, method& t )
        {
            try {
                boost::archive::xml_wiarchive ar( is );
                ar & boost::serialization::make_nvp( "m", t );
                return true;
            } catch ( std::exception& ex ) {
                BOOST_THROW_EXCEPTION( ex );
            }
            return false;

        }
    }
}

using namespace acqrscontrols::u5303a;

method:: method() : channels_( 0x01 )
                  , mode_( 2 ) // averager mode
{
}

method:: method( const method& t ) : channels_( t.channels_ )
                                   , mode_( t.mode_ )
                                   , method_( t.method_ )
                                   , threshold_( t.threshold_ )
{
}
