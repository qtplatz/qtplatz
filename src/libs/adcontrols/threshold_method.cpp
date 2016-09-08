/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
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

#include "threshold_method.hpp"
#include <adportable/float.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>
#include <adportable/portable_binary_iarchive.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/uuid/uuid_generators.hpp>

using namespace adcontrols;

threshold_method::threshold_method() : enable( false )
                                     , threshold_level( 0.100 )  // 100mV
                                     , time_resolution( 0.5e-9 ) // 0.5ns
                                     , response_time( 0.0 )
                                     , slope( CrossDown )
                                     , use_filter( false )
                                     , filter( SG_Filter )
                                     , sgwidth( 5.0e-9 )         // 5ns
                                     , hCutoffHz( 200e6 )        // 200MHz
                                     , lCutoffHz( 0 )            // high-pass = off
                                     , complex_( true )
                                     , algo_( Absolute )
{
}

namespace adcontrols {

    template<typename T=threshold_method>
    class threshold_method_archive {
    public:

        template<class Archive>
        void serialize( Archive& ar, T& _, const unsigned int version ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( _.enable );
            ar & BOOST_SERIALIZATION_NVP( _.threshold_level );
            ar & BOOST_SERIALIZATION_NVP( _.time_resolution );
            ar & BOOST_SERIALIZATION_NVP( _.response_time );
            ar & BOOST_SERIALIZATION_NVP( _.slope );
            ar & BOOST_SERIALIZATION_NVP( _.use_filter );
            ar & BOOST_SERIALIZATION_NVP( _.filter );
            ar & BOOST_SERIALIZATION_NVP( _.sgwidth );
            ar & BOOST_SERIALIZATION_NVP( _.hCutoffHz );
            ar & BOOST_SERIALIZATION_NVP( _.complex_ );
            if ( version >= 1 )
                ar & BOOST_SERIALIZATION_NVP( _.lCutoffHz );
            if ( version >= 2 )
                ar & BOOST_SERIALIZATION_NVP( _.algo_ );                
        }
    };

    template<> ADCONTROLSSHARED_EXPORT void threshold_method::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
    {
        threshold_method_archive<>().serialize( ar, *this, version );
    }

    template<> ADCONTROLSSHARED_EXPORT void threshold_method::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        threshold_method_archive<>().serialize( ar, *this, version );        
    }

    template<> ADCONTROLSSHARED_EXPORT void threshold_method::serialize( portable_binary_oarchive& ar, const unsigned int version )
    {
        threshold_method_archive<>().serialize( ar, *this, version );
    }

    template<> ADCONTROLSSHARED_EXPORT void threshold_method::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        threshold_method_archive<>().serialize( ar, *this, version );        
    }

    bool threshold_method::archive( std::ostream& os, const threshold_method& t )
    {
        try {
            portable_binary_oarchive ar( os );
            ar & boost::serialization::make_nvp( "m", t );
            return true;
        } catch ( std::exception& ) {
        }
        return false;
    }

    bool threshold_method::restore( std::istream& is, threshold_method& t )
    {
        try {
            portable_binary_iarchive ar( is );
            ar & boost::serialization::make_nvp( "m", t );
            return true;
        } catch ( std::exception& ) {
        }
        return false;
    }

    bool threshold_method::xml_archive( std::wostream& os, const threshold_method& t )
    {
        try {
            boost::archive::xml_woarchive ar( os );
            ar & boost::serialization::make_nvp( "m", t );
            return true;
        } catch ( std::exception& ) {
        }
        return false;
    }

    bool threshold_method::xml_restore( std::wistream& is, threshold_method& t )
    {
        try {
            boost::archive::xml_wiarchive ar( is );
            ar & boost::serialization::make_nvp( "m", t );
            return true;
        } catch ( std::exception& ) {
        }
        return false;

    }
}

bool
threshold_method::operator != ( const threshold_method& m ) const
{
    namespace ap = adportable;
    
    if ( enable != m.enable ||
         (!ap::compare<double>::approximatelyEqual( threshold_level, m.threshold_level )) ||
         (!ap::compare<double>::approximatelyEqual( response_time, m.response_time )) ||
         slope != m.slope ||
         use_filter != m.use_filter )
        return true;

    if ( use_filter ) {
        if ( ( filter != m.filter ) ||
             ( ( m.filter == adcontrols::threshold_method::SG_Filter ) && 
               ( !ap::compare<double>::approximatelyEqual( sgwidth, m.sgwidth ) ) ) ||
             ( ( m.filter == adcontrols::threshold_method::DFT_Filter ) && 
               ( ( !ap::compare<double>::approximatelyEqual( hCutoffHz, m.hCutoffHz ) ) ||
                 ( !ap::compare<double>::approximatelyEqual( lCutoffHz, m.lCutoffHz ) ) ||
                 ( m.complex_ != complex_ ) ) ) ) {
            return true;
        }
    }

    return false;
}

const boost::uuids::uuid&
threshold_method::clsid()
{
    static boost::uuids::uuid baseid = boost::uuids::string_generator()( "{3D2F180E-18E9-43D3-9A37-9E981B509CAA}" );
    static const boost::uuids::uuid myclsid = boost::uuids::name_generator( baseid )( "adcontrols::threshold_method" );
    return myclsid;
}
