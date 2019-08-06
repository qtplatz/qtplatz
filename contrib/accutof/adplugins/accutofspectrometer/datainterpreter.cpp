/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
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
//#include "importdata.hpp"
#include "massspectrometer.hpp"
#include <acqrscontrols/u5303a/waveform.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/traceaccessor.hpp>
#include <adfs/cpio.hpp>
#include <adlog/logger.hpp>
#include <adportable/bzip2.hpp>
#include <adportable/debug.hpp>
#include <adportable/binary_serializer.hpp>
#include <boost/exception/all.hpp>
#include <boost/format.hpp>

namespace accutofspectrometer {
    class DataInterpreterException : public boost::exception, public std::exception {
    public:
        DataInterpreterException() { }
    };
}

using namespace accutofspectrometer;

DataInterpreter::DataInterpreter()
{
}

bool
DataInterpreter::make_device_text( std::vector< std::pair< std::string, std::string > >& textv, const adcontrols::MSProperty& prop ) const
{
    textv.clear();

    try {
        // this was encoded by qtplatz/contrib/agilent/libs/acqrscontrols/u5303a/waveform.cpp

        acqrscontrols::u5303a::device_data d;
        if ( adportable::binary::deserialize<>()( d, prop.deviceData(), prop.deviceDataSize() ) ) {
            textv.emplace_back( "Identifier",      d.ident_.Identifier() );
            textv.emplace_back( "Revision",        d.ident_.Revision() );
            textv.emplace_back( "Vendor",          d.ident_.Vendor() );
            textv.emplace_back( "Description",     d.ident_.Description() );
            textv.emplace_back( "InstrumentModel", d.ident_.InstrumentModel() );
            textv.emplace_back( "FirmwareRevision",d.ident_.FirmwareRevision() );
            textv.emplace_back( "SerialNumber",    d.ident_.SerialNumber() );
            textv.emplace_back( "Options",         d.ident_.Options() );
            textv.emplace_back( "IOVersion",       d.ident_.IOVersion() );
            textv.emplace_back( "NbrADCBits",      ( boost::format("%1%") % d.ident_.NbrADCBits() ).str() );

            textv.emplace_back( "accutofspectrometer", "dataInterpreter endoded" );
            return true;
        }
    } catch ( ... ) {
    }
    return false;
}


adcontrols::translate_state
DataInterpreter::translate( adcontrols::MassSpectrum& ms
                            , const char * data, size_t dsize
                            , const char * meta, size_t msize
                            , const adcontrols::MassSpectrometer& spectrometer
                            , size_t idData
							, const wchar_t * traceId ) const
{
    ADDEBUG() << "AccuTOF does not support rawdata v2 format";
    assert( 0 );
    return adcontrols::translate_error;
}

adcontrols::translate_state
DataInterpreter::translate( adcontrols::TraceAccessor& trace
                            , const char * data, size_t dsize
                            , const char * meta, size_t msize, unsigned long events ) const
{
    ADDEBUG() << "AccuTOF does not support rawdata v2 format";
    assert( 0 );
    return adcontrols::translate_error;
}
