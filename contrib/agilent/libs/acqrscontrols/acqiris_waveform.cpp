/**************************************************************************
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com or info@ms-cheminfo.com
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

#include "acqiris_waveform.hpp"
#include <typeinfo>
#include <cstring>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_serialize.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <adportable/portable_binary_iarchive.hpp>

namespace boost {
    namespace serialization {

        template<class Archive>
        void serialize( Archive & ar, AqDataDescriptor& _, const unsigned int version )
        {
            ar & BOOST_SERIALIZATION_NVP( _.returnedSamplesPerSeg );
            ar & BOOST_SERIALIZATION_NVP( _.indexFirstPoint );    //!< 'data[desc.indexFirstPoint]' is the first valid point. 
            ar & BOOST_SERIALIZATION_NVP( _.sampTime );
            ar & BOOST_SERIALIZATION_NVP( _.vGain );
            ar & BOOST_SERIALIZATION_NVP( _.vOffset );
            ar & BOOST_SERIALIZATION_NVP( _.returnedSegments );   //!< When reading multiple segments in one waveform
            ar & BOOST_SERIALIZATION_NVP( _.nbrAvgWforms );        
            ar & BOOST_SERIALIZATION_NVP( _.actualTriggersInAcqLo );
            ar & BOOST_SERIALIZATION_NVP( _.actualTriggersInAcqHi );
            ar & BOOST_SERIALIZATION_NVP( _.actualDataSize );
            ar & BOOST_SERIALIZATION_NVP( _.reserved2 );    
            ar & BOOST_SERIALIZATION_NVP( _.reserved3 );
        }

        template<class Archive>
        void serialize( Archive & ar, AqSegmentDescriptor& _, const unsigned int version )
        {
            ar & BOOST_SERIALIZATION_NVP( _.horPos );
            ar & BOOST_SERIALIZATION_NVP( _.timeStampLo );
            ar & BOOST_SERIALIZATION_NVP( _.timeStampHi );
        }
        
    }
}
        
namespace acqrscontrols {
namespace aqdrv4 {
    
    template< typename T = waveform >
    struct waveform_archive {
        template<class Archive>
        void serialize( Archive& ar, T& _, const unsigned int version ) {
            using namespace boost::serialization;
        
            ar & BOOST_SERIALIZATION_NVP( _.serialnumber_ );
            ar & BOOST_SERIALIZATION_NVP( _.serialnumber0_ );
            ar & BOOST_SERIALIZATION_NVP( _.timeSinceEpoch_ );
            ar & BOOST_SERIALIZATION_NVP( _.timeSinceInject_ );
            ar & BOOST_SERIALIZATION_NVP( _.wellKnownEvents_ );
            ar & BOOST_SERIALIZATION_NVP( _.dataType_ );
            ar & BOOST_SERIALIZATION_NVP( _.methodNumber_ );
            ar & BOOST_SERIALIZATION_NVP( _.delayTime_ );
            ar & BOOST_SERIALIZATION_NVP( _.dataDesc_ );
            ar & BOOST_SERIALIZATION_NVP( _.segDesc_ );
            ar & BOOST_SERIALIZATION_NVP( _.d_ );
        }
    };

    template<> void waveform::serialize( portable_binary_oarchive& ar, const unsigned int version )
    {
        waveform_archive<>().serialize( ar, *this, version );
    }

    template<> void waveform::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        waveform_archive<>().serialize( ar, *this, version );
    }


    const boost::uuids::uuid&
    waveform::clsid()
    {
        static auto __clsid = boost::uuids::string_generator()( "{33f5bfd8-793c-11e6-9bd0-1b94f4251234}" );
        return __clsid;
    }

    waveform::waveform( int32_t dataType ) : serialnumber_( 0 )
                                           , wellKnownEvents_( 0 )
                                           , dataType_( dataType )
    {
        std::memset( &dataDesc_, 0, sizeof( dataDesc_ ) );
        std::memset( &segDesc_, 0, sizeof( segDesc_ ) );
    }

    size_t
    waveform::size() const
    {
        return dataDesc_.returnedSamplesPerSeg;
    }

    template<> const int8_t *
    waveform::begin() const
    {
        if ( dataType_ != sizeof(int8_t) )
            throw std::bad_cast();
        return reinterpret_cast< const int8_t* >( d_.data() ) + dataDesc_.indexFirstPoint;
    }

    template<> const int8_t *
    waveform::end() const
    {
        return reinterpret_cast< const int8_t* >( d_.data() ) + dataDesc_.indexFirstPoint + dataDesc_.returnedSamplesPerSeg;
    }

    template<> const int16_t *
    waveform::begin() const
    {
        if ( dataType_ != sizeof(int16_t) )
            throw std::bad_cast();        
        return reinterpret_cast< const int16_t* >( d_.data() ) + dataDesc_.indexFirstPoint;
    }

    template<> const int16_t *
    waveform::end() const
    {
        if ( dataType_ != sizeof(int16_t) )
            throw std::bad_cast();
        return reinterpret_cast< const int16_t* >( d_.data() ) + dataDesc_.indexFirstPoint + dataDesc_.returnedSamplesPerSeg;
    }

    template<> const int32_t *
    waveform::begin() const
    {
        if ( dataType_ != sizeof(int32_t) )
            throw std::bad_cast();        
        return reinterpret_cast< const int32_t* >( d_.data() ) + dataDesc_.indexFirstPoint;
    }

    template<> const int32_t *
    waveform::end() const
    {
        if ( dataType_ != sizeof(int32_t) )
            throw std::bad_cast();
        return reinterpret_cast< const int32_t* >( d_.data() ) + dataDesc_.indexFirstPoint + dataDesc_.returnedSamplesPerSeg;
    }

    waveform::value_type *
    waveform::data( size_t size )
    {
        d_.resize( size );
        return d_.data();
    }

    const waveform::value_type *
    waveform::data() const
    {
        return d_.data();
    }

    size_t
    waveform::data_size() const
    {
        return d_.size();
    }

    template<> const int8_t *
    waveform::data() const
    {
        if ( dataType_ != sizeof(int8_t) )
            throw std::bad_cast();        
        return reinterpret_cast< const int8_t* >( d_.data() ) + dataDesc_.indexFirstPoint;
    }

    template<> int8_t *
    waveform::data()
    {
        if ( dataType_ != sizeof(int8_t) )
            throw std::bad_cast();        
        return reinterpret_cast< int8_t* >( d_.data() ) + dataDesc_.indexFirstPoint;
    }

    template<> const int16_t *
    waveform::data() const
    {
        if ( dataType_ != sizeof(int16_t) )
            throw std::bad_cast();        
        return reinterpret_cast< const int16_t* >( d_.data() ) + dataDesc_.indexFirstPoint;
    }

    template<> int16_t *
    waveform::data()
    {
        if ( dataType_ != sizeof(int16_t) )
            throw std::bad_cast();        
        return reinterpret_cast< int16_t* >( d_.data() ) + dataDesc_.indexFirstPoint;
    }

    template<> const int32_t *
    waveform::data() const
    {
        if ( dataType_ != sizeof(int32_t) )
            throw std::bad_cast();        
        return reinterpret_cast< const int32_t* >( d_.data() ) + dataDesc_.indexFirstPoint;
    }

    template<> int32_t *
    waveform::data()
    {
        if ( dataType_ != sizeof(int32_t) )
            throw std::bad_cast();        
        return reinterpret_cast< int32_t* >( d_.data() ) + dataDesc_.indexFirstPoint;
    }

    int
    waveform::dataType() const
    {
        return dataType_;
    }

    int
    waveform::operator [] ( size_t idx ) const
    {
        switch( dataType_ ) {
        case 1: return *(begin<int8_t>()  + idx);
        case 2: return *(begin<int16_t>() + idx);
        case 4: return *(begin<int32_t>() + idx);
        }
        throw std::bad_cast();
    }

}
}
