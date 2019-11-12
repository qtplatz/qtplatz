/**************************************************************************
** Copyright (C) 2010-2020 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2020 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include <adportable/portable_binary_iarchive.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <adportable/float.hpp>
#include <compiler/boost/workaround.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/archive/archive_exception.hpp>
#include <boost/uuid/uuid_generators.hpp>

namespace aqmd3controls {

        template<typename T>
        class method_archive {
        public:
            template<class Archive>
            void serialize( Archive& ar, T& _, const unsigned int version ) {

                using namespace boost::serialization;

                ar & BOOST_SERIALIZATION_NVP( _.channels_ );
                ar & BOOST_SERIALIZATION_NVP( _.mode_ );
                ar & BOOST_SERIALIZATION_NVP( _.device_method_ );
                ar & BOOST_SERIALIZATION_NVP( _.protocolIndex_ );
                ar & BOOST_SERIALIZATION_NVP( _.protocols_ );
            }

        };

        template<> AQMD3CONTROLSSHARED_EXPORT void method::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
        {
            method_archive<>().serialize( ar, *this, version );
        }

        template<> AQMD3CONTROLSSHARED_EXPORT void method::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
        {
            method_archive<>().serialize( ar, *this, version );
        }

        template<> AQMD3CONTROLSSHARED_EXPORT void method::serialize( portable_binary_oarchive& ar, const unsigned int version )
        {
            method_archive<>().serialize( ar, *this, version );
        }

        template<> AQMD3CONTROLSSHARED_EXPORT void method::serialize( portable_binary_iarchive& ar, const unsigned int version )
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

using namespace aqmd3controls;

method:: method() : channels_( 0x01 )
                  , mode_( DigiMode::Digitizer ) // digitizer mode
                  , protocolIndex_( 0 )
                  , protocols_( 1 ) // at lease one protocol data should be exist
{
}

method:: method( const method& t ) : channels_( t.channels_ )
                                   , mode_( t.mode_ )
                                   , device_method_( t.device_method_ )
                                   , protocolIndex_( t.protocolIndex_ )
                                   , protocols_( t.protocols_ )
{
}

const boost::uuids::uuid&
method::clsid()
{
    // 82400772-b4e4-4756-8c38-b1d9dd1092cb
    static const boost::uuids::uuid myclsid = {{ 0x82, 0x40, 0x07, 0x72, 0xb4, 0xe4, 0x47, 0x56, 0x8c, 0x38, 0xb1, 0xd9, 0xdd, 0x10, 0x92, 0xcb }};
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

method::DigiMode
method::mode() const
{
    // 0 := digitizer, 2 := averager
    return mode_;
}

void
method::setMode( DigiMode mode )
{
    mode_ = mode;
}

const aqmd3controls::device_method&
method::device_method() const
{
    return device_method_;
}

aqmd3controls::device_method&
method::device_method()
{
    return device_method_;
}

uint32_t
method::protocolIndex() const
{
    return protocolIndex_;
}

bool
method::setProtocolIndex( uint32_t value , bool modifyDeviceMethod )
{
    protocolIndex_ = value;

    bool dirty( false );
    if ( modifyDeviceMethod && protocolIndex_ < protocols_.size() ) {

        const auto& proto = protocols_ [ protocolIndex_ ];

        if ( !adportable::compare<double>::essentiallyEqual( device_method_.delay_to_first_sample_, proto.digitizerDelayWidth().first ) ) {
            dirty = true;
            device_method_.delay_to_first_sample_ = proto.digitizerDelayWidth().first;
        }

        auto nbrSamples = uint32_t( proto.digitizerDelayWidth().second * device_method_.samp_rate + 0.5 );
        if ( device_method_.nbr_of_s_to_acquire_ != nbrSamples ) {
            dirty = true;
            device_method_.nbr_of_s_to_acquire_ = nbrSamples;
        }
    }
    return dirty;
}
