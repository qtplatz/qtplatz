/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
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

#include "threshold_action.hpp"

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>
#include <adportable/portable_binary_iarchive.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/uuid/uuid_generators.hpp>

namespace adcontrols {

    template< typename T = threshold_action >
    class threshold_action_archive {
    public:
        template<class Archive>
        void serialize( Archive& ar, T& _, const unsigned int version ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( _.enable );
            ar & BOOST_SERIALIZATION_NVP( _.enableTimeRange );
            ar & BOOST_SERIALIZATION_NVP( _.delay );
            ar & BOOST_SERIALIZATION_NVP( _.width );
            ar & BOOST_SERIALIZATION_NVP( _.recordOnFile );
            ar & BOOST_SERIALIZATION_NVP( _.exclusiveDisplay );
            if ( version >= 2 ) {
                ar & BOOST_SERIALIZATION_NVP( _.objid_spectrometer );
                ar & BOOST_SERIALIZATION_NVP( _.formula );
                ar & BOOST_SERIALIZATION_NVP( _.mode );
                ar & BOOST_SERIALIZATION_NVP( _.mass );
            }
        }
    };

    template<> ADCONTROLSSHARED_EXPORT void threshold_action::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
    {
        threshold_action_archive<>().serialize( ar, *this, version );
    }

    template<> ADCONTROLSSHARED_EXPORT void threshold_action::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        threshold_action_archive<>().serialize( ar, *this, version );
    }

    template<> ADCONTROLSSHARED_EXPORT void threshold_action::serialize( portable_binary_oarchive& ar, const unsigned int version )
    {
        threshold_action_archive<>().serialize( ar, *this, version );
    }

    template<> ADCONTROLSSHARED_EXPORT void threshold_action::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        threshold_action_archive<>().serialize( ar, *this, version );
    }

    bool threshold_action::archive( std::ostream& os, const threshold_action& t )
    {
        try {
            portable_binary_oarchive ar( os );
            ar & boost::serialization::make_nvp( "m", t );
            return true;
        } catch ( std::exception& ) {
        }
        return false;
    }

    bool threshold_action::restore( std::istream& is, threshold_action& t )
    {
        try {
            portable_binary_iarchive ar( is );
            ar & boost::serialization::make_nvp( "m", t );
            return true;
        } catch ( std::exception& ) {
        }
        return false;
    }

    bool threshold_action::xml_archive( std::wostream& os, const threshold_action& t )
    {
        try {
            boost::archive::xml_woarchive ar( os );
            ar & boost::serialization::make_nvp( "m", t );
            return true;
        } catch ( std::exception& ) {
        }
        return false;
    }

    bool threshold_action::xml_restore( std::wistream& is, threshold_action& t )
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

using namespace adcontrols;

threshold_action::~threshold_action(void)
{
}

threshold_action::threshold_action() : delay( 0.0 )
                                     , width( 1.0e-6 )
                                     , enable( false )
                                     , enableTimeRange( false )
                                     , recordOnFile( true )
                                     , exclusiveDisplay( true )
{
}

threshold_action::threshold_action( const threshold_action& t ) : delay( t.delay )
                                                          , width( t.width )
                                                          , enable( t.enable )
                                                          , enableTimeRange( t.enableTimeRange )
                                                          , recordOnFile( t.recordOnFile )
                                                          , exclusiveDisplay( t.exclusiveDisplay )
{
}

const boost::uuids::uuid&
threshold_action::clsid()
{
    static boost::uuids::uuid baseid = boost::uuids::string_generator()( "{3D2F180E-18E9-43D3-9A37-9E981B509CAA}" );
    static const boost::uuids::uuid myclsid = boost::uuids::name_generator( baseid )( "adcontrols::threshold_action" );
    return myclsid;
}
