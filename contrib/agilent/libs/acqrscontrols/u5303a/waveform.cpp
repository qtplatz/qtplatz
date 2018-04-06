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
#include <adcontrols/samplinginfo.hpp>
#include <adcontrols/scanlaw.hpp>
#include <adcontrols/tofprotocol.hpp>
#include <adcontrols/waveform_filter.hpp>

#include <adportable/asio/thread.hpp>
#include <adportable/binary_serializer.hpp>
#include <adportable/debug.hpp>
#include <adportable/float.hpp>
#include <adportable/mblock.hpp>
#include <adportable/serializer.hpp>
#include <adportable/spectrum_processor.hpp>
#include <adportable/timesquaredscanlaw.hpp>
#include <adportable/waveform_processor.hpp>
#include <adportable/waveform_wrapper.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <adportable/portable_binary_iarchive.hpp>
#include <adacquire/signalobserver.hpp>
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
#include <boost/serialization/version.hpp>
#include <boost/serialization/nvp.hpp>

namespace acqrscontrols {
    namespace u5303a {

        template<typename data_type> struct waveform_copy {

            void operator ()( adcontrols::MassSpectrum& sp, const waveform& w, int scale ) const {
                int idx = 0;
                if ( w.method_.mode() == method::DigiMode::Digitizer ) {
                    for ( auto it = w.begin<data_type>(); it != w.end<data_type>(); ++it )
                        sp.setIntensity( idx++, scale ? ( toVolts_< data_type, method::DigiMode::Digitizer >()( w.meta_, *it ) * scale ) : *it );
                } else {
                    for ( auto it = w.begin<data_type>(); it != w.end<data_type>(); ++it )
                        sp.setIntensity( idx++, scale ? ( toVolts_< data_type, method::DigiMode::Averager >()( w.meta_, *it ) * scale ) : *it );
                }
            }

            void operator ()( adcontrols::MassSpectrum& sp, const waveform& w, int scale, double dbase ) const {
                int idx = 0;
                if ( w.method_.mode() == method::DigiMode::Digitizer ) {
                    for ( auto it = w.begin<data_type>(); it != w.end<data_type>(); ++it ) {
                        double d = scale ? ( toVolts_< data_type, method::DigiMode::Digitizer >()( w.meta_, (*it - dbase)) * scale  ) : *it - dbase;
                        sp.setIntensity( idx++, d );
                    }
                } else {
                    for ( auto it = w.begin<data_type>(); it != w.end<data_type>(); ++it ) {
                        double d = scale ? ( toVolts_< data_type, method::DigiMode::Averager >()( w.meta_, (*it - dbase)) * scale  ) : *it - dbase;
                        sp.setIntensity( idx++, d );
                    }
                }
            }

        };

        template<typename T = waveform >
        class waveform_xmeta_archive {
            T& _;
        public:
            waveform_xmeta_archive( T& t ) : _( t ) {}

            template<class Archive>
            void serialize( Archive& ar, const unsigned int version ) {
                using namespace boost::serialization;
                ar & BOOST_SERIALIZATION_NVP( _.ident_ );
                ar & BOOST_SERIALIZATION_NVP( _.meta_ );
                ar & BOOST_SERIALIZATION_NVP( _.method_ );
                ar & BOOST_SERIALIZATION_NVP( _.serialnumber_ );
                ar & BOOST_SERIALIZATION_NVP( _.timeSinceEpoch_ );
                if ( version >= 1 ) {
                    ar & BOOST_SERIALIZATION_NVP( _.timeSinceInject_ );
                    ar & BOOST_SERIALIZATION_NVP( _.wellKnownEvents_ );
                }
            }

            // old bad implementation -- can't version this class -- for data compatibility
            template<class Archive>
            static void serialize( Archive& ar, T& x, const unsigned int ) {
                using namespace boost::serialization;
                ar & BOOST_SERIALIZATION_NVP( x.ident_ );
                ar & BOOST_SERIALIZATION_NVP( x.meta_ );
                ar & BOOST_SERIALIZATION_NVP( x.method_ );
                ar & BOOST_SERIALIZATION_NVP( x.serialnumber_ );
                ar & BOOST_SERIALIZATION_NVP( x.timeSinceEpoch_ );
            }

        };

        ////////////////////
        template<typename T = device_data>
        class device_data_archive {
            T& _;
        public:
            device_data_archive( T& t ) : _( t ) {}
        public:
            template<class Archive>
            void serialize( Archive& ar, const unsigned int ) {
                using namespace boost::serialization;
                ar & BOOST_SERIALIZATION_NVP( _.ident_ );
                ar & BOOST_SERIALIZATION_NVP( _.meta_ );
            }

            template<class Archive>
            static void serialize( Archive& ar, T& _, const unsigned int ) {
                using namespace boost::serialization;
                ar & BOOST_SERIALIZATION_NVP( _.ident_ );
                ar & BOOST_SERIALIZATION_NVP( _.meta_ );
            }
        };

        template<> ACQRSCONTROLSSHARED_EXPORT void device_data::serialize( boost::archive::xml_woarchive& ar, unsigned int )
        {
            device_data_archive<> x( *this );            
            ar & boost::serialization::make_nvp( "device_data", x );
        }

        template<> ACQRSCONTROLSSHARED_EXPORT void device_data::serialize( boost::archive::xml_wiarchive& ar, unsigned int )
        {
            device_data_archive<> x( *this );
            ar & boost::serialization::make_nvp( "device_data", x );
        }
        
        template<> ACQRSCONTROLSSHARED_EXPORT void device_data::serialize( portable_binary_oarchive& ar, unsigned int )
        {
            ar & device_data_archive<>( *this );
        }

        template<> ACQRSCONTROLSSHARED_EXPORT void device_data::serialize( portable_binary_iarchive& ar, unsigned int )
        {
            device_data_archive<> x( *this );            
            ar & x;
        }

    }
}

// **** BOOST_CLASS_VERSION( T, N ) *****
namespace boost { namespace serialization {
        using namespace acqrscontrols::u5303a;
        template< typename T > struct version< waveform_xmeta_archive< T > > { BOOST_STATIC_CONSTANT( int, value = 1 ); };
        template< typename T > struct version< device_data_archive< T > > { BOOST_STATIC_CONSTANT( int, value = 1 ); };
    }
}

using namespace acqrscontrols::u5303a;

#if defined __APPLE__

template<> const int boost::serialization::version< waveform_xmeta_archive< waveform > >::value;
template<> const int boost::serialization::version< waveform_xmeta_archive< const waveform > >::value;

template<> const int boost::serialization::version< device_data_archive< waveform > >::value;
template<> const int boost::serialization::version< device_data_archive< const waveform > >::value;

#endif

waveform::waveform() : serialnumber_( 0 )
                     , wellKnownEvents_( 0 )
                     , timeSinceEpoch_( 0 )
                     , firstValidPoint_( 0 )
                     , timeSinceInject_( 0.0 )
                     , hasTic_( false )
                     , tic_( 0 )
                     , dbase_( 0 )
                     , rms_( 0 )
{
}

waveform::waveform( std::shared_ptr< const identify > id
                    , uint32_t pos, uint32_t events, uint64_t tp ) : serialnumber_( pos )
                                                                   , wellKnownEvents_( events )
                                                                   , timeSinceEpoch_( tp )
                                                                   , firstValidPoint_( 0 )
                                                                   , timeSinceInject_( 0.0 )
                                                                   , hasTic_( false )
                                                                   , tic_( 0 )
                                                                   , dbase_( 0 )
                                                                   , rms_( 0 )
                                                                   , ident_( id )
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
                                    , timeSinceEpoch_( timeSinceEpoch )
                                    , firstValidPoint_( firstValidPoint )
                                    , timeSinceInject_( timeSinceInject )
                                    , ident_( id )
                                    , hasTic_( false )
                                    , tic_( 0 )
                                    , dbase_( 0 )
                                    , rms_( 0 )
                                    , mblock_( std::make_shared< adportable::mblock< int32_t > >( xdata, size ) )
{
    typedef int32_t value_type;

    meta_.dataType = sizeof( value_type );

    if ( invert ) {
        auto p = this->template data< value_type >();
        std::transform( p, p + size, p, std::negate<value_type>() );
    }
}

waveform::waveform( const waveform& t, int dataType ) : method_( t.method_ )
                                                      , meta_( t.meta_ )
                                                      , serialnumber_( t.serialnumber_ )
                                                      , wellKnownEvents_( t.wellKnownEvents_ )
                                                      , timeSinceEpoch_( t.timeSinceEpoch_ )
                                                      , firstValidPoint_( t.firstValidPoint_ )
                                                      , timeSinceInject_( t.timeSinceInject_ )
                                                      , ident_( t.ident_ )
                                                      , hasTic_( t.hasTic_ )
                                                      , tic_( t.tic_ )
                                                      , dbase_( t.dbase_ )
                                                      , rms_( t.rms_ )
{
    if ( dataType == 8 ) {
        auto mb = std::make_shared< adportable::mblock< int64_t > >( t.size() );
        if ( t.meta_.dataType == 2 )
            std::copy( t.data<int16_t>(), t.data<int16_t>() + size(), mb->data() );
        else if ( t.meta_.dataType == 4 )
            std::copy( t.data<int32_t>(), t.data<int32_t>() + size(), mb->data() );
        mblock_ = mb;
    }
    if ( dataType == 4 ) {
        auto mb = std::make_shared< adportable::mblock< int32_t > >( t.size() );
        if ( t.meta_.dataType == 2 )
            std::copy( t.data<int16_t>(), t.data<int16_t>() + size(), mb->data() );
        else if ( t.meta_.dataType == 4 )
            std::copy( t.data<int32_t>(), t.data<int32_t>() + size(), mb->data() );
        mblock_ = mb;        
    }
    meta_.dataType = dataType;
}

waveform&
waveform::operator += ( const waveform& t )
{
    if ( adportable::compare<double>::essentiallyEqual( meta_.xIncrement, t.meta_.xIncrement )
         && ( meta_.actualPoints <= t.meta_.actualPoints ) ) {

        meta_.actualAverages += ( t.meta_.actualAverages == 0 ) ? 1 : t.meta_.actualAverages;
        wellKnownEvents_ |= t.wellKnownEvents_;

        double tic(0), dbase(0), rms(0);

        if ( t.meta_.dataType == 2 ) {
            tic = adportable::spectrum_processor::tic( t.size(), t.begin<int16_t>(), dbase, rms, 5 );
        } else if ( t.meta_.dataType == 4 ) {
            tic = adportable::spectrum_processor::tic( t.size(), t.begin<int32_t>(), dbase, rms, 5 );
        } else if ( t.meta_.dataType == 8 ) {
            tic = adportable::spectrum_processor::tic( t.size(), t.begin<int64_t>(), dbase, rms, 5 );            
        } else
            throw std::bad_cast();

        if ( t.meta_.dataType == 2 ) {
            typedef int16_t rvalue_t;
            switch( meta_.dataType ) {
            case 2:
                add< int16_t, rvalue_t >( t, dbase );
                break;
            case 4:
                add< int32_t, rvalue_t >( t, dbase );
                break;
            case 8:
                add< int64_t, rvalue_t >( t, dbase );
                break;
            }
        } else if ( t.meta_.dataType == 4 ) {
            typedef int32_t rvalue_t;
            switch( meta_.dataType ) {
            case 2:
                throw std::bad_cast();                
                break;
            case 4:
                add< int32_t, rvalue_t >( t, dbase );
                break;
            case 8:
                add< int64_t, rvalue_t >( t, dbase );
                break;
            }
        } else if ( t.meta_.dataType == 8 ) {
            typedef int64_t rvalue_t;
            switch( meta_.dataType ) {
            case 2:
            case 4:
                throw std::bad_cast();
                break;
            case 8:
                add< int64_t, rvalue_t >( t, dbase );
                break;
            }
        }

#if 0
        /// debug
        if ( t.meta_.dataType == 4 ) {
            auto pair = std::minmax_element( t.begin< int32_t >(), t.begin< int32_t >() + t.size() );
            auto a = (*this)[ std::distance( t.begin< int32_t >(), pair.second ) ];
            ADDEBUG() << boost::format( "t=%d[%4d] (max,min)= (%-9d, %-9d), sum = %-9d" ) % meta_.dataType % meta_.actualAverages % *pair.second % *pair.first % a;
        }
        // end debug
#endif
    }
    return *this;
}

double
waveform::accumulate( double tof, double window ) const
{
    double tic(0), dbase(0), rms(0);
    if ( ! hasTic_ ) {
        if ( meta_.dataType == 2 ) {
            tic = adportable::spectrum_processor::tic( size(), begin<int16_t>(), dbase, rms, 5 );
        } else if ( meta_.dataType == 4 ) {
            tic = adportable::spectrum_processor::tic( size(), begin<int32_t>(), dbase, rms, 5 );
        } else if ( meta_.dataType == 8 ) {
            tic = adportable::spectrum_processor::tic( size(), begin<int64_t>(), dbase, rms, 5 );
        }
        const_cast< waveform& >(*this).hasTic_ = true;
        const_cast< waveform& >(*this).dbase_ = dbase;
        const_cast< waveform& >(*this).rms_ = rms;
        const_cast< waveform& >(*this).tic_ = tic;
    }
    
    if ( std::abs( tof ) <= std::numeric_limits< double >::epsilon() ) {

        return tic_;

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

        if ( meta_.dataType == 2 ) {
            return adportable::spectrum_processor::area( frac, dbase_, begin<int16_t>(), size() );
        } else if ( meta_.dataType == 4 ) {
            return adportable::spectrum_processor::area( frac, dbase_, begin<int32_t>(), size() );
        } else if ( meta_.dataType == 8 ) {
            return adportable::spectrum_processor::area( frac, dbase_, begin<int64_t>(), size() );            
        }
    }
    return 0;
}

size_t
waveform::size() const
{
    return meta_.actualPoints;
}

void
waveform::set_events( uint32_t e )
{
    wellKnownEvents_ |= e;
}

int
waveform::dataType() const
{
    return meta_.dataType;
}

int64_t
waveform::operator [] ( size_t idx ) const
{
    switch( meta_.dataType ) {
    case 2: return *(begin<int16_t>() + idx);
    case 4: return *(begin<int32_t>() + idx);
    case 8: return *(begin<int64_t>() + idx);
    }
	throw std::bad_cast();    
}

std::pair<double, int64_t>
waveform::xy( size_t idx ) const
{
    double time = this->time( idx ); //idx * meta_.xIncrement + meta_.initialXOffset;

    assert( meta_.dataType == 2 || meta_.dataType == 4 );
    
    switch( meta_.dataType ) {
    case 2: return std::make_pair( time, *(begin<int16_t>() + idx) );
    case 4: return std::make_pair( time, *(begin<int32_t>() + idx) );
    case 8: return std::make_pair( time, *(begin<int64_t>() + idx) );
    }
	throw std::bad_cast();
}

double
waveform::time( size_t idx ) const
{
    double ext_trig_delay( 0 );

    if ( method_.protocols().size() > method_.protocolIndex() ) {
        const auto& this_protocol = method_.protocols()[ method_.protocolIndex() ];
        ext_trig_delay = this_protocol.delay_pulses() [ adcontrols::TofProtocol::EXT_ADC_TRIG ].first;
    }

    return idx * meta_.xIncrement + meta_.initialXOffset + ext_trig_delay;
}

double
waveform::toVolts( int32_t d ) const
{
    if ( method_.mode() == method::DigiMode::Digitizer )
        return toVolts_<int32_t,method::DigiMode::Digitizer>()( meta_, d );
    else
        return toVolts_<int32_t,method::DigiMode::Averager>()( meta_, d );
}

double
waveform::toVolts( int64_t d ) const
{
    if ( method_.mode() == method::DigiMode::Digitizer )
        return toVolts_<int64_t,method::DigiMode::Digitizer>()( meta_, d );
    else
        return toVolts_<int64_t,method::DigiMode::Averager>()( meta_, d );
}

double
waveform::toVolts( double d ) const
{
    if ( method_.mode() == method::DigiMode::Digitizer )
        return toVolts_<double,method::DigiMode::Digitizer>()( meta_, d );
    else
        return toVolts_<double,method::DigiMode::Averager>()( meta_, d );
}

size_t
waveform::serialize_xmeta( std::string& os ) const
{
    boost::iostreams::back_insert_device< std::string > inserter( os );
    boost::iostreams::stream< boost::iostreams::back_insert_device< std::string > > device( inserter );

    portable_binary_oarchive ar( device );

    waveform_xmeta_archive< const waveform > x( *this );
    ar & x;

    return os.size();
}

bool
waveform::deserialize_xmeta( const char * data, size_t size )
{
    boost::iostreams::basic_array_source< char > device( data, size );
    boost::iostreams::stream< boost::iostreams::basic_array_source< char > > st( device );

    portable_binary_iarchive ar( st );

    // ==> workaround for old bad serializer implementation
    // Only the data using old archive should have follwoing sequence of data, that does not match with collect serializer.
    if ( size >= 44 && 
         data [ 39 ] == 'A' && data [ 40 ] == 'g' && data [ 41 ] == 'M' && data [ 42 ] == 'D' && data [ 43 ] == '2' ) {

        // this data is only on WSPC, for CO2 and N2O quantitative analysis acquired from 2015-NOV-24 to 2015-DEC-28
        try {
            waveform_xmeta_archive< waveform >::serialize( ar, *this, 0 );
            return true;
        } catch ( ... ) {
            ADDEBUG() << boost::current_exception_diagnostic_information();
        }
    }
    // <== end of workaround

    // good impl.
    try {
        waveform_xmeta_archive< waveform > x( *this );
        ar & x;
        return true;
    } catch ( ... ) {
        ADDEBUG() << boost::current_exception_diagnostic_information();
    }

    return false;
}

size_t
waveform::serialize_xdata( std::string& device ) const
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

bool
waveform::deserialize_xdata( const char * data, size_t size )
{
    if ( size < sizeof( int32_t ) * 2 )
        return 0;

    if ( !( meta_.dataType == 2 || meta_.dataType == 4 ) )
        throw std::bad_cast();

    const int32_t * src_p = reinterpret_cast<const int32_t *>( data );
    size_t count( 0 );

    if ( *src_p++ != 0x7ffe0001 )
        throw std::bad_cast();        

    count = *src_p++;

    assert( count == meta_.actualPoints );

    if ( meta_.dataType == 2 ) {

        auto mblk = std::make_shared< adportable::mblock< int16_t > >( count );
        mblock_ = mblk;
        std::copy( reinterpret_cast< const int16_t * >(src_p), reinterpret_cast< const int16_t * >(src_p) + count, mblk->data() );
        
    } else if ( meta_.dataType == 4 ) {

        auto mblk = std::make_shared< adportable::mblock< int32_t > >( count );
        mblock_ = mblk;
        std::copy( reinterpret_cast< const int32_t * >(src_p), reinterpret_cast< const int32_t * >(src_p) + count, mblk->data() );

    }
    
    return true;
}

bool
waveform::deserialize( const char * xdata, size_t dsize, const char * xmeta, size_t msize )
{
    return deserialize_xdata( xdata, dsize ) && deserialize_xmeta( xmeta, msize );
}

bool
waveform::transform( std::vector< double >& v, const waveform& w, int scale )
{
    v.resize( w.size() );

    if ( w.meta_.dataType == 2 ) {
        std::transform( w.begin<int16_t>(), w.end<int16_t>(), v.begin()
                        , [&]( int16_t y ){ return scale ? w.toVolts( y ) * scale : y; } );
    } else {
        std::transform( w.begin<int32_t>(), w.end<int32_t>(), v.begin()
                        , [&]( int32_t y ){ return scale ? w.toVolts( y ) * scale : y; } );
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
                adcontrols::waveform_filter::fft4c::bandpass_filter( v.size(), v.data()
                                                                     , w.meta_.xIncrement, m.hCutoffHz, m.lCutoffHz );
            else
                adcontrols::waveform_filter::fft4g::bandpass_filter( v.size(), v.data()
                                                                     , w.meta_.xIncrement, m.hCutoffHz, m.lCutoffHz );
        }
        
        return true;
    }

    return false;
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

bool
waveform::translate( adcontrols::MassSpectrum& sp, const threshold_result& result, mass_assignor_t assign, int scale )
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

//static
bool
waveform::translate( adcontrols::MassSpectrum& sp, const waveform& waveform, int scale )
{
    sp.setCentroid( adcontrols::CentroidNone );

    const adcontrols::TofProtocol * this_protocol( 0 );
    double ext_trig_delay( 0 );
    if ( waveform.method_.protocols().size() > waveform.method_.protocolIndex() ) {
        this_protocol = &waveform.method_.protocols() [ waveform.method_.protocolIndex() ];
        ext_trig_delay = this_protocol->delay_pulses() [ adcontrols::TofProtocol::EXT_ADC_TRIG ].first;
    }
    
    adcontrols::MSProperty prop = sp.getMSProperty();
    int mode = ( this_protocol == nullptr ) ? 0 : this_protocol->mode();
    //double zhalf = waveform.meta_.initialXOffset < 0 ? (-0.5) : 0.5;

    double delayTime = waveform.meta_.initialXOffset + ext_trig_delay;

    adcontrols::SamplingInfo info( waveform.meta_.xIncrement
                                   , delayTime
                                   , int32_t( delayTime / waveform.meta_.xIncrement )
                                   , uint32_t( waveform.size() )
                                   , waveform.meta_.actualAverages
                                   , mode );

    prop.setAcceleratorVoltage( 3000 );
    prop.setSamplingInfo( info );
    prop.setTDelay(ext_trig_delay + waveform.meta_.initialXOffset);
    
    prop.setTimeSinceInjection( waveform.meta_.initialXTimeSeconds );
    prop.setTimeSinceEpoch( waveform.timeSinceEpoch_ ); // nanoseconds
    prop.setDataInterpreterClsid( "u5303a" );
    
    if ( this_protocol )
        prop.setTofProtocol( *this_protocol );
    const device_data data( *waveform.ident_, waveform.meta_ );
    std::string ar;
    adportable::binary::serialize<>()( data, ar );
    prop.setDeviceData( ar.data(), ar.size() );
#if ! defined NDEBUG && 0
    ADDEBUG() << "===== device_data =====\nIdentifier:\t " << waveform.ident_->Identifier()
              << "\nRevision:\t" << waveform.ident_->Revision()
              << "\nVendor:\t" << waveform.ident_->Vendor()
              << "\nDescription:\t" << waveform.ident_->Description()
              << "\nInstrumentModel:\t" << waveform.ident_->InstrumentModel()
              << "\nFirmwareRevision:\t" << waveform.ident_->FirmwareRevision()
              << "\nSerialNumber:\t" << waveform.ident_->SerialNumber()
              << "\nOptions:\t" << waveform.ident_->Options()
              << "\nIOVersion\t" << waveform.ident_->IOVersion()
              << "\nNbrADCBits\t" << waveform.ident_->NbrADCBits();
#endif

    // prop.setDeviceData(); TBA
    sp.setMSProperty( prop );
    sp.resize( waveform.size() );
    sp.setProtocol( waveform.method_.protocolIndex(), waveform.method_.protocols().size() );

    // waveform_copy
    // normalize waveform to volts/millivolts scale with respect to actual number of average

    if ( waveform.meta_.actualAverages == 0 ) { // digitizer mode data
		switch( waveform.meta_.dataType ) {
        case 2:
			waveform_copy<int16_t>()( sp, waveform, scale );
            break;
        case 4:
            waveform_copy<int32_t>()( sp, waveform, scale );
            break;
        case 8:
			waveform_copy<int64_t>()( sp, waveform, scale );
            break;
        default:
            assert(0);
		}
    } else {
        double dbase(0), rms(0);
        switch( waveform.meta_.dataType ) {
        case 4:
            adportable::spectrum_processor::tic( waveform.size(), waveform.begin<int32_t>(), dbase, rms );
            waveform_copy<int32_t>()( sp, waveform, scale, dbase );
            break;
        case 8:
            adportable::spectrum_processor::tic( waveform.size(), waveform.begin<int64_t>(), dbase, rms );
            waveform_copy<int64_t>()( sp, waveform, scale, dbase );
            break;
        default:
            assert(0);
        }
    }
	return true;
}

//static
bool
waveform::translate( adcontrols::MassSpectrum& sp, const threshold_result& result, int scale )
{
    if ( result.data() == nullptr )
    	return false;

    sp.setCentroid( adcontrols::CentroidNone );
    const waveform& waveform = *result.data();

    translate( sp, waveform, scale );

	int idx = 0;
    if ( result.processed().size() == waveform.size() ) { // has filterd waveform
        // overwrite intensity array
        if ( scale <= 1 )
            sp.setIntensityArray( result.processed().data() );
        else
            for ( auto it = result.processed().begin(); it != result.processed().end(); ++it )
                sp.setIntensity( idx++, *it * scale ); // Volts -> mV (where scale = 1000)
    }

	return true;
}

bool
waveform::isDEAD() const
{
    size_t count( 99 );
	if ( meta_.dataType == 2 ) {
		for ( auto it = begin<int16_t>(); it != end<int16_t>() && count--; ++it )
			if ( !( *it == 0 || *it == int16_t(0xdead) ) )
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

//////////////// int8_t -- does not exist -- ////////////////
template<> const int8_t *
waveform::begin() const
{
    throw std::bad_cast();
}

template<> const int8_t *
waveform::end() const
{
    throw std::bad_cast();
}

//////////////// int16_t ////////////////
template<> const int16_t *
waveform::begin() const
{
    if ( mblock_.which() == 0 ) {
        auto&& mblk = boost::get < std::shared_ptr< adportable::mblock<int16_t> > >( mblock_ );
        return mblk->data() + firstValidPoint_;
    }
    throw std::bad_cast();
}

template<> const int16_t *
waveform::end() const
{
    if ( mblock_.which() == 0 ) {
        auto&& mblk = boost::get < std::shared_ptr< adportable::mblock<int16_t> > >( mblock_ );
        return mblk->data() + firstValidPoint_ + meta_.actualPoints;
    }
    throw std::bad_cast();
}

template<> const int16_t *
waveform::data() const
{
    if ( mblock_.which() == 0 ) {
        auto&& mblk = boost::get < std::shared_ptr< adportable::mblock<int16_t> > >( mblock_ );
        return mblk->data() + firstValidPoint_;
    }
    throw std::bad_cast();
}

template<> int16_t *
waveform::data()
{
    if ( mblock_.which() == 0 ) {
        auto&& mblk = boost::get < std::shared_ptr< adportable::mblock<int16_t> > >( mblock_ );
        return mblk->data() + firstValidPoint_;
    }
    throw std::bad_cast();    
}

//////////////// int32_t ////////////////
template<> const int32_t *
waveform::begin() const
{
    if ( mblock_.which() == 1 ) {
        auto&& mblk = boost::get < std::shared_ptr< adportable::mblock<int32_t> > >( mblock_ );
        return mblk->data() + firstValidPoint_;
    }
    throw std::bad_cast();
}

template<> const int32_t *
waveform::end() const
{
    if ( mblock_.which() == 1 ) {
        auto&& mblk = boost::get < std::shared_ptr< adportable::mblock<int32_t> > >( mblock_ );
        return mblk->data() + firstValidPoint_ + meta_.actualPoints;
    }
    throw std::bad_cast();
}


template<> const int32_t *
waveform::data() const
{
    if ( mblock_.which() == 1 ) {
        auto&& mblk = boost::get < std::shared_ptr< adportable::mblock<int32_t> > >( mblock_ );
        return mblk->data() + firstValidPoint_;
    }
    throw std::bad_cast();        
}

template<> int32_t *
waveform::data()
{
    if ( mblock_.which() == 1 ) {
        auto&& mblk = boost::get < std::shared_ptr< adportable::mblock<int32_t> > >( mblock_ );
        return mblk->data() + firstValidPoint_;
    }
    throw std::bad_cast();        
}

//////////////// int64_t ////////////////
template<> const int64_t *
waveform::begin() const
{
    if ( mblock_.which() == 2 ) {
        auto&& mblk = boost::get < std::shared_ptr< adportable::mblock< int64_t > > >( mblock_ );
        return mblk->data() + firstValidPoint_;
    }
    throw std::bad_cast();
}

template<> const int64_t *
waveform::end() const
{
    if ( mblock_.which() == 2 ) {
        auto&& mblk = boost::get < std::shared_ptr< adportable::mblock< int64_t > > >( mblock_ );
        return mblk->data() + firstValidPoint_ + meta_.actualPoints;
    }
    throw std::bad_cast();
}

template<> const int64_t *
waveform::data() const
{
    if ( mblock_.which() == 2 ) {
        auto&& mblk = boost::get < std::shared_ptr< adportable::mblock< int64_t > > >( mblock_ );
        return mblk->data() + firstValidPoint_;
    }
    throw std::bad_cast();        
}

template<> int64_t *
waveform::data()
{
    if ( mblock_.which() == 2 ) {
        auto&& mblk = boost::get < std::shared_ptr< adportable::mblock< int64_t > > >( mblock_ );
        return mblk->data() + firstValidPoint_;
    }
    throw std::bad_cast();        
}

