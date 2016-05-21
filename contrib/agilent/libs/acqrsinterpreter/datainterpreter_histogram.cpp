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

#include "datainterpreter_histogram.hpp"
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/timedigitalhistogram.hpp>
#include <adcontrols/waveform.hpp>
#include <adportable/debug.hpp>
#include <adportable/serializer.hpp>
#include <adportable/bzip2.hpp>

using namespace acqrsinterpreter::histogram;

DataInterpreter::~DataInterpreter()
{
}

DataInterpreter::DataInterpreter()
{
}

adcontrols::translate_state
DataInterpreter::translate( adcontrols::MassSpectrum& ms
                            , const char * data, size_t dsize
                            , const char * meta, size_t msize
                            , const adcontrols::MassSpectrometer& spectrometer
                            , size_t idData
                            , const wchar_t * traceId ) const
{
    if ( dsize > 0 ) {

        adcontrols::TimeDigitalHistogram histogram;
        

        boost::iostreams::basic_array_source< char > source( data, dsize );
        boost::iostreams::stream< boost::iostreams::basic_array_source< char > > is( source );

        if ( adcontrols::TimeDigitalHistogram::restore( is, histogram ) ) {

            if ( adcontrols::TimeDigitalHistogram::translate( ms, histogram, [=](double t, int m){ return 0; } ) ) {
                spectrometer.assignMasses( ms );
                return adcontrols::translate_complete;

            }
        }

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
    auto native = std::make_shared< adcontrols::TimeDigitalHistogram >();
    waveform = native;

    if ( data && dsize ) {

        if ( adportable::bzip2::is_a( reinterpret_cast<const char *>( data ), dsize ) ) {

            std::string ar;            
            adportable::bzip2::decompress( ar, reinterpret_cast<const char *>( data ), dsize );
            adportable::serializer< adcontrols::TimeDigitalHistogram >::deserialize( *native, ar.data(), ar.size() );

        } else {

            adportable::serializer< adcontrols::TimeDigitalHistogram >::deserialize( *native, reinterpret_cast<const char *>( data ), dsize );

        }
        return adcontrols::translate_complete;
    }

    return adcontrols::translate_error;
}
