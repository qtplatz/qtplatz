/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#include "datainterpreter_timecount.hpp"
#include <acqrscontrols/u5303a/threshold_result.hpp>
#include <adcontrols/waveform.hpp>
#include <adportable/debug.hpp>
#include <adcontrols/timedigitalhistogram.hpp>
#include <adportable/serializer.hpp>
#include <adportable/bzip2.hpp>

using namespace acqrsinterpreter::timecount;

DataInterpreter::~DataInterpreter()
{
}

DataInterpreter::DataInterpreter()
{
}

adcontrols::translate_state
DataInterpreter::translate( adcontrols::MassSpectrum&
                            , const char * data, size_t dsize
                            , const char * meta, size_t msize
                            , const adcontrols::MassSpectrometer&
                            , size_t idData
                            , const wchar_t * traceId ) const
{
    (void)meta;
    (void)msize;

    if ( dsize > 0 ) {
 #if 0       
        std::unique_ptr< infitofinterface::AveragerData > avgr( new infitofinterface::AveragerData );
        
        if ( infitofinterface::serializer::deserialize( *avgr, data, dsize ) ) {

            if ( avgr->avgrType == infitofinterface::Averager_AP240 ) {

                return ap240translator::translate( ms, *avgr, idData, spectrometer );

            } else if ( avgr->avgrType == infitofinterface::Averager_ARP ) {

                return arptranslator::translate( ms, *avgr, idData, spectrometer );
            }

        }
#endif
    }
    return adcontrols::translate_error;
}

adcontrols::translate_state
DataInterpreter::translate( adcontrols::TraceAccessor&
           , const char * data, size_t dsize
           , const char * meta, size_t msize, unsigned long events ) const
{
    (void)meta;
    (void)msize;

	if ( dsize > 0 ) {
#if 0
		std::vector< infitofinterface::SpectrumProcessedData > vec;
		infitofinterface::serializer::deserialize( vec, data, dsize );
		for ( const infitofinterface::SpectrumProcessedData& d: vec ) {
			adcontrols::seconds_t sec( double( d.uptime ) * 1.0e-6 );
			accessor.push_back( d.fcn, d.npos, sec, d.tic, events );
		}
        return adcontrols::translate_complete;
#endif
	}
    return adcontrols::translate_error;
}

adcontrols::translate_state
DataInterpreter::translate( acqrsinterpreter::waveform_types& waveform, const int8_t * data, size_t dsize, const int8_t * meta, size_t msize )
{
    auto native = std::make_shared< acqrscontrols::u5303a::threshold_result >();
    waveform = native;

    if ( data && dsize ) {

        if ( adportable::bzip2::is_a( reinterpret_cast<const char *>( data ), dsize ) ) {

            std::string ar;            
            adportable::bzip2::decompress( ar, reinterpret_cast<const char *>( data ), dsize );
            native->deserialize( reinterpret_cast< const int8_t *>(ar.data()), ar.size(), meta, msize );

        } else {

            native->deserialize( data, dsize, meta, msize );

        }
        return adcontrols::translate_complete;
    }

    return adcontrols::translate_error;
}
