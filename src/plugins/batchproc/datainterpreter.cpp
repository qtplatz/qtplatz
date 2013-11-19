/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "datainterpreter.hpp"
#include "importdata.hpp"
#include "massspectrometer.hpp"
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/traceaccessor.hpp>
#include <adfs/cpio.hpp>
#include <adportable/bzip2.hpp>
#include <adportable/serializer.hpp>

using namespace batchproc;

DataInterpreter::DataInterpreter()
{
}

adcontrols::translate_state
DataInterpreter::translate( adcontrols::MassSpectrum& ms
                            , const char * data, size_t dsize
                            , const char * meta, size_t msize
                            , const adcontrols::MassSpectrometer& spectrometer
                            , size_t idData
							, int fcn ) const
{
    import_profile profile;
    import_continuum_massarray ma;
    
    const batchproc::MassSpectrometer* pSpectrometer = dynamic_cast< const batchproc::MassSpectrometer * >( &spectrometer );
    if ( pSpectrometer == 0 )
        return adcontrols::translate_error;

    if ( adportable::bzip2::is_a( data, dsize ) ) {

        std::string ar;
        adportable::bzip2::decompress( ar, data, dsize );

        if ( ! adportable::serializer< import_profile >::deserialize( profile, ar.data(), ar.size() ) )
            return adcontrols::translate_error;

    } else {

        if ( ! adportable::serializer< import_profile >::deserialize( profile, data, dsize ) ) 
            return adcontrols::translate_error;

    }
    
    if ( meta && msize ) {
        if ( adportable::bzip2::is_a( meta, msize ) ) {
            std::string ar;
            adportable::bzip2::decompress( ar, meta, msize );
            if ( ! adportable::serializer< import_continuum_massarray >::deserialize( ma, ar.data(), ar.size() ) )
                return adcontrols::translate_error;
        } else {
            if ( ! adportable::serializer< import_continuum_massarray >::deserialize( ma, meta, msize ) )
                return adcontrols::translate_error;
        }
    }

    const import_continuum_massarray& continuum_massarray = meta ? ma : pSpectrometer->continuum_massarray();
    
	ms.setMSProperty( profile.prop_ );
	ms.setPolarity( profile.polarity_ );
    ms.resize( profile.intensities_.size() );

    ms.setMassArray( continuum_massarray.masses_.data() );
    auto intens = profile.intensities_.data();
    for ( size_t i = 0; i < ms.size(); ++i )
        ms.setIntensity( i, *intens++ );

    ms.setAcquisitionMassRange( ms.getMass( 0 ), ms.getMass( ms.size() - 1 ) );

    return adcontrols::translate_complete;
}

adcontrols::translate_state
DataInterpreter::translate( adcontrols::TraceAccessor& trace
                            , const char * data, size_t dsize
                            , const char * meta, size_t msize, unsigned long events ) const
{
	adcontrols::Chromatogram c;

    if ( adportable::bzip2::is_a( data, dsize ) ) {
        std::string ar;
        adportable::bzip2::decompress( ar, data, dsize );
        if ( !adfs::cpio< adcontrols::Chromatogram >::deserialize( c, ar.data(), ar.size() ) )
            return adcontrols::translate_error;
    } else
        if ( ! adfs::cpio< adcontrols::Chromatogram >::deserialize( c, data, dsize ) )
            return adcontrols::translate_error;

    const double * intens = c.getIntensityArray();
    for ( size_t i = 0; i < c.size(); ++i )
        trace.push_back( 0, i, c.timeFromDataIndex(i), intens[i], 0 );

	return adcontrols::translate_complete;
}

