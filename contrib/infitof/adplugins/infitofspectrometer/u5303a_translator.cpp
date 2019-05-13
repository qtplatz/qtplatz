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

#include "u5303a_translator.hpp"
#include "infitofdatainterpreter.hpp"
#include "constants.hpp"
#include <multumcontrols/scanlaw.hpp>
#include <infitofdefns/avgrdata.hpp>

#include <adportable/debug.hpp>
#include <adportable/binary_serializer.hpp>
#include <adportable/spectrum_processor.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/mscalibration.hpp>
#include <adcontrols/traceaccessor.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <boost/format.hpp>
#include <string>
#include <sstream>
#include <memory>

using namespace infitofspectrometer;

u5303a_translator::u5303a_translator()
{
}

adcontrols::translate_state
u5303a_translator::translate( adcontrols::MassSpectrum& ms
                             , const infitof::AveragerData& avgr
                             , size_t idData
                             , const adcontrols::MassSpectrometer& inst )
{
    if ( idData > 0 ) // this never hold more than two spectra in dataReadBuffer
        return adcontrols::translate_error;

    std::ostringstream title;
    title << boost::format("Spectrum pos[%1%]") % avgr.npos;

    const infitof::u5303a::Descriptors * udesc = 0;
    try {
        const infitof::u5303a::Descriptors& t
            = boost::get< infitof::u5303a::Descriptors >( avgr.desc );
        udesc = &t;
    } catch ( std::exception& ex ) {
        adportable::debug( __FILE__, __LINE__ ) << "Exception: " << ex.what();
        return adcontrols::translate_error;
    }

    multumcontrols::infitof::ScanLaw scanLaw( avgr.kAcceleratorVoltage, avgr.tDelay );

    unsigned long wellKnownEvents = avgr.wellKnownEvents;
    (void)wellKnownEvents;
    double delay = ( avgr.sampInterval * 1.0e-12 ) * avgr.nSamplingDelay;

    adcontrols::SamplingInfo info( avgr.sampInterval * 1.0e-12, delay, avgr.nSamplingDelay, avgr.nbrSamples, avgr.nbrAverage, 0 );

    adcontrols::MSProperty prop;
    prop.setDataInterpreterClsid( constants::dataInterpreter::spectrometer::utf8_name() );
    prop.setTimeSinceInjection( avgr.timeSinceInject, adcontrols::metric::micro );

    title << boost::format( "#%1%[%2%]" ) % avgr.npos % avgr.protocolId;

    int sid = 0;
    if ( avgr.protocolId == 0 ) {
        ms.resize( 0 );
        ms.clearSegments();
    } else {
        if ( ms.size() != 0 || ms.numSegments() != 0 ) { // has attached segment(s)
            while ( ( avgr.protocolId - 1 ) >= ms.numSegments() )
                ms << std::make_shared< adcontrols::MassSpectrum >();
            sid = avgr.protocolId;
        }
    }

    adcontrols::MassSpectrum& fgms = ( sid == 0 ) ? ms : ms.getSegment( sid - 1 );
    fgms.addDescription( adcontrols::description( { "acquire.title", title.str() } ) );
    fgms.setProtocol( avgr.protocolId, avgr.nProtocols );

    fgms.resize( avgr.nbrSamples );
    const infitof::OrbitProtocol& proto = avgr.protocol_;

    // set device specific data
    std::string device;
    if ( adportable::binary::serialize<>()( proto, device ) )
        prop.setDeviceData( device.data(), device.size() );

    // set sampling data, and set property to spectrum
    prop.setAcceleratorVoltage( avgr.kAcceleratorVoltage );
    prop.setTDelay( avgr.tDelay );
    info.setMode( avgr.protocol_.nlaps_ );
    prop.setSamplingInfo( info );
    fgms.setMSProperty( prop );

    //int mode = nTurns == 0 ? 0 : 1;
    const adcontrols::MSCalibration * calib = inst.findCalibration( info.mode() == 0 ? 0 : 1 );
    if ( calib == 0 )
        calib = inst.findCalibration( info.mode() == 0 ? 1 : 0 );  // take 2nd best

    using namespace adcontrols::metric;

    if ( const int32_t * parray = avgr.waveform.data() ) {
        double dbase = parray[ 0 ], sd = 0;
        // either calculate dbase or take minimum in array ??? for baseline to zero
        adportable::spectrum_processor::tic( avgr.nbrSamples, parray, dbase, sd );

#if defined DEBUG && 0
        adportable::debug(__FILE__, __LINE__) << "exit delay: " << adcontrols::metric::scale_to_micro( exitDelay )
            << " m/z: " << proto.lower_mass
            << " lap: " <<  lap
            << " nTurns: " << nTurns
            << " dbase: " << dbase
            << " calib: " << ( calib ? calib->calibId() : L"n/a" );
#endif

        if ( calib ) {
            for ( uint32_t i = 0; i < avgr.nbrSamples; ++i ) {
                double tof = scale_to_base<double>( ( avgr.nSamplingDelay + i ) * avgr.sampInterval, pico );
                double x = InfiTofDataInterpreter::compute_mass( tof, avgr.protocol_.nlaps_, *calib, scanLaw );
                double y = parray[ i ] - dbase;
                fgms.setMass( i, x );
                fgms.setIntensity( i, y );
            }
        } else {
            for ( uint32_t i = 0; i < avgr.nbrSamples; ++i ) {
                double tof = udesc->meta_.initialXOffset + ( udesc->meta_.xIncrement * i );
                // double tof = scale_to_base<double>(( avgr.nSamplingDelay + i ) * avgr.sampInterval, pico );
                double x = scanLaw.getMass( tof, int( avgr.protocol_.nlaps_ ) );
                double y = parray[ i ] - dbase;
                fgms.setMass( i, x );
                fgms.setIntensity( i, y );
            }
        }
    }

    ms.setAcquisitionMassRange( ms.getMass( 0 ), fgms.getMass( fgms.size() - 1 ) );

    if ( avgr.nProtocols == 0 || avgr.protocolId == ( avgr.nProtocols - 1 ) )
        return adcontrols::translate_complete;

    return adcontrols::translate_indeterminate;
}
