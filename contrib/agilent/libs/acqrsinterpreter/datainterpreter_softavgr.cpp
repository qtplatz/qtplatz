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

#include "datainterpreter_softavgr.hpp"
#include <acqrscontrols/u5303a/waveform.hpp>
#include <adcontrols/waveform.hpp>
#include <adportable/bzip2.hpp>
#include <adportable/debug.hpp>
#include <adportable/serializer.hpp>
#include <boost/exception/all.hpp>

using namespace acqrsinterpreter::softavgr;

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
                            , const adcontrols::MassSpectrometer&
                            , size_t idData
                            , const wchar_t * traceId ) const
{
    (void)meta;
    (void)msize;

    if ( dsize > 0 && msize > 0 ) {

        acqrscontrols::u5303a::waveform waveform;

        if ( waveform.deserialize_xmeta( meta, msize ) && waveform.deserialize_xdata( data, dsize ) ) {

            if ( acqrscontrols::u5303a::waveform::translate( ms, waveform, 1000 ) ) {

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
    return adcontrols::translate_error;
}

adcontrols::translate_state
DataInterpreter::translate( acqrsinterpreter::waveform_types& waveform, const int8_t * data, size_t dsize, const int8_t * meta, size_t msize )
{
    if ( msize && dsize ) {
        
        auto native = std::make_shared< acqrscontrols::u5303a::waveform >();
        waveform = native;

        if ( native->deserialize_xmeta( reinterpret_cast<const char *>( meta ), msize ) ) {

            if ( adportable::bzip2::is_a( reinterpret_cast<const char *>( data ), dsize ) ) {

                std::string ar;
                adportable::bzip2::decompress( ar, reinterpret_cast<const char *>( data ), dsize );
                native->deserialize_xdata( ar.data(), ar.size() );
                return adcontrols::translate_complete;

            } else {

                native->deserialize_xdata( reinterpret_cast<const char *>( data ), dsize );
                return adcontrols::translate_complete;

            }
        }

    }

    return adcontrols::translate_error;
}
