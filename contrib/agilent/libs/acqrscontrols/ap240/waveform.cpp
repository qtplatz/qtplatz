/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include "../acqiris_waveform.hpp"
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/samplinginfo.hpp>
#include <adcontrols/waveform_filter.hpp>
#include <adportable/asio/thread.hpp>
#include <adportable/binary_serializer.hpp>
#include <adportable/debug.hpp>
#include <adportable/float.hpp>
#include <adportable/mblock.hpp>
#include <adportable/serializer.hpp>
#include <adportable/spectrum_processor.hpp>
#include <adportable/timesquaredscanlaw.hpp>
#include <adportable/waveform_wrapper.hpp>
#include <adicontroller/signalobserver.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/bind.hpp>
#include <boost/exception/all.hpp>
#include <boost/format.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/type_traits.hpp>
#include <boost/variant.hpp>


using namespace acqrscontrols::ap240;

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

waveform::waveform( const identify& id
                    , uint32_t pos, uint32_t events, uint64_t tp, uint32_t pos0 ) : serialnumber_origin_( pos0 )
                                                                                  , serialnumber_( pos )
                                                                                  , wellKnownEvents_( events )
                                                                                  , firstValidPoint_( 0 )
                                                                                  , timeSinceEpoch_( tp )
                                                                                  , ident_( id )
{
}

waveform::waveform() : serialnumber_( 0 )
                     , wellKnownEvents_( 0 )
                     , firstValidPoint_( 0 )
                     , timeSinceEpoch_( 0 )
                     , timeSinceInject_( 0.0 )
{
}

waveform::waveform( std::shared_ptr< const identify > id
                    , uint32_t pos, uint32_t events, uint64_t tp ) : serialnumber_( pos )
                                                                   , wellKnownEvents_( events )
                                                                   , firstValidPoint_( 0 )
                                                                   , timeSinceEpoch_( tp )
                                                                   , timeSinceInject_( 0.0 )
                                                                   , ident_( *id )
{
}

waveform::waveform( const method& method
                    , const metadata& meta
                    , uint32_t serialnumber
                    , uint32_t wellKnownEvents
                    , uint64_t timeSinceEpoch
                    , uint64_t firstValidPoint
                    , double timeSinceInject
                    , const std::shared_ptr< const identify >& id
                    , std::unique_ptr< int32_t [] >&& xdata
                    , size_t size
                    , bool invert ) : method_( method )
                                    , meta_( meta )
                                    , serialnumber_( serialnumber )
                                    , wellKnownEvents_( wellKnownEvents )
                                    , firstValidPoint_( firstValidPoint )                                      
                                    , timeSinceEpoch_( timeSinceEpoch )
                                    , timeSinceInject_( timeSinceInject )
                                    , ident_( *id )
                                    , d_( size )
{
    typedef int32_t value_type;

    meta_.dataType = sizeof( value_type );

    auto p = this->template data< value_type >();    
    if ( invert ) {
        // std::transform( p, p + size, p, std::negate<value_type>() );
        std::transform( xdata.get(), xdata.get() + size, p, std::negate<value_type>() );
    } else {
        std::copy( xdata.get(), xdata.get() + size, p );
    }
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
    if ( meta_.dataType != sizeof(int8_t) )
        throw std::bad_cast();        
    return reinterpret_cast< const int8_t* >( d_.data() ) + meta_.indexFirstPoint;
    // if ( mblock_.which() == 2 ) {
    //     auto&& mblk = boost::get < std::shared_ptr< adportable::mblock<int8_t> > >( mblock_ );
    //     return mblk->data() + firstValidPoint_;
    // }
    // throw std::bad_cast();
}

template<> int8_t *
waveform::data()
{
    if ( meta_.dataType != sizeof(int8_t) )
        throw std::bad_cast();        
    return reinterpret_cast< int8_t* >( d_.data() ) + meta_.indexFirstPoint;
    // if ( mblock_.which() == 2 ) {
    //     auto&& mblk = boost::get < std::shared_ptr< adportable::mblock<int8_t> > >( mblock_ );
    //     return mblk->data() + firstValidPoint_;
    // }
    throw std::bad_cast();    
}

template<> const int16_t *
waveform::data() const
{
    if ( meta_.dataType != sizeof(int16_t) )
        throw std::bad_cast();        
    return reinterpret_cast< const int16_t* >( d_.data() ) + meta_.indexFirstPoint;
    // if ( mblock_.which() == 1 ) {
    //     auto&& mblk = boost::get < std::shared_ptr< adportable::mblock<int16_t> > >( mblock_ );
    //     return mblk->data() + firstValidPoint_;
    // }
    // throw std::bad_cast();
}

template<> int16_t *
waveform::data()
{
    if ( meta_.dataType != sizeof(int16_t) )
        throw std::bad_cast();        
    return reinterpret_cast< int16_t* >( d_.data() ) + meta_.indexFirstPoint;
}

template<> const int32_t *
waveform::data() const
{
    if ( meta_.dataType != sizeof(int32_t) )
        throw std::bad_cast();        
    return reinterpret_cast< const int32_t* >( d_.data() ) + meta_.indexFirstPoint;
}

template<> int32_t *
waveform::data()
{
    if ( meta_.dataType != sizeof(int32_t) )
        throw std::bad_cast();        
    return reinterpret_cast< int32_t* >( d_.data() ) + meta_.indexFirstPoint;
}

int
waveform::dataType() const
{
    return meta_.dataType;
}

int64_t
waveform::operator [] ( size_t idx ) const
{
    // double time = idx * meta_.xIncrement + meta_.horPos + meta_.initialXOffset;    

    switch( meta_.dataType ) {
    case 1: return *(begin<int8_t>()  + idx);
    case 2: return *(begin<int16_t>() + idx);
    case 4: return *(begin<int32_t>() + idx);
    }
    throw std::exception();
}

std::pair<double, int>
waveform::xy( size_t idx ) const
{
    double time = idx * meta_.xIncrement + meta_.horPos + meta_.initialXOffset;

    switch( meta_.dataType ) {
    case 1: return std::make_pair( time, *(begin<int8_t>()  + idx) );
    case 2: return std::make_pair( time, *(begin<int16_t>() + idx) );
    case 4: return std::make_pair( time, *(begin<int32_t>() + idx) );
    }
    throw std::exception();
}

double
waveform::toVolts( int32_t d ) const
{
    if ( meta_.actualAverages == 0 )
        return meta_.scaleFactor * d - meta_.scaleOffset;
    else
        return double( meta_.scaleFactor * d ) / meta_.actualAverages - meta_.scaleOffset;
}

double
waveform::toVolts( int64_t d ) const
{
    if ( meta_.actualAverages == 0 )
        return meta_.scaleFactor * d - meta_.scaleOffset;
    else
        return double( meta_.scaleFactor * d ) / meta_.actualAverages - meta_.scaleOffset;
}

double
waveform::toVolts( double d ) const
{
    return d * meta_.scaleFactor /  meta_.actualAverages;
}

namespace acqrscontrols {
    namespace ap240 {

        template<typename T = identify>
        class identify_archive {
        public:
            template<class Archive> void serialize( Archive& ar, T& _, const unsigned int version ) {
                using namespace boost::serialization;
                ar & BOOST_SERIALIZATION_NVP( _.serial_number_ );
            }
        };

        template<> ACQRSCONTROLSSHARED_EXPORT void identify::serialize( boost::archive::xml_woarchive& ar, unsigned int version )
        {
            identify_archive<>().serialize( ar, *this, version );
        }

        template<> ACQRSCONTROLSSHARED_EXPORT void identify::serialize( boost::archive::xml_wiarchive& ar, unsigned int version )
        {
            identify_archive<>().serialize( ar, *this, version );
        }

        template<> ACQRSCONTROLSSHARED_EXPORT void identify::serialize( portable_binary_oarchive& ar, unsigned int version )
        {
            identify_archive<>().serialize( ar, *this, version );
        }

        template<> ACQRSCONTROLSSHARED_EXPORT void identify::serialize( portable_binary_iarchive& ar, unsigned int version )
        {
            identify_archive<>().serialize( ar, *this, version );
        }

        ///////////////////////

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

        template<> ACQRSCONTROLSSHARED_EXPORT void device_data::serialize( boost::archive::xml_woarchive& ar, unsigned int version )
        {
            device_data_archive<>().serialize( ar, *this, version );
        }

        template<> ACQRSCONTROLSSHARED_EXPORT void device_data::serialize( boost::archive::xml_wiarchive& ar, unsigned int version )
        {
            device_data_archive<>().serialize( ar, *this, version );
        }

        template<> ACQRSCONTROLSSHARED_EXPORT void device_data::serialize( portable_binary_oarchive& ar, unsigned int version )
        {
            device_data_archive<>().serialize( ar, *this, version );
        }

        template<> ACQRSCONTROLSSHARED_EXPORT void device_data::serialize( portable_binary_iarchive& ar, unsigned int version )
        {
            device_data_archive<>().serialize( ar, *this, version );
        }

        ////////////////////
        template<typename T = waveform >
        class waveform_xmeta_archive_t {
            T& _;
        public:
            waveform_xmeta_archive_t( T& t ) : _( t ) {}
            template<class Archive>
            void serialize( Archive& ar, const unsigned int version ) {
                using namespace boost::serialization;
                ar & BOOST_SERIALIZATION_NVP( _.ident_ );
                ar & BOOST_SERIALIZATION_NVP( _.meta_ );
                ar & BOOST_SERIALIZATION_NVP( _.method_ );                
                ar & BOOST_SERIALIZATION_NVP( _.timeSinceInject_ );
                ar & BOOST_SERIALIZATION_NVP( _.wellKnownEvents_ );
                if ( version >= 1 ) {
                    ar & BOOST_SERIALIZATION_NVP( _.serialnumber_origin_ );
                    ar & BOOST_SERIALIZATION_NVP( _.serialnumber_ );
                    ar & BOOST_SERIALIZATION_NVP( _.firstValidPoint_ );
                    ar & BOOST_SERIALIZATION_NVP( _.timeSinceEpoch_ );
                }
            }
        };

                    
        ////////////////////
        // serializer for stream (for file io)
        template<typename T = waveform >
        class waveform_xdata_archive_t {
            T& _;
        public:
            waveform_xdata_archive_t( T& t ) : _( t ) {}
            template<class Archive>
            void serialize( Archive& ar, const unsigned int version ) {
                using namespace boost::serialization;
                ar & BOOST_SERIALIZATION_NVP( _.d_ );
            }
        };
        

        class waveform_xmeta_archive {
            waveform_xmeta_archive( const waveform_xmeta_archive& ) = delete;
            waveform_xmeta_archive& operator = ( const waveform_xmeta_archive& ) = delete;
        public:
            waveform_xmeta_archive() {}
            ~waveform_xmeta_archive()
            {}

            identify ident_;
            std::vector< metadata > meta_;
            std::shared_ptr< acqrscontrols::ap240::method > method_;
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
}

// **** BOOST_CLASS_VERSION( T, N ) *****
namespace boost { namespace serialization {
        using namespace acqrscontrols::ap240;
        template< typename T > struct version< waveform_xmeta_archive_t< T > > { BOOST_STATIC_CONSTANT( int, value = 1 ); };
        template< typename T > struct version< waveform_xdata_archive_t< T > > { BOOST_STATIC_CONSTANT( int, value = 1 ); };
    }
}

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
                        const acqrscontrols::ap240::waveform::value_type * data_p = reinterpret_cast<const acqrscontrols::ap240::waveform::value_type *>( pdata );
                        std::copy( data_p, data_p + size, waveform->data( size ) );
                        pdata = reinterpret_cast<const uint32_t *>( data_p + size );
                    }
                }
            }
        }
        return std::array< std::shared_ptr< const waveform >, 2 >( {{ waveforms[ 0 ], waveforms[ 1 ] }} );
    }
    return std::array< std::shared_ptr< const waveform >, 2 >();
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

        const size_t data_count = ( ch1 ? ch1->data_size() : 0 ) + ( ch2 ? ch2->data_size() : 0 );
        if ( ch1 )
            x.meta_.push_back( ch1->meta_ );
        if ( ch2 )
            x.meta_.push_back( ch2->meta_ );

        { // serialize xmeta
            x.ident_ = waveform.ident_;
            x.method_ = std::make_shared< acqrscontrols::ap240::method >( waveform.method_ );
            std::ostringstream o;
            portable_binary_oarchive ar( o );
            ar << x;
            const std::string& device = o.str();
            rb.xmeta().resize( device.size() );
            std::copy( device.data(), device.data() + device.size(), rb.xmeta().data() );
        }
        
        rb.xdata().resize( ( data_count + 4 ) * sizeof( int32_t ) );
        int32_t * dest_p = reinterpret_cast<int32_t *>( rb.xdata().data() );
        
        for ( auto& ptr : { ch1, ch2 } ) {
            *dest_p++ = 0x7ffe0001; // separater & endian marker
            *dest_p++ = ptr ? int32_t( ptr->data_size() ) : 0;
            if ( ptr ) {
                rb.ndata()++;
                std::copy( ptr->d_.begin(), ptr->d_.end(), dest_p );
                dest_p += ptr->data_size();
            }
        }
        return true;
    }
    return false;
}

bool
waveform::translate_property( adcontrols::MassSpectrum& sp, const waveform& waveform )
{
    using namespace adcontrols::metric;

    sp.setCentroid( adcontrols::CentroidNone );
    
    adcontrols::MSProperty prop = sp.getMSProperty();
    double zhalf = waveform.meta_.initialXOffset < 0 ? (-0.5) : 0.5;
    adcontrols::SamplingInfo info( waveform.meta_.xIncrement
                                   , waveform.meta_.initialXOffset
                                   , int32_t( waveform.meta_.initialXOffset / waveform.meta_.xIncrement + zhalf )
                                   , uint32_t( waveform.size() )
                                   , waveform.meta_.actualAverages
                                   , 0 /* mode */ );
    prop.setAcceleratorVoltage( 3000 );
    prop.setSamplingInfo( info );
    prop.setTrigNumber( waveform.serialnumber_, waveform.serialnumber_origin_ );
    prop.setTimeSinceInjection( waveform.timeSinceInject_ ); // meta_.initialXTimeSeconds );
    prop.setTimeSinceEpoch( waveform.timeSinceEpoch_ ); // nanoseconds
    prop.setDataInterpreterClsid( "ap240" );

    const device_data data( waveform.ident_, waveform.meta_ );
    std::string ar;
    adportable::binary::serialize<>()( data, ar );
    prop.setDeviceData( ar.data(), ar.size() );

    sp.setMSProperty( prop );

    return true;
}

bool
waveform::transform( std::vector< double >& v, const waveform& w, int scale )
{
    v.resize( w.size() );

    if ( w.meta_.dataType == 1 ) {
        std::transform( w.begin<int8_t>(), w.end<int8_t>(), v.begin(), [&]( int8_t y ){ return scale ? w.toVolts( y ) * scale : y; } );
    } else if ( w.meta_.dataType == 2 ) {
        std::transform( w.begin<int16_t>(), w.end<int16_t>(), v.begin(), [&]( int16_t y ){ return scale ? w.toVolts( y ) * scale : y; } );
    } else {
        std::transform( w.begin<int32_t>(), w.end<int32_t>(), v.begin(), [&]( int32_t y ){ return scale ? w.toVolts( y ) * scale : y; } );
    }
    return true;
}


bool
waveform::apply_filter( std::vector<double>& v, const waveform& w, const adcontrols::threshold_method& m )
{
    if ( m.filter ) {

        transform( v, w, 1 );

        if ( m.filter == adcontrols::threshold_method::SG_Filter ) {
            
            adcontrols::waveform_filter::sg::lowpass_filter( v.size(), v.data(), w.meta_.xIncrement, m.sgwidth );
            
        } else if ( m.filter == adcontrols::threshold_method::DFT_Filter ) {

            if ( m.complex_ )
                adcontrols::waveform_filter::fft4c::bandpass_filter( v.size(), v.data(), w.meta_.xIncrement, m.hCutoffHz, m.lCutoffHz );
            else
                adcontrols::waveform_filter::fft4g::bandpass_filter( v.size(), v.data(), w.meta_.xIncrement, m.hCutoffHz, m.lCutoffHz );
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

    translate_property( sp, waveform );

    sp.resize( waveform.size() );
	int idx = 0;

    // ADDEBUG() << __FUNCTION__ << " offset=" << waveform.meta_.scaleOffset << ", dataType=" << waveform.meta_.dataType << " N=" << waveform.meta_.actualAverages;

    if ( waveform.meta_.dataType == 1 ) {
        if ( scale )
            for ( auto y = waveform.begin<int8_t>(); y != waveform.end<int8_t>(); ++y )
                sp.setIntensity( idx++, waveform.toVolts( *y ) * scale );
        else
            for ( auto y = waveform.begin<int8_t>(); y != waveform.end<int8_t>(); ++y )
                sp.setIntensity( idx++, *y );

    } else if ( waveform.meta_.dataType == 2 ) {

        double dbase, rms;
        adportable::spectrum_processor::tic( waveform.size(), waveform.begin<int16_t>(), dbase, rms );

        if ( scale )
            for ( auto y = waveform.begin<int16_t>(); y != waveform.end<int16_t>(); ++y )
                sp.setIntensity( idx++, waveform.toVolts( *y - dbase ) * scale );
        else
            for ( auto y = waveform.begin<int16_t>(); y != waveform.end<int16_t>(); ++y )
                sp.setIntensity( idx++, *y - dbase );

    } else {

        // double dbase, rms;
        // adportable::spectrum_processor::tic( waveform.size(), waveform.begin<int32_t>(), dbase, rms );
        // dbase = waveform.toVolts( int32_t( dbase ) );
        
        if ( scale )
            for ( auto y = waveform.begin<int32_t>(); y != waveform.end<int32_t>(); ++y )
                sp.setIntensity( idx++, waveform.toVolts( *y ) * scale );
        else
            for ( auto y = waveform.begin<int32_t>(); y != waveform.end<int32_t>(); ++y )
                sp.setIntensity( idx++, *y );
    }

    // TBA: mass array, need scanlaw

	return true;
}

//static
bool
waveform::translate( adcontrols::MassSpectrum& sp, const waveform& waveform, mass_assignor_t assign, int scale )
{
    if ( translate( sp, waveform, scale ) ) {

        const adcontrols::MSProperty& prop = sp.getMSProperty();
        const auto& sinfo = prop.samplingInfo();

        double lMass = assign( sinfo.fSampDelay(), prop.mode() );
        double hMass = assign( sinfo.fSampDelay() + sinfo.fSampInterval() * sinfo.nSamples(), prop.mode() );

        sp.setAcquisitionMassRange( lMass, hMass );
        
        return sp.assign_masses( assign );
    }

    return false;
}


//static
bool
waveform::translate( adcontrols::MassSpectrum& sp, const threshold_result_< ap240::waveform >& result, int scale )
{
    using namespace adcontrols::metric;

    auto& waveform = *result.data();

    if ( result.processed().size() == waveform.size() ) { // has filterd waveform
        translate_property( sp, waveform );
        sp.resize( waveform.size() );
        int idx = 0;
        
        if ( scale <= 1 )
            sp.setIntensityArray( result.processed().data() ); // return Volts (no binary avilable for processed waveform)
        else
            for ( auto it = result.processed().begin(); it != result.processed().end(); ++it )
                sp.setIntensity( idx++, *it * scale ); // Volts -> mV (where scale = 1000)
    } else {
        
        return translate( sp, waveform, scale );
        
    }
    
    return false;
}

bool
waveform::translate( adcontrols::MassSpectrum& sp, const threshold_result_< ap240::waveform >& result, mass_assignor_t assign, int scale )
{
    if ( translate( sp, result, scale ) ) {

        const adcontrols::MSProperty& prop = sp.getMSProperty();
        const auto& sinfo = prop.samplingInfo();
        
        double lMass = assign( sinfo.fSampDelay(), prop.mode() );
        double hMass = assign( sinfo.fSampDelay() + sinfo.fSampInterval() * sinfo.nSamples(), prop.mode() );
        
        sp.setAcquisitionMassRange( lMass, hMass );

        return sp.assign_masses( assign );

    }

    return false;
}

bool
waveform::serialize_xmeta( std::string& os ) const
{
    boost::iostreams::back_insert_device< std::string > inserter( os );
    boost::iostreams::stream< boost::iostreams::back_insert_device< std::string > > device( inserter );

    portable_binary_oarchive ar( device );
    waveform_xmeta_archive_t< const waveform > x( *this );
    
    try {
        ar & x;
    } catch ( std::exception& ) {
        return false;
    }
    return true;
}

bool
waveform::deserialize_xmeta( const char * data, size_t size )
{
    boost::iostreams::basic_array_source< char > device( data, size );
    boost::iostreams::stream< boost::iostreams::basic_array_source< char > > st( device );

    portable_binary_iarchive ar( st );

    try {
        waveform_xmeta_archive_t< waveform > x( *this );
        ar & x;
        return true;
    } catch ( ... ) {
        ADDEBUG() << boost::current_exception_diagnostic_information();
    }

    return false;
}

bool
waveform::serialize_xdata( std::string& os ) const
{
    boost::iostreams::back_insert_device< std::string > inserter( os );
    boost::iostreams::stream< boost::iostreams::back_insert_device< std::string > > device( inserter );

    portable_binary_oarchive ar( device );
    waveform_xdata_archive_t< const waveform > x( *this );
    
    try {
        ar & x;
    } catch ( std::exception& ) {
        return false;
    }
    return true;
}

bool
waveform::deserialize_xdata( const char * data, size_t size )
{
    boost::iostreams::basic_array_source< char > device( data, size );
    boost::iostreams::stream< boost::iostreams::basic_array_source< char > > st( device );

    portable_binary_iarchive ar( st );

    try {
        waveform_xdata_archive_t< waveform > x( *this );
        ar & x;
        return true;
    } catch ( ... ) {
        ADDEBUG() << boost::current_exception_diagnostic_information();
    }

    return false;
}

bool
waveform::deserialize( const char * xdata, size_t dsize, const char * xmeta, size_t msize )
{
    return deserialize_xdata( xdata, dsize ) && deserialize_xmeta( xmeta, msize );
}

void
waveform::lvalue_cast()
{
    std::vector< value_type > d( size() );
    
    switch( meta_.dataType ) {
    case 1:
        std::copy( begin< int8_t >(), begin< int8_t >() + size(), d.data() ); break;
    case 2:
        std::copy( begin< int16_t >(), begin< int16_t >() + size(), d.data() ); break;
    case 4:
        std::copy( begin< int32_t >(), begin< int32_t >() + size(), d.data() ); break;            
    }

    d_ = std::move( d );

    meta_.dataType = 4;
    meta_.indexFirstPoint = 0;
    
    if ( meta_.actualAverages == 0 )
        meta_.actualAverages = 1;
}

waveform&
waveform::operator += ( const waveform& t )
{
    if ( meta_.actualAverages < 2 )
        lvalue_cast();
    
    if ( adportable::compare<double>::essentiallyEqual( meta_.xIncrement, t.meta_.xIncrement )
         && adportable::compare<double>::essentiallyEqual( meta_.initialXOffset, t.meta_.initialXOffset )
         && ( meta_.actualPoints <= t.meta_.actualPoints ) ) {
        
        meta_.actualAverages += ( t.meta_.actualAverages ? t.meta_.actualAverages : 1 );
        wellKnownEvents_ |= t.wellKnownEvents_;
        
        if ( t.meta_.dataType == 1 ) { // 8bit 
            std::transform( t.begin<int8_t>(), t.begin<int8_t>() + size(), d_.begin(), d_.begin(), std::plus<int32_t>() );
        } else if ( t.meta_.dataType == 2 ) {
            std::transform( t.begin<int16_t>(), t.begin<int16_t>() + size(), d_.begin(), d_.begin(), std::plus<int32_t>() );
        } else {
            std::transform( t.begin<int32_t>(), t.begin<int32_t>() + size(), d_.begin(), d_.begin(), std::plus<int32_t>() );
        }
    }
    return *this;
}

double
waveform::accumulate( double tof, double window ) const
{
    double tic(0), dbase(0), rms(0);

    if ( meta_.dataType == 1 ) {
        tic = adportable::spectrum_processor::tic( size(), begin<int8_t>(), dbase, rms, 5 );    
    } else if ( meta_.dataType == 2 ) {
        tic = adportable::spectrum_processor::tic( size(), begin<int16_t>(), dbase, rms, 5 );
    } else if ( meta_.dataType == 4 ) {
        tic = adportable::spectrum_processor::tic( size(), begin<int32_t>(), dbase, rms, 5 );
    }
    
    if ( std::abs( tof ) <= std::numeric_limits< double >::epsilon() ) {

        return tic;

    } else {

        const adcontrols::TofProtocol& this_protocol = method_.protocols() [ method_.protocolIndex() ];
        double ext_adc_delay = this_protocol.delay_pulses().at( adcontrols::TofProtocol::EXT_ADC_TRIG ).first;

        auto x1 = ( ( tof - window / 2.0 ) - meta_.initialXOffset - ext_adc_delay ) / meta_.xIncrement;
        auto x2 = ( ( tof + window / 2.0 ) - meta_.initialXOffset - ext_adc_delay ) / meta_.xIncrement;
        adportable::spectrum_processor::areaFraction frac;
        x1 = std::max( 0.0, x1 );
        x2 = std::max( 0.0, x2 );
        frac.lPos = size_t( std::ceil( x1 ) );
        frac.uPos = size_t( std::floor( x2 ) );
        if ( frac.lPos > 0 )
            frac.lFrac = x1 - double( frac.lPos - 1 );
        frac.uFrac = x2 - double( frac.uPos );

        if ( meta_.dataType == 1 ) {
            return adportable::spectrum_processor::area( frac, dbase, begin<int8_t>(), size() );            
        } else if ( meta_.dataType == 2 ) {
            return adportable::spectrum_processor::area( frac, dbase, begin<int16_t>(), size() );
        } else if ( meta_.dataType == 4 ) {
            return adportable::spectrum_processor::area( frac, dbase, begin<int32_t>(), size() );
        }
    }
    return 0;
}

void
waveform::move( std::shared_ptr< acqrscontrols::aqdrv4::waveform >&& t )
{
    const auto& dataDesc = t->dataDesc();

    timeSinceEpoch_           = t->timeSinceEpoch_;
    meta_.dataType            = t->dataType_;
    meta_.indexFirstPoint     = dataDesc.indexFirstPoint;
    meta_.channel             = 0;
    meta_.actualAverages      = 0;
    meta_.actualPoints        = dataDesc.returnedSamplesPerSeg; // data.d_.size();
    meta_.flags               = 0;                              // not supported on digitizer
    meta_.initialXOffset      = t->delayTime_;                  // data.method_.hor_.delayTime;
    meta_.initialXTimeSeconds = double( t->timeStamp() ) / 1.0e12;
            
    meta_.scaleFactor         = dataDesc.vGain;     // V = vGain * data - vOffset
    meta_.scaleOffset         = dataDesc.vOffset;
    meta_.xIncrement          = dataDesc.sampTime;
    meta_.horPos              = t->segDesc_.horPos;
    
    // acqrscontrols::ap240::method method_;
    serialnumber_             = t->serialnumber_;
    serialnumber_origin_      = t->serialnumber0_;
    wellKnownEvents_          = t->wellKnownEvents_;
    firstValidPoint_          = t->dataDesc_.indexFirstPoint;
    timeSinceEpoch_           = t->timeSinceEpoch_;  // uint64_t (ns)
    timeSinceInject_          = double( t->timeSinceInject_ ) / std::nano::den; // uint64_t(ns) -> double(s)
    // identify ident_;

    d_ = std::move( t->d_ );
}
