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
#include "method.hpp"
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/waveform_filter.hpp>

#include <adportable/asio/thread.hpp>
#include <adportable/binary_serializer.hpp>
#include <adportable/debug.hpp>
#include <adportable/mblock.hpp>
#include <adportable/serializer.hpp>
#include <adportable/spectrum_processor.hpp>
#include <adportable/timesquaredscanlaw.hpp>
#include <adportable/waveform_processor.hpp>
#include <adportable/waveform_wrapper.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <adportable/portable_binary_iarchive.hpp>
#include <adicontroller/signalobserver.hpp>
#include <adlog/logger.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/exception/all.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/type_traits.hpp>
#include <boost/variant.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>

namespace acqrscontrols { namespace u5303a {

        template<typename data_type> struct waveform_copy {

            void operator ()( adcontrols::MassSpectrum& sp, const waveform& w, int scale ) const {
                int idx = 0;
                for ( auto it = w.begin<data_type>(); it != w.end<data_type>(); ++it ) {
                    if ( scale ) // mV(1000) or V(1)
                        sp.setIntensity( idx++, scale ? ( w.toVolts( *it ) * scale ) : *it );
                }         
            }

            void operator ()( adcontrols::MassSpectrum& sp, const waveform& w, int scale, double dbase ) const {
                int idx = 0;                
                for ( auto it = w.begin<data_type>(); it != w.end<data_type>(); ++it ) {
					double d = scale ? ( w.toVolts(*it - dbase) * scale  ) : *it - dbase;
					sp.setIntensity( idx++, d );
                }         
            }

        };

        // class waveform_xmeta_archive {
        //     waveform_xmeta_archive( const waveform_xmeta_archive& ) = delete;
        //     waveform_xmeta_archive& operator = ( const waveform_xmeta_archive& ) = delete;
        // public:
        //     waveform_xmeta_archive() {}
        //     ~waveform_xmeta_archive()
        //     {}

        //     identify ident_;
        //     std::vector< acqrscontrols::u5303a::metadata > meta_;
        //     std::shared_ptr< acqrscontrols::u5303a::method > method_;
        // private:
        //     friend class boost::serialization::access;
        //     template<class Archive>
        //     void serialize( Archive& ar, const unsigned int ) {
        //         using namespace boost::serialization;
        //         ar & BOOST_SERIALIZATION_NVP( ident_ );
        //         ar & BOOST_SERIALIZATION_NVP( meta_ );
        //         ar & BOOST_SERIALIZATION_NVP( method_ );
        //     }
        // };

        template<typename T = waveform >
        class waveform_xmeta_archive {
        public:
            template<class Archive>
            void serialize( Archive& ar, T& _, const unsigned int ) {
                using namespace boost::serialization;
                ar & BOOST_SERIALIZATION_NVP( _.ident_ );
                ar & BOOST_SERIALIZATION_NVP( _.meta_ );
                ar & BOOST_SERIALIZATION_NVP( _.method_ );
                ar & BOOST_SERIALIZATION_NVP( _.serialnumber_ );
                ar & BOOST_SERIALIZATION_NVP( _.timeSinceEpoch_ );
            }
        };

        
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

    }
}


using namespace acqrscontrols::u5303a;

waveform::waveform( std::shared_ptr< const identify > id
                    , uint32_t pos, uint32_t events, uint64_t tp ) : ident_( id )
                                                                   , serialnumber_( pos )
                                                                   , wellKnownEvents_( events )
                                                                   , timeSinceEpoch_( tp )
                                                                   , firstValidPoint_( 0 )
{
}

waveform::waveform( const waveform& rv
                    , std::unique_ptr< int32_t [] >& data
                    , size_t size
                    , bool invert ) : method_( rv.method_ )
                                    , meta_( rv.meta_ )
                                    , ident_( rv.ident_ )
                                    , serialnumber_( rv.serialnumber_ )
                                    , wellKnownEvents_( rv.wellKnownEvents_ )
                                    , timeSinceEpoch_( rv.timeSinceEpoch_ )
                                    , firstValidPoint_( 0 )
                                    , mblock_( std::make_shared< adportable::mblock< int32_t > >( data, size ) )
{
    typedef int32_t value_type;

    meta_.dataType = sizeof( value_type );

    if ( invert ) {
        auto p = this->template data< value_type >();
        std::transform( p, p + size, p, std::negate<value_type>() );
    }
}


const int32_t *
waveform::trim( metadata& meta, uint32_t& nSamples ) const
{
    meta = meta_;

    size_t offset = 0;
    if ( method_.method_.digitizer_delay_to_first_sample < method_.method_.delay_to_first_sample_ )
        offset = size_t( ( ( method_.method_.delay_to_first_sample_ - method_.method_.digitizer_delay_to_first_sample ) / meta.xIncrement ) + 0.5 );

    nSamples = method_.method_.nbr_of_s_to_acquire_;
    if ( nSamples + offset > method_.method_.digitizer_nbr_of_s_to_acquire )
        nSamples = uint32_t( method_.method_.digitizer_nbr_of_s_to_acquire - offset );

    meta.initialXOffset = method_.method_.delay_to_first_sample_;
    meta.actualPoints = nSamples;

	return begin<int32_t>() + offset;
}

size_t
waveform::size() const
{
    return meta_.actualPoints;
}

int
waveform::dataType() const
{
    return meta_.dataType;
}

int
waveform::operator [] ( size_t idx ) const
{
    switch( meta_.dataType ) {
    case 2: return *(begin<int16_t>() + idx);
    case 4: return *(begin<int32_t>() + idx);
    }
	throw std::bad_cast();    
}

std::pair<double, int>
waveform::xy( size_t idx ) const
{
    double time = idx * meta_.xIncrement + meta_.initialXOffset;

    assert( meta_.dataType == 2 || meta_.dataType == 4 );
    
    switch( meta_.dataType ) {
    case 2: return std::make_pair( time, *(begin<int16_t>() + idx) );
    case 4: return std::make_pair( time, *(begin<int32_t>() + idx) );
    }
	throw std::bad_cast();
}

double
waveform::toVolts( int d ) const
{
    if ( meta_.actualAverages == 0 )
        return meta_.scaleFactor * d + meta_.scaleOffset;
    else
        return double( meta_.scaleFactor * d ) / meta_.actualAverages * ( meta_.scaleOffset * meta_.actualAverages );
}

double
waveform::toVolts( double d ) const
{
    if ( meta_.actualAverages )
        return d * meta_.scaleFactor / meta_.actualAverages;
    return d * meta_.scaleFactor;
}

size_t
waveform::serialize_xmeta( std::string& os )
{
    boost::iostreams::back_insert_device< std::string > inserter( os );
    boost::iostreams::stream< boost::iostreams::back_insert_device< std::string > > device( inserter );

    portable_binary_oarchive ar( device );

    int version ( 0 );
    waveform_xmeta_archive<>().serialize( ar, *this, version );

    return os.size();
}

size_t
waveform::serialize_xdata( std::string& device )
{
    if ( meta_.dataType == 2 ) {
        
        device.resize( ( size() * sizeof(int16_t) + ( 4 * sizeof(int32_t) ) ) );
        int32_t * dest_p = reinterpret_cast<int32_t *>( const_cast< char * >( device.data() ) );
        *dest_p++ = 0x7ffe0001; // separater & endian marker
        *dest_p++ = int32_t( size() );

        adportable::waveform_wrapper< int16_t, acqrscontrols::u5303a::waveform > _16( *this );
        std::copy( _16.begin(), _16.end(), reinterpret_cast< int16_t *>(dest_p) );        
        
    } else {
        
        device.resize( ( size() * sizeof(int32_t) + ( 4 * sizeof(int32_t) ) ) );
        int32_t * dest_p = reinterpret_cast<int32_t *>( const_cast< char * >( device.data() ) );
        *dest_p++ = 0x7ffe0001; // separater & endian marker
        *dest_p++ = int32_t( size() );
        
        adportable::waveform_wrapper< int32_t, acqrscontrols::u5303a::waveform > _32( *this );
        std::copy( _32.begin(), _32.end(), dest_p );
    }
    
    return device.size();
}

#if 0
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

        try {
            waveform_xmeta_archive x;
            if ( adportable::binary::deserialize<>()( x, reinterpret_cast<const char *>( rb->xmeta().data() ), rb->xmeta().size() ) ) {
            
                waveforms[ 0 ] = std::make_shared< waveform >( std::make_shared< identify >( x.ident_ ), rb->pos(), rb->events(), rb->timepoint() );
                waveforms[ 0 ]->meta_ = x.meta_[ 0 ];
                if ( x.method_ )
                    waveforms[ 0 ]->method_ = *x.method_;
            
                const uint32_t * pdata = reinterpret_cast<const uint32_t *>( rb->xdata().data() );
                for ( auto& waveform : waveforms ) {
                    if ( *pdata == 0x7ffe0001 ) {
                        ++pdata;
                        uint32_t size = *pdata++;
                        if ( size && waveform ) {
                            auto * data_p = reinterpret_cast<const acqrscontrols::u5303a::waveform::value_type *>( pdata );
							std::copy( data_p, data_p + size, waveform->data( size ) );
                            pdata = reinterpret_cast<const uint32_t *>( data_p + size );
                        }
                    }
                }
            }
            return std::array< std::shared_ptr< const waveform >, 2 >( { waveforms[ 0 ], nullptr } );
        } catch ( ... ) {
            ADERROR() << boost::current_exception_diagnostic_information();
        }
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

        const size_t data_count = ( ch1 ? ch1->d_.size() : 0 ) + ( ch2 ? ch2->d_.size() : 0 );
        if ( ch1 )
            x.meta_.push_back( ch1->meta_ );
        if ( ch2 )
            x.meta_.push_back( ch2->meta_ );

        { // serialize xmeta
            x.ident_ = *waveform.ident_;
            x.method_ = std::make_shared< acqrscontrols::u5303a::method >( waveform.method_ );
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
            *dest_p++ = ptr ? int32_t( ptr->d_.size() ) : 0;
            if ( ptr ) {
                rb.ndata()++;
                std::copy( ptr->d_.begin(), ptr->d_.end(), dest_p );
                dest_p += ptr->d_.size();
            }
        }
        return true;
    }
    return false;
}
#endif

bool
waveform::transform( std::vector< double >& v, const waveform& w, int scale )
{
    v.resize( w.size() );

    if ( w.meta_.dataType == 2 ) {
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
                adcontrols::waveform_filter::fft4c::lowpass_filter( v.size(), v.data(), w.meta_.xIncrement, m.cutoffHz );
            else
                adcontrols::waveform_filter::fft4g::lowpass_filter( v.size(), v.data(), w.meta_.xIncrement, m.cutoffHz );
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
    
    prop.setTimeSinceInjection( waveform.meta_.initialXTimeSeconds );
    prop.setTimeSinceEpoch( waveform.timeSinceEpoch_ ); // nanoseconds
    prop.setDataInterpreterClsid( "u5303a" );

    const device_data data( *waveform.ident_, waveform.meta_ );
    std::string ar;
    adportable::binary::serialize<>()( data, ar );
    prop.setDeviceData( ar.data(), ar.size() );

    // prop.setDeviceData(); TBA
    sp.setMSProperty( prop );
    sp.resize( waveform.size() );

	if ( waveform.meta_.actualAverages == 0 ) { // digitizer mode data

		if ( waveform.meta_.dataType == 2 ) {
			waveform_copy<int16_t>()( sp, waveform, scale );
		} else {
			waveform_copy<int32_t>()( sp, waveform, scale );
		}

    } else {

        double dbase, rms;
		double tic = adportable::spectrum_processor::tic( waveform.size(), waveform.begin<int32_t>(), dbase, rms );
        waveform_copy<int32_t>()( sp, waveform, scale, dbase );

    }

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
    
    prop.setTimeSinceInjection( waveform.meta_.initialXTimeSeconds );
    prop.setTimeSinceEpoch( waveform.timeSinceEpoch_ ); // nanoseconds
    prop.setDataInterpreterClsid( "u5303a" );

    const device_data data( *waveform.ident_, waveform.meta_ );
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
        
    } else if ( waveform.meta_.actualAverages == 0 ) { // digitizer mode data
		if ( waveform.meta_.dataType == 2 ) {
			waveform_copy<int16_t>()( sp, waveform, scale );
		} else {
			waveform_copy<int32_t>()( sp, waveform, scale );
		}
    } else {
        double dbase, rms;
		double tic = adportable::spectrum_processor::tic( waveform.size(), waveform.begin<int32_t>(), dbase, rms );
		waveform_copy<int32_t>()( sp, waveform, scale, dbase );
    }
    
	return true;
}

bool
waveform::isDEAD() const
{
    size_t count( 99 );
	if ( meta_.dataType == 2 ) {
		for ( auto it = begin<int16_t>(); it != end<int16_t>() && count--; ++it )
			if ( !( *it == 0 || *it == 0xdead ) )
				return false;
	} else {
		for ( auto it = begin<int32_t>(); it != end<int32_t>() && count--; ++it )
			if ( !( *it == 0 || *it == 0xffffdead ) )
				return false;			
	}
    return true;
}

void
waveform::setData( const std::shared_ptr< adportable::mblock<int32_t> >& mblk, size_t firstValidPoint )
{
    mblock_ = mblk;
    firstValidPoint_ = firstValidPoint;
}

void
waveform::setData( const std::shared_ptr< adportable::mblock<int16_t> >& mblk, size_t firstValidPoint )
{
    mblock_ = mblk;
    firstValidPoint_ = firstValidPoint;
}

template<> const int16_t *
waveform::begin() const
{
    if ( mblock_.which() == 1 ) {
        auto&& mblk = boost::get < std::shared_ptr< adportable::mblock<int16_t> > >( mblock_ );
        return mblk->data() + firstValidPoint_;
    }
    throw std::bad_cast();
}

template<> const int16_t *
waveform::end() const
{
    if ( mblock_.which() == 1 ) {
        auto&& mblk = boost::get < std::shared_ptr< adportable::mblock<int16_t> > >( mblock_ );
        return mblk->data() + firstValidPoint_ + meta_.actualPoints;
    }
    throw std::bad_cast();
}

template<> const int32_t *
waveform::begin() const
{
    if ( mblock_.which() == 0 ) {
        auto&& mblk = boost::get < std::shared_ptr< adportable::mblock<int32_t> > >( mblock_ );
        return mblk->data() + firstValidPoint_;
    }
    throw std::bad_cast();
}

template<> const int32_t *
waveform::end() const
{
    if ( mblock_.which() == 0 ) {
        auto&& mblk = boost::get < std::shared_ptr< adportable::mblock<int32_t> > >( mblock_ );
        return mblk->data() + firstValidPoint_ + meta_.actualPoints;
    }
    throw std::bad_cast();
}

template<> int16_t *
waveform::data()
{
    if ( mblock_.which() == 1 ) {
        auto&& mblk = boost::get < std::shared_ptr< adportable::mblock<int16_t> > >( mblock_ );
        return mblk->data() + firstValidPoint_;
    }
    throw std::bad_cast();    
}

template<> int32_t *
waveform::data()
{
    if ( mblock_.which() == 0 ) {
        auto&& mblk = boost::get < std::shared_ptr< adportable::mblock<int32_t> > >( mblock_ );
        return mblk->data() + firstValidPoint_;
    }
    throw std::bad_cast();        
}


