/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include <adportable/float.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>
#include <adportable/portable_binary_iarchive.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/uuid/uuid_generators.hpp>

using namespace acqrscontrols::ap240;

const boost::uuids::uuid&
method::clsid()
{
    static boost::uuids::uuid baseid = boost::uuids::string_generator()( "{3D2F180E-18E9-43D3-9A37-9E981B509CAA}" );
    static const boost::uuids::uuid myclsid = boost::uuids::name_generator( baseid )( "acqrscontrols::ap240::method" );
    return myclsid;
}

method::method() : channels_( 0x01 )
                 , protocolIndex_( 0 )
                 , protocols_( 1 ) // at least one protocol data should be exist
{
    ch1_.set_enable( bool( channels_ & 01 ) );
    ch2_.set_enable( bool( channels_ & 02 ) );
}

method::method( const method& t ) : channels_( t.channels_ )
                                  , hor_( t.hor_ )
                                  , trig_( t.trig_ )
                                  , ext_( t.ext_ )
                                  , ch1_( t.ch1_ )
                                  , ch2_( t.ch2_ )
                                  , slope1_( t.slope1_ )
                                  , slope2_( t.slope2_ )
                                  , action_( t.action_ )
                                  , protocolIndex_( t.protocolIndex_ )
                                  , protocols_( t.protocols_ ) 
{
}

method::DigiMode
method::mode() const
{
    return DigiMode( hor_.mode );
}

uint32_t
method::protocolIndex() const
{
    return protocolIndex_;
}

bool
method::setProtocolIndex( uint32_t value, bool modifyDeviceMethod )
{
    protocolIndex_ = value;
    bool dirty( false );
    if ( modifyDeviceMethod && protocolIndex_ < protocols_.size() ) {

        const auto& proto = protocols_ [ protocolIndex_ ];

        if ( !adportable::compare<double>::essentiallyEqual( hor_.delayTime, proto.digitizerDelayWidth().first ) ) {
            dirty = true;
            hor_.delayTime = proto.digitizerDelayWidth().first;
        }

        auto nbrSamples = uint32_t( proto.digitizerDelayWidth().second * hor_.sampInterval + 0.5 );
        if ( hor_.nbrSamples != nbrSamples ) {
            dirty = true;
            hor_.nbrSamples = nbrSamples;
        }
    }
    return dirty;
}

std::vector< adcontrols::TofProtocol >&
method::protocols()
{
    return protocols_;
}

const std::vector< adcontrols::TofProtocol >&
method::protocols() const
{
    return protocols_;
}

namespace acqrscontrols {
    namespace ap240 {

        ///////////////////////////////////////
        template<typename T = method>
        class method_archive {
        public:
            template<class Archive>
            void serialize( Archive& ar, T& _, const unsigned int version ) {
                using namespace boost::serialization;
                ar & BOOST_SERIALIZATION_NVP( _.channels_ );
                ar & BOOST_SERIALIZATION_NVP( _.trig_ );
                ar & BOOST_SERIALIZATION_NVP( _.hor_ );
                ar & BOOST_SERIALIZATION_NVP( _.ext_ );
                ar & BOOST_SERIALIZATION_NVP( _.ch1_ );
                ar & BOOST_SERIALIZATION_NVP( _.ch2_ );
                ar & BOOST_SERIALIZATION_NVP( _.slope1_ );
                ar & BOOST_SERIALIZATION_NVP( _.slope2_ );
                if ( version >= 1 )
                    ar & BOOST_SERIALIZATION_NVP( _.action_ );
                if ( version >= 2 ) {
                    ar & BOOST_SERIALIZATION_NVP( _.protocolIndex_ );
                    ar & BOOST_SERIALIZATION_NVP( _.protocols_ );
                }
                if ( Archive::is_loading::value ) {
                    _.ch1_.set_enable( bool( _.channels_ & 01 ) );
                    _.ch2_.set_enable( bool( _.channels_ & 02 ) );
                    if ( _.protocols_.empty() )
                        _.protocols_.resize(1);
                }
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

