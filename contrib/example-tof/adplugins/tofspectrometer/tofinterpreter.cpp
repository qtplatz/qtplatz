/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#include "tofinterpreter.hpp"
#include <tofinterface/signalC.h>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/traceaccessor.hpp>
#include <adinterface/signalobserverC.h>
#include <boost/assert.hpp>
#include <memory>
#include <sstream>

using namespace tofspectrometer;

tofInterpreter::tofInterpreter()
{
}

bool
tofInterpreter::translate( adcontrols::MassSpectrum& ms
                               , const SignalObserver::DataReadBuffer& rb
                               , const adcontrols::MassSpectrometer&
                               , size_t idData ) const
{
    TOFSignal::tofDATA * data = 0;

    if ( rb.data >>= data ) {

        if ( data->numberOfProfiles > idData ) {
            TOFSignal::datum& datum = data->data[ 0 ];
            const size_t ndata = datum.values.length();

            adcontrols::MSProperty prop;
            std::vector< adcontrols::MSProperty::SamplingInfo > sampInfo;
            adcontrols::MSProperty::SamplingInfo info;
            
            info.sampInterval = 1000000;
            info.nSamplingDelay = 0;
            info.nSamples = ndata;
            info.nAverage = 1;
            sampInfo.push_back( info );

            prop.setSamplingInfo( sampInfo );
            prop.setInstSamplingInterval( 1000 );
            prop.setInstSamplingStartDelay( 0 );
            prop.setTimeSinceInjection( 0 ); // uptime

            ms.setMSProperty( prop );

            std::wostringstream o;
            o << L"TOF data id: " << rb.pos;
            ms.addDescription( adcontrols::Description( L"title", o.str() ) );

            ms.resize( ndata );
			std::unique_ptr< double [] > pX( new double [ ndata ] );
            std::unique_ptr< double [] > pY( new double [ ndata ] );
            for ( size_t i = 0; i < ndata; ++i ) {
                pY[ i ] = datum.values[ i ];
                pX[ i ] = double( i ) * 512.0 / 65536 ; // scanLaw.getMass
            }
            ms.setMassArray( pX.get(), true ); // update acq range
            ms.setIntensityArray( pY.get() );
            return true;
        }
    }
	return false; // no more data
}

bool
tofInterpreter::translate( adcontrols::TraceAccessor& accessor
                         , const SignalObserver::DataReadBuffer& rb ) const
{
    unsigned long events = rb.events;

    accessor.pos( rb.pos );

    TOFSignal::SpectrumProcessedDataArray * ar;
    if ( rb.data >>= ar ) {
        size_t ndata = ar->length();
        for ( size_t i = 0; i < ndata; ++i ) {
            TOFSignal::SpectrumProcessedData& d = (*ar)[i];
            adcontrols::seconds_t sec( double( d.uptime ) * 1.0e-6 );
            accessor.push_back( d.tic, events, sec );
        }
        return true;
    }
	return false;
}


