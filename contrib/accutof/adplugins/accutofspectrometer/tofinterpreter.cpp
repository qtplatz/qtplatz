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
#include <tofinterface/tofdata.hpp>
#include <tofinterface/tofprocessed.hpp>
#include <tofinterface/serializer.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/traceaccessor.hpp>
#include <adinterface/signalobserverC.h>
#include <adportable/debug.hpp>
#include <boost/assert.hpp>
#include <memory>
#include <sstream>

using namespace tofspectrometer;

tofInterpreter::tofInterpreter()
{
}


adcontrols::translate_state
tofInterpreter::translate( adcontrols::MassSpectrum& ms
                           , const char * data, size_t dsize
                           , const char * meta, size_t msize
                           , const adcontrols::MassSpectrometer&
                           , size_t idData ) const
{
	(void)meta;
	(void)msize;

	tofinterface::tofDATA tofdata;

	if ( tofinterface::serializer::deserialize( tofdata, data, dsize ) ) {
        
        if ( tofdata.numberOfProfiles() > idData ) {

            tofinterface::tofDATA::datum& datum = tofdata.data()[ 0 ];
            const size_t ndata = datum.values().size();

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
            o << L"TOF data";
            ms.addDescription( adcontrols::Description( L"title", o.str() ) );

            ms.resize( ndata );
            size_t idx = 0;
            for ( const auto& iy: datum.values() ) {
                ms.setIntensity( idx, iy );
                ms.setMass( idx, double( idx ) * 512.0 / 65536 );
                ++idx;
            }
            ms.setAcquisitionMassRange( ms.getMass( 0 ), ms.getMass( idx - 1 ) );
			return adcontrols::translate_complete;
        }
    }
	return adcontrols::translate_error; // no more data
}

adcontrols::translate_state
tofInterpreter::translate( adcontrols::TraceAccessor& accessor
                           , const char * data, size_t dsize
                           , const char * meta, size_t msize, unsigned long events ) const
{
	(void)meta;
	(void)msize;

    std::vector< tofinterface::tofProcessedData > vec;

	if ( tofinterface::serializer::deserialize( vec, data, dsize ) ) {
        for ( const auto d: vec ) {
            adcontrols::seconds_t sec( double( d.uptime ) * 1.0e-6 );
            accessor.push_back( d.fcn, d.npos, sec, d.tic, events );
        }
        return adcontrols::translate_complete;
    }
	return adcontrols::translate_error;
}


