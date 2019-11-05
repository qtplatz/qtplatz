/**************************************************************************
** Copyright (C) 2013-2019 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include "method.hpp"
#include "threshold_result.hpp"
#include <adcontrols/waveform_translator.hpp>
#include <adportable/debug.hpp>
#include <adportable/mblock.hpp>
#include <adportable/serializer.hpp>
#include <adportable/spectrum_processor.hpp>
#include <boost/serialization/vector.hpp>
#include <stdexcept>
#include <numeric>

namespace aqmd3controls {

    template<typename data_type> struct waveform_copy {

        void operator ()( adcontrols::MassSpectrum& sp, const waveform& w, int scale ) const {
            int idx = 0;
            if ( w.method().mode() == method::DigiMode::Digitizer ) {
                for ( auto it = w.begin<data_type>(); it != w.end<data_type>(); ++it )
                    sp.setIntensity( idx++, scale ? ( toVolts_< data_type, method::DigiMode::Digitizer >()( w.xmeta(), *it ) * scale ) : *it );
            } else {
                for ( auto it = w.begin<data_type>(); it != w.end<data_type>(); ++it )
                    sp.setIntensity( idx++, scale ? ( toVolts_< data_type, method::DigiMode::Averager >()( w.xmeta(), *it ) * scale ) : *it );
            }
        }

        void operator ()( adcontrols::MassSpectrum& sp, const waveform& w, int scale, double dbase ) const {
            int idx = 0;
            if ( w.method().mode() == method::DigiMode::Digitizer ) {
                double vb = toVolts_< data_type, method::DigiMode::Digitizer >()( w.xmeta(), dbase );
                for ( auto it = w.begin<data_type>(); it != w.end<data_type>(); ++it ) {
                    double d = scale ? ( toVolts_< data_type, method::DigiMode::Digitizer >()( w.xmeta(), *it ) - vb ) * scale : *it - dbase;
                    sp.setIntensity( idx++, d );
                }
            } else {
                double vb = toVolts_< data_type, method::DigiMode::Averager >()( w.xmeta(), dbase );
                for ( auto it = w.begin<data_type>(); it != w.end<data_type>(); ++it ) {
                    double d = scale ? ( toVolts_< data_type, method::DigiMode::Averager >()( w.xmeta(), *it ) - vb ) * scale : *it - dbase;
                    sp.setIntensity( idx++, d );
                }
            }
        }

    };

    template< typename value_type >
    struct pkd_waveform_copy {
        bool operator()( adcontrols::MassSpectrum& sp, const waveform& w ) const {
            if ( w.xmeta().channelMode == PKD ) {
                size_t sz = std::accumulate( w.begin< value_type >(), w.end< value_type >(), size_t(0)
                                             , []( size_t a, const value_type& b ) { return a + ( b > 0 ? 1 : 0 ); });
                sp.resize( sz );
                size_t idx(0);
                for ( auto it = w.begin< value_type >(); it != w.end< value_type >(); ++it ) {
                    if ( *it > 0 ) {
                        sp.setIntensity( idx, *it );
                        sp.setTime( idx, w.time( std::distance( w.begin< value_type >(), it ) ) );
                        ++idx;
                    }
                }
                sp.setCentroid( adcontrols::CentroidHistogram );
                return true;
            }
            return false;
        }
    };

}

using namespace aqmd3controls;

waveform::waveform()
{
}

waveform::waveform( const waveform& t )
    : basic_waveform< waveform::value_type, waveform::meta_type >( t )
    , trigger_delay_( t.trigger_delay_ )
    , is_pkd_( t.is_pkd_ )
{
}

waveform::waveform( uint32_t pos
                    , uint32_t fcn
                    , uint32_t serialnumber
                    , uint32_t wellKnownEvents
                    , uint64_t timepoint
                    , uint64_t elapsed_time
                    , uint64_t epoch_time )
{
    pos_ = pos;
    pn_ = fcn;
    serialnumber_ = serialnumber;
    wellKnownEvents_ = wellKnownEvents;
    timepoint_ = timepoint;
    elapsed_time_ = elapsed_time;
    epoch_time_ = epoch_time;
    //
    trigger_delay_ = 0;
}

waveform::waveform( uint32_t pos, const meta_data& meta )
{
    pos_ = pos;
    set_xmeta( meta );
    // set_trigger_delay( meta.trigger_delay() );
}

waveform::waveform( std::shared_ptr< const identify > id
                    , uint32_t pos, uint32_t events, uint64_t tp )// : serialnumber_( pos )
                                                                   // , wellKnownEvents_( events )
                                                                   // , timeSinceEpoch_( tp )
                                                                   // , firstValidPoint_( 0 )
                                                                   // , timeSinceInject_( 0.0 )
                                                                   // , ident_( id )
                                                                   // , hasTic_( false )
                                                                   // , tic_( 0 )
                                                                   // , dbase_( 0 )
// , rms_( 0 )
{
}

void
waveform::set_xmeta( const meta_data& meta )
{
    xmeta_ = meta; // set to basic_waveform

    // pn_ = meta.flags_ & 03; // LSB 2bits
    // serialnumber_ = meta.trig_number_;
    // wellKnownEvents_ = meta.flags_ & ~03;
    // timepoint_    = meta.clock_counts_;
    // elapsed_time_ = std::nano::den * double( meta.clock_counts_ ) / meta.clock_hz_; // s
    // epoch_time_   = meta.epoch_time_;

    // trigger_delay_ = meta.trigger_delay();
}

void
waveform::set_is_pkd( bool pkd )
{
    is_pkd_ = pkd;
}

bool
waveform::is_pkd() const
{
    return is_pkd_;
}

void
waveform::set_elapsed_time( uint64_t value )
{
    elapsed_time_ = value;
}

void
waveform::set_trigger_delay( double value )
{
    trigger_delay_ = value;
}

double
waveform::time( size_t idx ) const
{
    double ext_trig_delay( 0 );

    assert( method_ );

    if ( method_->protocols().size() > method_->protocolIndex() ) {
        const auto& this_protocol = method_->protocols()[ method_->protocolIndex() ];
        ext_trig_delay = this_protocol.delay_pulses() [ adcontrols::TofProtocol::EXT_ADC_TRIG ].first;
    }

    return idx * xmeta().xIncrement + xmeta().initialXOffset + ext_trig_delay;
}

// static
double
waveform::toVolts( int32_t d, size_t actual_averages )
{
    size_t n = actual_averages == 0 ? 1 : actual_averages;
    return ( double(d) * (1.9/2) / 0x7ff ) / n;
}

std::pair< double, uint64_t >
waveform::xy( uint32_t idx ) const
{
    return std::make_pair( time( idx ), toVolts( d_[ idx ] ) );
}

//static
bool
waveform::translate( adcontrols::MassSpectrum& sp, const waveform& waveform, mass_assignor_t assign, int scale )
{
    if ( translate( sp, waveform, scale ) ) {

        adcontrols::MSProperty& prop = sp.getMSProperty();
        const auto& sinfo = prop.samplingInfo();

        double lMass = assign( sinfo.fSampDelay(), prop.mode() );
        double hMass = assign( sinfo.fSampDelay() + sinfo.fSampInterval() * sinfo.nSamples(), prop.mode() );
        prop.setInstMassRange( std::make_pair( lMass, hMass ) );

        sp.setAcquisitionMassRange( lMass, hMass );

        return sp.assign_masses( assign );
    }

    return false;
}

bool
waveform::translate( adcontrols::MassSpectrum& sp, const threshold_result& result, mass_assignor_t assign, int scale )
{
    if ( translate( sp, result, scale ) ) {

        adcontrols::MSProperty& prop = sp.getMSProperty();
        const auto& sinfo = prop.samplingInfo();

        double lMass = assign( sinfo.fSampDelay(), prop.mode() );
        double hMass = assign( sinfo.fSampDelay() + sinfo.fSampInterval() * sinfo.nSamples(), prop.mode() );
        prop.setInstMassRange( std::make_pair( lMass, hMass ) );
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
    if ( waveform.method_->protocols().size() > waveform.method_->protocolIndex() ) {
        this_protocol = &waveform.method_->protocols() [ waveform.method_->protocolIndex() ];
        ext_trig_delay = this_protocol->delay_pulses() [ adcontrols::TofProtocol::EXT_ADC_TRIG ].first;
    }

    adcontrols::MSProperty prop = sp.getMSProperty();
    int mode = ( this_protocol == nullptr ) ? 0 : this_protocol->mode();

    double delayTime = waveform.xmeta_.initialXOffset + ext_trig_delay;

    adcontrols::SamplingInfo info( waveform.xmeta_.xIncrement
                                   , delayTime
                                   , int32_t( delayTime / waveform.xmeta_.xIncrement )
                                   , uint32_t( waveform.size() )
                                   , waveform.xmeta_.actualAverages
                                   , mode );

    prop.setAcceleratorVoltage( 3000 );
    prop.setSamplingInfo( info );
    prop.setTDelay(ext_trig_delay + waveform.xmeta_.initialXOffset);

    //prop.setTimeSinceInjection( waveform.meta_.initialXTimeSeconds );
    prop.setTimeSinceInjection( double(waveform.elapsed_time()) / std::nano::den );
    prop.setTimeSinceEpoch( waveform.epoch_time() ); // nanoseconds
    // prop.setDataInterpreterClsid( "u5303a" );

    if ( this_protocol )
        prop.setTofProtocol( *this_protocol );
    // --- TODO --->
    //const device_data data( *waveform.ident_, waveform.meta_ );
    //std::string ar;
    //adportable::binary::serialize<>()( data, ar );
    //prop.setDeviceData( ar.data(), ar.size() );

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
    sp.setProtocol( waveform.method().protocolIndex(), waveform.method().protocols().size() );

    // waveform_copy
    // normalize waveform to volts/millivolts scale with respect to actual number of average

    if ( waveform.xmeta_.actualAverages == 0 ) { // digitizer mode data
		switch( waveform.xmeta_.dataType ) {
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
            ADDEBUG() << "ERROR: Unexpected data type in waveform";
		}
    } else {
        if ( waveform.xmeta().channelMode == PKD ) {
            switch( waveform.xmeta().dataType ) {
            case 4:
                pkd_waveform_copy< int32_t >()( sp, waveform );
                break;
            case 8:
                pkd_waveform_copy< int64_t >()( sp, waveform );
                break;
            default:
                ADDEBUG() << "ERROR: Unexpected data type in waveform";
            }
        } else {
            double dbase(0), rms(0);
            switch( waveform.xmeta().dataType ) {
            case 4:
                adportable::spectrum_processor::tic( waveform.size(), waveform.begin<int32_t>(), dbase, rms );
                waveform_copy<int32_t>()( sp, waveform, scale, dbase );
                break;
            case 8:
                adportable::spectrum_processor::tic( waveform.size(), waveform.begin<int64_t>(), dbase, rms );
                waveform_copy<int64_t>()( sp, waveform, scale, dbase );
                break;
            default:
                ADDEBUG() << "ERROR: Unexpected data type in waveform";
            }
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

/***
bool
waveform::operator += ( const waveform& rhs )
{
    if ( size() != rhs.size() ) {
        basic_waveform< waveform::value_type, waveform::meta_type >{ rhs };
        //trigger_delay_ = rhs.trigger_delay_;
        // throw std::invalid_argument( "size error" );
        return true;
    }

    xmeta_.actual_averages_ += rhs.xmeta_.actual_averages_ + 1;

    std::transform( rhs.begin(), rhs.end(), begin(), begin(), std::plus<int>() );

    return true;
}
***/

size_t
waveform::serialize_xmeta( std::string& ar ) const
{
    ar.clear();
    if ( adportable::serializer< meta_data >::serialize( xmeta_, ar ) )
        return ar.size();
    return 0;
}

bool
waveform::deserialize_xmeta( const char * data, size_t size )
{
    return adportable::serializer< meta_data >::deserialize( xmeta_, data, size );
}

size_t
waveform::serialize_xdata( std::string& ar ) const
{
    ar.clear();
    if ( adportable::serializer< std::vector< int32_t > >::serialize( d_, ar ) )
        return ar.size();
    return 0;
}

bool
waveform::deserialize_xdata( const char * data, size_t size )
{
    return adportable::serializer< std::vector< int32_t > >::deserialize( d_, data, size );
}

///////////////////////
void
waveform::setData( const std::shared_ptr< adportable::mblock<int32_t> >& mblk, size_t firstValidPoint )
{
    mblock_ = mblk;
    xmeta().firstValidPoint = firstValidPoint;
}

void
waveform::setData( const std::shared_ptr< adportable::mblock<int16_t> >& mblk, size_t firstValidPoint )
{
    mblock_ = mblk;
    xmeta().firstValidPoint = firstValidPoint;
}

void
waveform::set_method( const aqmd3controls::method& m )
{
    method_ = std::make_unique< aqmd3controls::method >(m);
}

const aqmd3controls::method&
waveform::method() const
{
    return *method_;
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
        return mblk->data() + xmeta_.firstValidPoint;
    }
    throw std::bad_cast();
}

template<> const int16_t *
waveform::end() const
{
    if ( mblock_.which() == 0 ) {
        auto&& mblk = boost::get < std::shared_ptr< adportable::mblock<int16_t> > >( mblock_ );
        return mblk->data() + xmeta_.firstValidPoint + xmeta_.actualPoints;
    }
    throw std::bad_cast();
}

template<> const int16_t *
waveform::data() const
{
    if ( mblock_.which() == 0 ) {
        auto&& mblk = boost::get < std::shared_ptr< adportable::mblock<int16_t> > >( mblock_ );
        return mblk->data() + xmeta_.firstValidPoint;
    }
    throw std::bad_cast();
}

template<> int16_t *
waveform::data()
{
    if ( mblock_.which() == 0 ) {
        auto&& mblk = boost::get < std::shared_ptr< adportable::mblock<int16_t> > >( mblock_ );
        return mblk->data() + xmeta_.firstValidPoint;
    }
    throw std::bad_cast();
}

//////////////// int32_t ////////////////
template<> const int32_t *
waveform::begin() const
{
    if ( mblock_.which() == 1 ) {
        auto&& mblk = boost::get < std::shared_ptr< adportable::mblock<int32_t> > >( mblock_ );
        return mblk->data() + xmeta_.firstValidPoint;
    }
    throw std::bad_cast();
}

template<> const int32_t *
waveform::end() const
{
    if ( mblock_.which() == 1 ) {
        auto&& mblk = boost::get < std::shared_ptr< adportable::mblock<int32_t> > >( mblock_ );
        return mblk->data() + xmeta_.firstValidPoint + xmeta_.actualPoints;
    }
    throw std::bad_cast();
}


template<> const int32_t *
waveform::data() const
{
    if ( mblock_.which() == 1 ) {
        auto&& mblk = boost::get < std::shared_ptr< adportable::mblock<int32_t> > >( mblock_ );
        return mblk->data() + xmeta_.firstValidPoint;
    }
    throw std::bad_cast();
}

template<> int32_t *
waveform::data()
{
    if ( mblock_.which() == 1 ) {
        auto&& mblk = boost::get < std::shared_ptr< adportable::mblock<int32_t> > >( mblock_ );
        return mblk->data() + xmeta_.firstValidPoint;
    }
    throw std::bad_cast();
}

//////////////// int64_t ////////////////
template<> const int64_t *
waveform::begin() const
{
    if ( mblock_.which() == 2 ) {
        auto&& mblk = boost::get < std::shared_ptr< adportable::mblock< int64_t > > >( mblock_ );
        return mblk->data() + xmeta_.firstValidPoint;
    }
    throw std::bad_cast();
}

template<> const int64_t *
waveform::end() const
{
    if ( mblock_.which() == 2 ) {
        auto&& mblk = boost::get < std::shared_ptr< adportable::mblock< int64_t > > >( mblock_ );
        return mblk->data() + xmeta_.firstValidPoint + xmeta_.actualPoints;
    }
    throw std::bad_cast();
}

template<> const int64_t *
waveform::data() const
{
    if ( mblock_.which() == 2 ) {
        auto&& mblk = boost::get < std::shared_ptr< adportable::mblock< int64_t > > >( mblock_ );
        return mblk->data() + xmeta_.firstValidPoint;
    }
    throw std::bad_cast();
}

template<> int64_t *
waveform::data()
{
    if ( mblock_.which() == 2 ) {
        auto&& mblk = boost::get < std::shared_ptr< adportable::mblock< int64_t > > >( mblock_ );
        return mblk->data() + xmeta_.firstValidPoint;
    }
    throw std::bad_cast();
}
