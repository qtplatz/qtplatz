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
#include <tuple>

namespace aqmd3controls {

    template<typename data_type> struct waveform_copy {

        void operator ()( adcontrols::MassSpectrum& sp, const waveform& w, int scale ) const {
            int idx = 0;
            if ( auto m = w.method() ) {
                if ( m->mode() == method::DigiMode::Digitizer ) {
                    for ( auto it = w.begin(); it != w.end(); ++it )
                        sp.setIntensity( idx++, scale ? ( toVolts_< data_type, method::DigiMode::Digitizer >()( w.xmeta(), *it ) * scale ) : *it );
                } else {
                    for ( auto it = w.begin(); it != w.end(); ++it )
                        sp.setIntensity( idx++, scale ? ( toVolts_< data_type, method::DigiMode::Averager >()( w.xmeta(), *it ) * scale ) : *it );
                }
            }
        }

        void operator ()( adcontrols::MassSpectrum& sp, const waveform& w, int scale, double dbase ) const {
            int idx = 0;
            if ( auto m = w.method() ) {
                if ( m->mode() == method::DigiMode::Digitizer ) {
                    double vb = toVolts_< data_type, method::DigiMode::Digitizer >()( w.xmeta(), dbase );
                    for ( auto it = w.begin(); it != w.end(); ++it ) {
                        double d = scale ? ( toVolts_< data_type, method::DigiMode::Digitizer >()( w.xmeta(), *it ) - vb ) * scale : *it - dbase;
                        sp.setIntensity( idx++, d );
                    }
                } else {
                    double vb = toVolts_< data_type, method::DigiMode::Averager >()( w.xmeta(), dbase );
                    for ( auto it = w.begin(); it != w.end(); ++it ) {
                        double d = scale ? ( toVolts_< data_type, method::DigiMode::Averager >()( w.xmeta(), *it ) - vb ) * scale : *it - dbase;
                        sp.setIntensity( idx++, d );
                    }
                }
            }
        }
    };

    template< typename value_type >
    struct pkd_waveform_copy {
        bool operator()( adcontrols::MassSpectrum& sp, const waveform& w ) const {
            if ( w.xmeta().channelMode == PKD ) {
                size_t sz = std::accumulate( w.begin(), w.end(), size_t(0)
                                             , []( size_t a, const value_type& b ) { return a + ( b > 0 ? 1 : 0 ); });
                sp.resize( sz );
                size_t idx(0);
                for ( auto it = w.begin(); it != w.end(); ++it ) {
                    if ( *it > 0 ) {
                        sp.setIntensity( idx, *it );
                        sp.setTime( idx, w.time( std::distance( w.begin(), it ) ) );
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

waveform::waveform() : flag_warned_( false )
{
}

waveform::waveform( const waveform& t )
    : basic_waveform< waveform::value_type, waveform::meta_type >( t )
    , trigger_delay_( t.trigger_delay_ )
    , flag_warned_( false )
{
}

waveform::waveform( uint32_t pos
                    , uint32_t fcn
                    , uint32_t serialnumber
                    , uint32_t wellKnownEvents
                    , uint64_t timepoint
                    , uint64_t elapsed_time
                    , uint64_t epoch_time ) : flag_warned_( false )
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

waveform::waveform( uint32_t pos, const meta_data& meta ) : flag_warned_( false )
{
    pos_ = pos;
    set_xmeta( meta );
}

waveform::waveform( std::shared_ptr< const identify > /* ignore */
                    , uint32_t pos, uint32_t events, uint64_t tp ) : flag_warned_( false )
{
    set_pos( pos ); // sequential number
    set_serialnumber( pos ); // trigger number
    set_well_known_events( events );
    set_epoch_time( tp );
}

void
waveform::set_xmeta( const meta_data& meta )
{
    xmeta_ = meta; // set to basic_waveform
}

bool
waveform::is_pkd() const
{
    return xmeta_.channelMode == aqmd3controls::ChannelMode::PKD;
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

    if ( method_ ) {
        if ( method_->protocols().size() > method_->protocolIndex() ) {
            const auto& this_protocol = method_->protocols()[ method_->protocolIndex() ];
            ext_trig_delay = this_protocol.delay_pulses() [ adcontrols::TofProtocol::EXT_ADC_TRIG ].first;
        }
    } else {
        if ( !flag_warned_ ) {
            ADDEBUG() << "Warning: no method attached to waveform";
            const_cast< waveform * >(this)->flag_warned_ = true;
        }
    }
    return idx * xmeta().xIncrement + xmeta().initialXOffset + ext_trig_delay;
}

double
waveform::toVolts( int32_t d ) const
{
    if ( method_->mode() == method::DigiMode::Digitizer )
        return toVolts_<int32_t,method::DigiMode::Digitizer>()( xmeta_, d );
    else
        return toVolts_<int32_t,method::DigiMode::Averager>()( xmeta_, d );
}

std::pair< double, int64_t >
waveform::xy( uint32_t idx ) const
{
    return std::make_pair( time( idx ), d_[ idx ] );
}

int32_t
waveform::toBinary( double d ) const
{
    if ( method_->mode() == method::DigiMode::Digitizer ) {
        int actualAverages = xmeta_.actualAverages == 0 ? 1 : xmeta_.actualAverages;
        return actualAverages * ( d - xmeta_.scaleOffset ) / xmeta_.scaleFactor;
    } else {
        return ( d - xmeta_.scaleOffset ) / xmeta_.scaleFactor;
    }
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
    if ( waveform.method_ ) {
        if ( waveform.method_->protocols().size() > waveform.method_->protocolIndex() ) {
            this_protocol = &waveform.method_->protocols() [ waveform.method_->protocolIndex() ];
            ext_trig_delay = this_protocol->delay_pulses() [ adcontrols::TofProtocol::EXT_ADC_TRIG ].first;
        }
        sp.setProtocol( waveform.method_->protocolIndex(), waveform.method_->protocols().size() );
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

    // prop.setDeviceData(); TBA
    sp.setMSProperty( prop );
    sp.resize( waveform.size() );

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
            sp.setCentroid( adcontrols::CentroidNative );
        } else {
            double dbase(0), rms(0);
            std::tie( std::ignore, dbase, rms ) = adportable::spectrum_processor::tic< waveform::value_type >( waveform.size(), waveform.data() );
            waveform_copy< waveform::value_type >()( sp, waveform, scale, dbase );
        }
    }
	return true;
}

//static
bool
waveform::translate( adcontrols::MassSpectrum& sp, const threshold_result& result, int scale )
{
#if 0
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
#endif
    ADDEBUG() << "TODO";
	return true;
}

bool
waveform::operator += ( const waveform& rhs )
{
    if ( size() != rhs.size() ) {
        throw std::invalid_argument( "size error" );
        return true;
    }

    xmeta_.actualAverages += rhs.xmeta().actualAverages + 1;
    std::transform( rhs.begin(), rhs.end(), begin(), begin(), std::plus<int>() );

    return true;
}

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
waveform::setData( const int32_t * data, size_t firstValidPoint, size_t actualPoints )
{
    xmeta_.firstValidPoint = 0; // override, firstValidPoint;
    xmeta_.actualPoints = actualPoints;
    d_.resize( actualPoints );
    std::copy( data + firstValidPoint, data + firstValidPoint + actualPoints, d_.begin() );
}
void
waveform::setData( std::shared_ptr< const adportable::mblock<int32_t> > mblk, size_t firstValidPoint, size_t actualPoints )
{
    setData( mblk->data(), firstValidPoint, actualPoints );
}


void
waveform::setData( std::shared_ptr< const adportable::mblock<int16_t> > mblk, size_t firstValidPoint, size_t actualPoints )
{
    xmeta_.firstValidPoint = 0; // firstValidPoint;
    xmeta_.actualPoints = actualPoints;
    d_.resize( actualPoints );
    std::copy( mblk->data() + firstValidPoint, mblk->data() + firstValidPoint + actualPoints, d_.begin() );
}

void
waveform::set_method( const aqmd3controls::method& m )
{
    method_ = std::make_unique< aqmd3controls::method >(m);
}

const aqmd3controls::method *
waveform::method() const
{
    return method_.get();
}
