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
#include <boost/archive/archive_exception.hpp>
#include <boost/uuid/uuid_generators.hpp>

namespace acqrscontrols {
    namespace u5303a {

        template<typename T>
        class method_archive {
        public:
            template<class Archive>
            void serialize( Archive& ar, T& _, const unsigned int version ) {

                if ( version < 4 )
                    throw boost::archive::archive_exception( boost::archive::archive_exception::unsupported_version );
                
                using namespace boost::serialization;
                
                ar & BOOST_SERIALIZATION_NVP( _.channels_ );
                ar & BOOST_SERIALIZATION_NVP( _.mode_ );
                ar & BOOST_SERIALIZATION_NVP( _.method_ );
                if ( version < 6 ) {
                    // follwing items has been removed since v6
                    adcontrols::threshold_method threshold;
                    adcontrols::threshold_action action;
                    ar & BOOST_SERIALIZATION_NVP( threshold );
                    ar & BOOST_SERIALIZATION_NVP( action );
                }
                if ( version >= 7 ) {
                    ar & BOOST_SERIALIZATION_NVP( _.protocolIndex_ );
                    ar & BOOST_SERIALIZATION_NVP( _.protocols_ );
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

        template<> ACQRSCONTROLSSHARED_EXPORT void method::serialize( portable_binary_oarchive& ar, const unsigned int version )
        {
            method_archive<>().serialize( ar, *this, version );
        }

        template<> ACQRSCONTROLSSHARED_EXPORT void method::serialize( portable_binary_iarchive& ar, const unsigned int version )
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
                  , mode_( 0 ) // digitizer mode
                  , protocolIndex_( 0 )
                  , protocols_( 1 ) // at lease one protocol data should be exist
{
}

method:: method( const method& t ) : channels_( t.channels_ )
                                   , mode_( t.mode_ )
                                   , method_( t.method_ )
                                   , protocolIndex_( t.protocolIndex_ )
                                   , protocols_( t.protocols_ )
{
}

const boost::uuids::uuid&
method::clsid()
{
    static boost::uuids::uuid baseid = boost::uuids::string_generator()( "{3D2F180E-18E9-43D3-9A37-9E981B509CAA}" );
    static const boost::uuids::uuid myclsid = boost::uuids::name_generator( baseid )( "acqrscontrols::u5303a::method" );
    return myclsid;
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

uint32_t
method::channels() const
{
    return channels_;
}

void
method::setChannels( uint32_t channels )
{
    channels_ = channels;
}

uint32_t
method::mode() const
{
    // 0 := digitizer, 2 := averager
    return mode_;
}

void
method::setMode( uint32_t mode )
{
    mode_ = mode;
}

const device_method&
method::device_method() const
{
    return method_;
}

device_method&
method::device_method()
{
    return method_;
}

uint32_t
method::protocolIndex() const
{
    return protocolIndex_;
}

void
method::setProtocolIndex( uint32_t value )
{
    protocolIndex_ = value;
}

