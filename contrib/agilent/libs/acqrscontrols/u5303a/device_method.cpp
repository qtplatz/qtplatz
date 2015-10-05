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

#include "device_method.hpp"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>
#include <adportable/portable_binary_iarchive.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>


namespace acqrscontrols {
    
    namespace u5303a {

        template<typename T = device_method>
        class device_method_archive {
        public:
            template<class Archive>
            void serialize( Archive& ar, T& _, const unsigned int version ) {

                using namespace boost::serialization;

                ar & BOOST_SERIALIZATION_NVP( _.front_end_range );
                ar & BOOST_SERIALIZATION_NVP( _.front_end_offset );
                ar & BOOST_SERIALIZATION_NVP( _.ext_trigger_level );
                ar & BOOST_SERIALIZATION_NVP( _.samp_rate );
                ar & BOOST_SERIALIZATION_NVP( _.nbr_of_s_to_acquire_ );
                ar & BOOST_SERIALIZATION_NVP( _.nbr_of_averages );
                ar & BOOST_SERIALIZATION_NVP( _.delay_to_first_sample_ );
                ar & BOOST_SERIALIZATION_NVP( _.invert_signal );
                ar & BOOST_SERIALIZATION_NVP( _.nsa );
                if ( version >= 3 ) {
                    ar & BOOST_SERIALIZATION_NVP( _.digitizer_delay_to_first_sample );
                    ar & BOOST_SERIALIZATION_NVP( _.digitizer_nbr_of_s_to_acquire );
                }

            }

        };

        template<> ACQRSCONTROLSSHARED_EXPORT void device_method::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
        {
            device_method_archive<>().serialize( ar, *this, version );
        }

        template<> ACQRSCONTROLSSHARED_EXPORT void device_method::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
        {
            device_method_archive<>().serialize( ar, *this, version );
        }

        template<> ACQRSCONTROLSSHARED_EXPORT void device_method::serialize( portable_binary_oarchive& ar, const unsigned int version )
        {
            device_method_archive<>().serialize( ar, *this, version );
        }

        template<> ACQRSCONTROLSSHARED_EXPORT void device_method::serialize( portable_binary_iarchive& ar, const unsigned int version )
        {
            device_method_archive<>().serialize( ar, *this, version );
        }

    }
}

using namespace acqrscontrols::u5303a;

device_method::device_method() : front_end_range( 2.0 )         // 1V,2V range
                               , front_end_offset( 0.0 )        // [-0.5V,0.5V], [-1V,1V] offset
                               , ext_trigger_level( 1.0 )       // external trigger threshold
                               , samp_rate( 1.0e9 )			   // sampling rate (1.0GS/s)
                               , nbr_of_s_to_acquire_( 100000 ) // from 1 to 480,000 samples
                               , nbr_of_averages( 512 )		   // number of averages minus one. >From 0 to 519,999 averages in steps of 8. For instance 0,7,15
                               , delay_to_first_sample_( 0 )    // delay from trigger (seconds)
                               , invert_signal( 0 )             // 0-> no inversion , 1-> signal inverted
                               , nsa( 0x0 )
                               , digitizer_delay_to_first_sample( 0 )    // device set value
                               , digitizer_nbr_of_s_to_acquire( 100000 ) // device set value
{
}
