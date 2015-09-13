/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "waveform.hpp"
#include "threshold_result.hpp"
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adportable/asio/thread.hpp>
#include <adportable/binary_serializer.hpp>
#include <adportable/debug.hpp>
#include <adportable/serializer.hpp>
#include <adportable/spectrum_processor.hpp>
#include <adportable/timesquaredscanlaw.hpp>
#include <adicontroller/signalobserver.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/type_traits.hpp>
#include <boost/variant.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>

using namespace ap240controls;

identify::identify() : bus_number_( 0 )
                     , slot_number_( 0 )
                     , serial_number_( 0 )
{
}

identify::identify( const identify& t ) : bus_number_( t.bus_number_ )
                                        , slot_number_( t.slot_number_ )
                                        , serial_number_( t.serial_number_ )
{
}

waveform::waveform( const identify& id, uint32_t pos, uint32_t events, uint64_t tp ) : ident_( id )
                                                                                     , serialnumber_( pos )
                                                                                     , wellKnownEvents_( events )
                                                                                     , timeSinceEpoch_( tp )
{
}


size_t
waveform::size() const
{
    return method_.hor_.nbrSamples;
}

template<> const int8_t *
waveform::begin() const
{
    if ( meta_.dataType != sizeof(int8_t) )
        throw std::bad_cast();
    return reinterpret_cast< const int8_t* >( d_.data() ) + meta_.indexFirstPoint;
}

template<> const int8_t *
waveform::end() const
{
    if ( meta_.dataType != sizeof(int8_t) )
        throw std::bad_cast();    
    return reinterpret_cast< const int8_t* >( d_.data() ) + meta_.indexFirstPoint + method_.hor_.nbrSamples;
}

template<> const int16_t *
waveform::begin() const
{
    if ( meta_.dataType != sizeof(int16_t) )
        throw std::bad_cast();        
    return reinterpret_cast< const int16_t* >( d_.data() ) + meta_.indexFirstPoint;
}

template<> const int16_t *
waveform::end() const
{
    if ( meta_.dataType != sizeof(int16_t) )
        throw std::bad_cast();
    return reinterpret_cast< const int16_t* >( d_.data() ) + meta_.indexFirstPoint + method_.hor_.nbrSamples;
}

template<> const int32_t *
waveform::begin() const
{
    if ( meta_.dataType != sizeof(int32_t) )
        throw std::bad_cast();                
    return reinterpret_cast< const int32_t* >( d_.data() ) + meta_.indexFirstPoint;
}

template<> const int32_t *
waveform::end() const
{
    if ( meta_.dataType != sizeof(int32_t) )
        throw std::bad_cast();                    
    return reinterpret_cast< const int32_t* >( d_.data() ) + meta_.indexFirstPoint + method_.hor_.nbrSamples;
}

std::pair<double, int>
waveform::operator [] ( size_t idx ) const
{
    double time = idx * meta_.xIncrement + meta_.horPos;

    if ( method_.hor_.mode == 0 )
        time += method_.hor_.delay;
    else
        time += method_.hor_.nStartDelay * meta_.xIncrement;
    
    switch( meta_.dataType ) {
    case 1: return std::make_pair( time, *(begin<int8_t>()  + idx) );
    case 2: return std::make_pair( time, *(begin<int16_t>() + idx) );
    case 4: return std::make_pair( time, *(begin<int32_t>() + idx) );
    }
    throw std::exception();
}

double
waveform::toVolts( int d ) const
{
    if ( meta_.actualAverages == 0 )
        return meta_.scaleFactor * d - meta_.scaleOffset;
    else
        return double( meta_.scaleFactor * d ) / meta_.actualAverages - ( meta_.scaleOffset * meta_.actualAverages );
}

double
waveform::toVolts( double d ) const
{
    return d * meta_.scaleFactor /  meta_.actualAverages;
}

namespace ap240controls {

    template<typename T = identify>
    class identify_archive {
    public:
        template<class Archive> void serialize( Archive& ar, T& _, const unsigned int version ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( _.serial_number_ );
        }
    };

    template<> AP240CONTROLSSHARED_EXPORT void identify::serialize( boost::archive::xml_woarchive& ar, unsigned int version )
    {
        identify_archive<>().serialize( ar, *this, version );
    }

    template<> AP240CONTROLSSHARED_EXPORT void identify::serialize( boost::archive::xml_wiarchive& ar, unsigned int version )
    {
        identify_archive<>().serialize( ar, *this, version );
    }

    template<> AP240CONTROLSSHARED_EXPORT void identify::serialize( portable_binary_oarchive& ar, unsigned int version )
    {
        identify_archive<>().serialize( ar, *this, version );
    }

    template<> AP240CONTROLSSHARED_EXPORT void identify::serialize( portable_binary_iarchive& ar, unsigned int version )
    {
        identify_archive<>().serialize( ar, *this, version );
    }

    ///////////////////////

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
            ar & BOOST_SERIALIZATION_NVP( _.initialXTimeSeconds );
            ar & BOOST_SERIALIZATION_NVP( _.initialXOffset );
            ar & BOOST_SERIALIZATION_NVP( _.xIncrement );
            ar & BOOST_SERIALIZATION_NVP( _.scaleFactor );
            ar & BOOST_SERIALIZATION_NVP( _.scaleOffset );
            ar & BOOST_SERIALIZATION_NVP( _.horPos );
        }
    };

    template<> AP240CONTROLSSHARED_EXPORT void metadata::serialize( boost::archive::xml_woarchive& ar, unsigned int version )
    {
        metadata_archive<>().serialize( ar, *this, version );
    }

    template<> AP240CONTROLSSHARED_EXPORT void metadata::serialize( boost::archive::xml_wiarchive& ar, unsigned int version )
    {
        metadata_archive<>().serialize( ar, *this, version );
    }

    template<> AP240CONTROLSSHARED_EXPORT void metadata::serialize( portable_binary_oarchive& ar, unsigned int version )
    {
        metadata_archive<>().serialize( ar, *this, version );
    }

    template<> AP240CONTROLSSHARED_EXPORT void metadata::serialize( portable_binary_iarchive& ar, unsigned int version )
    {
        metadata_archive<>().serialize( ar, *this, version );
    }

    ////////////////////
    template<typename T = device_data>
    class device_data_archive {
    public:
        template<class Archive>
        void serialize( Archive& ar, T& _, const unsigned int version ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( _.ident_ );
            ar & BOOST_SERIALIZATION_NVP( _.meta_ );
        }
    };

    template<> AP240CONTROLSSHARED_EXPORT void device_data::serialize( boost::archive::xml_woarchive& ar, unsigned int version )
    {
        device_data_archive<>().serialize( ar, *this, version );
    }

    template<> AP240CONTROLSSHARED_EXPORT void device_data::serialize( boost::archive::xml_wiarchive& ar, unsigned int version )
    {
        device_data_archive<>().serialize( ar, *this, version );
    }

    template<> AP240CONTROLSSHARED_EXPORT void device_data::serialize( portable_binary_oarchive& ar, unsigned int version )
    {
        device_data_archive<>().serialize( ar, *this, version );
    }

    template<> AP240CONTROLSSHARED_EXPORT void device_data::serialize( portable_binary_iarchive& ar, unsigned int version )
    {
        device_data_archive<>().serialize( ar, *this, version );
    }

    ////////////////////

    class waveform_xmeta_archive {
    public:
        identify ident_;
        std::vector< metadata > meta_;
        std::shared_ptr< ap240controls::method > method_;
    private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize( Archive& ar, const unsigned int ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( ident_ );
            ar & BOOST_SERIALIZATION_NVP( meta_ );
            ar & BOOST_SERIALIZATION_NVP( method_ );
        }
    };

} // namespace

//static
std::array< std::shared_ptr< const waveform >, 2 >
waveform::deserialize( const adicontroller::SignalObserver::DataReadBuffer * rb )
{
    if ( rb ) {
        std::array< std::shared_ptr< waveform >, 2 > waveforms;

        auto self( rb->shared_from_this() );
        if ( !self || self->ndata() == 0 )
            return std::array< std::shared_ptr< const waveform >, 2 >();

        size_t nChannels = rb->ndata();

        waveform_xmeta_archive x;
        if ( adportable::binary::deserialize<>()( x, reinterpret_cast<const char *>( rb->xmeta().data() ), rb->xmeta().size() ) ) {
            
            for ( const auto& meta : x.meta_ ) {
                if ( meta.channel == 1 || meta.channel == 2 ) {
                    waveforms[ meta.channel - 1 ] = std::make_shared< waveform >( x.ident_, rb->pos(), rb->events(), rb->timepoint() );
                    waveforms[ meta.channel - 1 ]->meta_ = meta;
                    if ( x.method_ )
                        waveforms[ meta.channel - 1 ]->method_ = *x.method_;
                }
            }
            
            const uint32_t * pdata = reinterpret_cast<const uint32_t *>( rb->xdata().data() );
            for ( auto& waveform : waveforms ) {
                if ( *pdata == 0x7ffe0001 ) {
                    ++pdata;
                    uint32_t size = *pdata++;
                    if ( size && waveform ) {
                        const ap240controls::waveform::value_type * data_p = reinterpret_cast<const ap240controls::waveform::value_type *>( pdata );
                        std::copy( data_p, data_p + size, waveform->data( size ) );
                        pdata = reinterpret_cast<const uint32_t *>( data_p + size );
                    }
                }
            }
        }
        return { waveforms[ 0 ], waveforms[ 1 ] };
    }
    return { 0, 0 };// std::array< std::shared_ptr< const waveform >, 2 >();
}

//static
bool
waveform::serialize( adicontroller::SignalObserver::DataReadBuffer& rb
                     , std::shared_ptr< const waveform > ch1
                     , std::shared_ptr< const waveform > ch2 )
{
    rb.ndata() = 0;
    if ( ch1 || ch2 ) {
        const waveform& waveform = ch1 ? *ch1 : *ch2;

        rb.pos() = waveform.serialnumber_;
        rb.timepoint() = waveform.timeSinceEpoch_;

        waveform_xmeta_archive x;

        size_t data_octets( 4 * sizeof( uint32_t ) );
        
        if ( ch1 ) {
            rb.ndata()++;
            x.meta_.push_back( ch1->meta_ );
            data_octets += ch1->size() * sizeof( waveform::value_type );
        }
        
        if ( ch2 ) {
            rb.ndata()++;
            x.meta_.push_back( ch2->meta_ );
            data_octets += ch2->size() * sizeof( waveform::value_type );
        }

        { // serialize xmeta
            x.ident_ = waveform.ident_;
            x.method_ = std::make_shared< ap240controls::method >( waveform.method_ );
            std::ostringstream o;
            portable_binary_oarchive ar( o );
            ar << x;
            const std::string& device = o.str();
            rb.xmeta().resize( device.size() );
            std::copy( device.data(), device.data() + device.size(), rb.xmeta().data() );
        }
        
        rb.xdata().resize( data_octets );
        waveform::value_type * dest_p = reinterpret_cast<waveform::value_type *>( rb.xdata().data() );
        
        for ( auto& ptr : { ch1, ch2 } ) {
            
            uint32_t * size_p = reinterpret_cast<uint32_t *>( dest_p );
            size_p[ 0 ] = 0x7ffe0001; // separater & endian marker
            size_p[ 1 ] = ptr ? uint32_t( ptr->size() ) : 0;
            dest_p = reinterpret_cast<waveform::value_type *>( &size_p[ 2 ] );
            
            if ( ptr ) {
                std::copy( ptr->data(), ptr->data() + ptr->data_size(), dest_p );
                dest_p += ptr->size();
            }
        }
        return true;
    }
    return false;
}


//static
bool
waveform::translate( adcontrols::MassSpectrum& sp, const waveform& waveform, int scale )
{
    using namespace adcontrols::metric;

    sp.setCentroid( adcontrols::CentroidNone );
    
    adcontrols::MSProperty prop = sp.getMSProperty();
    adcontrols::MSProperty::SamplingInfo info( 0 /* sampInterval (ps) */
                                               , uint32_t( waveform.meta_.initialXOffset / waveform.meta_.xIncrement + 0.5 )
                                               , uint32_t( waveform.size() )
                                               , waveform.meta_.actualAverages
                                               , 0 /* mode */ );
    info.fSampInterval( waveform.meta_.xIncrement );
    prop.acceleratorVoltage( 3000 );
    prop.setSamplingInfo( info );
    
    prop.setTimeSinceInjection( uint64_t( scale_to_micro( waveform.meta_.initialXTimeSeconds ) ) );
    prop.setTimeSinceEpoch( waveform.timeSinceEpoch_ ); // nanoseconds
    prop.setDataInterpreterClsid( "ap240" );

    const device_data data( waveform.ident_, waveform.meta_ );
    std::string ar;
    adportable::binary::serialize<>()( data, ar );
    prop.setDeviceData( ar.data(), ar.size() );

    // prop.setDeviceData(); TBA
    sp.setMSProperty( prop );
    sp.resize( waveform.size() );
	int idx = 0;

    if ( waveform.meta_.dataType == 1 ) {
        if ( scale )
            for ( auto y = waveform.begin<int8_t>(); y != waveform.end<int8_t>(); ++y )
                sp.setIntensity( idx++, waveform.toVolts( *y ) * scale );
        else
            for ( auto y = waveform.begin<int8_t>(); y != waveform.end<int8_t>(); ++y )
                sp.setIntensity( idx++, *y );                
    } else {
        double dbase, rms;
        double tic = adportable::spectrum_processor::tic( waveform.size(), waveform.begin<int32_t>(), dbase, rms );

        if ( scale )
            for ( auto y = waveform.begin<int32_t>(); y != waveform.end<int32_t>(); ++y )
                sp.setIntensity( idx++, waveform.toVolts( *y - dbase ) * scale );
        else
            for ( auto y = waveform.begin<int32_t>(); y != waveform.end<int32_t>(); ++y )
                sp.setIntensity( idx++, *y - dbase );
        
    }

    // TBA: mass array, need scanlaw

	return true;
}

//static
bool
waveform::translate( adcontrols::MassSpectrum& sp, const threshold_result& result, int scale )
{
    using namespace adcontrols::metric;

    sp.setCentroid( adcontrols::CentroidNone );
    const waveform& waveform = *result.data();
    
    adcontrols::MSProperty prop = sp.getMSProperty();
    adcontrols::MSProperty::SamplingInfo info( 0 /* sampInterval (ps) */
                                               , uint32_t( waveform.meta_.initialXOffset / waveform.meta_.xIncrement + 0.5 )
                                               , uint32_t( waveform.size() )
                                               , waveform.meta_.actualAverages
                                               , 0 /* mode */ );
    info.fSampInterval( waveform.meta_.xIncrement );
    prop.acceleratorVoltage( 3000 );
    prop.setSamplingInfo( info );
    
    prop.setTimeSinceInjection( uint64_t( scale_to_micro( waveform.meta_.initialXTimeSeconds ) ) );
    prop.setTimeSinceEpoch( waveform.timeSinceEpoch_ ); // nanoseconds
    prop.setDataInterpreterClsid( "ap240" );

    const device_data data( waveform.ident_, waveform.meta_ );
    std::string ar;
    adportable::binary::serialize<>()( data, ar );
    prop.setDeviceData( ar.data(), ar.size() );

    // prop.setDeviceData(); TBA
    sp.setMSProperty( prop );
    sp.resize( waveform.size() );
	int idx = 0;

    if ( result.processed().size() == waveform.size() ) { // has filterd waveform
        if ( scale <= 1 )
            sp.setIntensityArray( result.processed().data() ); // return Volts (no binary avilable for processed waveform)
        else
            for ( auto it = result.processed().begin(); it != result.processed().end(); ++it )
                sp.setIntensity( idx++, *it * scale ); // Volts -> mV (where scale = 1000)
        
    } else if ( waveform.meta_.dataType == 1 ) {
        if ( scale )
            for ( auto y = waveform.begin<int8_t>(); y != waveform.end<int8_t>(); ++y )
                sp.setIntensity( idx++, waveform.toVolts( *y ) * scale );        // V, mV ...
        else
            for ( auto y = waveform.begin<int8_t>(); y != waveform.end<int8_t>(); ++y )
                sp.setIntensity( idx++, *y );          // binary 
    } else {
        double dbase, rms;
        double tic = adportable::spectrum_processor::tic( waveform.size(), waveform.begin<int32_t>(), dbase, rms );

        if ( scale )
            for ( auto y = waveform.begin<int32_t>(); y != waveform.end<int32_t>(); ++y )
                sp.setIntensity( idx++, waveform.toVolts( *y - dbase ) * scale ); // V, mV ...
        else
            for ( auto y = waveform.begin<int32_t>(); y != waveform.end<int32_t>(); ++y )
                sp.setIntensity( idx++, *y - dbase );  // binary
        
    }
    
	return true;
}

