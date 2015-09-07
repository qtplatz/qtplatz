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

using namespace ap240spectrometer::ap240;

method::method() : channels_( 0x01 )
{
}

method::method( const method& t ) : channels_( t.channels_ )
                                  , hor_( t.hor_ )
                                  , trig_( t.trig_ )
                                  , ext_( t.ext_ )
                                  , ch1_( t.ch1_ )
                                  , ch2_( t.ch2_ )
                                  , slope1_( t.slope1_ )
                                  , slope2_( t.slope2_ )
{
}


namespace ap240spectrometer {
    namespace ap240 {

        class trigger_method::impl {
            trigger_method& _;
        public:
            impl( trigger_method& t ) : _(t) {}
        private:
            class impl; friend class impl;
            friend class boost::serialization::access;
            template<class Archive>
            void serialize( Archive& ar, const unsigned int version ) {
                using namespace boost::serialization;
                ar & BOOST_SERIALIZATION_NVP( _.trigClass );
                ar & BOOST_SERIALIZATION_NVP( _.trigPattern );
                ar & BOOST_SERIALIZATION_NVP( _.trigCoupling );
                ar & BOOST_SERIALIZATION_NVP( _.trigSlope );
                ar & BOOST_SERIALIZATION_NVP( _.trigLevel1 );
                ar & BOOST_SERIALIZATION_NVP( _.trigLevel2 );
            }
        };
        template<> void trigger_method::serialize( boost::archive::xml_woarchive& ar, const unsigned int )
        {
            ar & BOOST_SERIALIZATION_NVP( impl(*this) );
        }

        template<> void trigger_method::serialize( boost::archive::xml_wiarchive& ar, const unsigned int )
        {
            ar & BOOST_SERIALIZATION_NVP( impl(*this) );        
        }

        template<> void trigger_method::serialize( portable_binary_oarchive& ar, const unsigned int version )
        {
            ar << impl(*this);        
        }

        template<> void trigger_method::serialize( portable_binary_iarchive& ar, const unsigned int version )
        {
            ar >> impl(*this);
        }

        class horizontal_method::impl {
            horizontal_method& _;
        public:
            impl( horizontal_method& t ) : _(t) {}
        private:
            friend class boost::serialization::access;
            template<class Archive>
                void serialize( Archive& ar, const unsigned int version ) {
                using namespace boost::serialization;
                ar & BOOST_SERIALIZATION_NVP( _.sampInterval );
                ar & BOOST_SERIALIZATION_NVP( _.delay );
                ar & BOOST_SERIALIZATION_NVP( _.width );
                ar & BOOST_SERIALIZATION_NVP( _.mode );
                ar & BOOST_SERIALIZATION_NVP( _.flags );
                ar & BOOST_SERIALIZATION_NVP( _.nbrAvgWaveforms );
                ar & BOOST_SERIALIZATION_NVP( _.nStartDelay );
                ar & BOOST_SERIALIZATION_NVP( _.nbrSamples );
            }
        };
        template<> void horizontal_method::serialize( boost::archive::xml_woarchive& ar, const unsigned int )
        {
            ar & BOOST_SERIALIZATION_NVP( impl(*this) );
        }

        template<> void horizontal_method::serialize( boost::archive::xml_wiarchive& ar, const unsigned int )
        {
            ar & BOOST_SERIALIZATION_NVP( impl(*this) );        
        }

        template<> void horizontal_method::serialize( portable_binary_oarchive& ar, const unsigned int version )
        {
            ar << impl(*this);        
        }

        template<> void horizontal_method::serialize( portable_binary_iarchive& ar, const unsigned int version )
        {
            ar >> impl(*this);
        }
        
        
        class vertical_method::impl {
            vertical_method& _;
        public:
            impl( vertical_method& t ) : _(t) {}

            template<class Archive>
            void serialize( Archive& ar, const unsigned int version ) {
                using namespace boost::serialization;
                ar & BOOST_SERIALIZATION_NVP( _.fullScale );
                ar & BOOST_SERIALIZATION_NVP( _.offset );
                ar & BOOST_SERIALIZATION_NVP( _.coupling );
                ar & BOOST_SERIALIZATION_NVP( _.bandwidth );
                ar & BOOST_SERIALIZATION_NVP( _.invertData );
                ar & BOOST_SERIALIZATION_NVP( _.autoScale );
            }
        };
        template<> void vertical_method::serialize( boost::archive::xml_woarchive& ar, const unsigned int )
        {
            ar & BOOST_SERIALIZATION_NVP( impl(*this) );
        }

        template<> void vertical_method::serialize( boost::archive::xml_wiarchive& ar, const unsigned int )
        {
            ar & BOOST_SERIALIZATION_NVP( impl(*this) );        
        }

        template<> void vertical_method::serialize( portable_binary_oarchive& ar, const unsigned int version )
        {
            ar << impl(*this);        
        }

        template<> void vertical_method::serialize( portable_binary_iarchive& ar, const unsigned int version )
        {
            ar >> impl(*this);
        }
        
        
        
        ///////////////////////////////////////
        
        class method::impl {
            method& _;
        public:
            impl( method& t ) : _(t) {}

            template<class Archive>
            void serialize( Archive& ar, const unsigned int version ) {
                using namespace boost::serialization;
                ar & BOOST_SERIALIZATION_NVP( _.channels_ );            
                ar & BOOST_SERIALIZATION_NVP( _.trig_ );
                ar & BOOST_SERIALIZATION_NVP( _.hor_ );
                ar & BOOST_SERIALIZATION_NVP( _.ext_ );
                ar & BOOST_SERIALIZATION_NVP( _.ch1_ );
                ar & BOOST_SERIALIZATION_NVP( _.ch2_ );
                ar & BOOST_SERIALIZATION_NVP( _.slope1_ );
                ar & BOOST_SERIALIZATION_NVP( _.slope2_ );
            }
            
        };

        template<> void method::serialize( boost::archive::xml_woarchive& ar, const unsigned int )
        {
            ar & BOOST_SERIALIZATION_NVP( impl(*this) );
        }

        template<> void method::serialize( boost::archive::xml_wiarchive& ar, const unsigned int )
        {
            ar & BOOST_SERIALIZATION_NVP( impl(*this) );        
        }

        template<> void method::serialize( portable_binary_oarchive& ar, const unsigned int version )
        {
            ar << impl(*this);        
        }

        template<> void method::serialize( portable_binary_iarchive& ar, const unsigned int version )
        {
            ar >> impl(*this);
        }

        bool method::archive( std::ostream& os, const method& t )
        {
            try {
                portable_binary_oarchive ar( os );
                ar & impl(const_cast<method&>(t));
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
                ar & impl(t);
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
                ar & boost::serialization::make_nvp( "ap240_method", impl(const_cast<method&>(t)) );
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
                ar & boost::serialization::make_nvp( "ap240_method", impl(t) );
                return true;
            } catch ( std::exception& ex ) {
                BOOST_THROW_EXCEPTION( ex );
            }
            return false;

        }
    }
}
