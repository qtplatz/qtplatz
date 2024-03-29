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

#include "datainterpreter.hpp"

using namespace acqrsinterpreter;

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
DataInterpreter::translate( waveform_types& waveform, const int8_t * data, size_t dsize, const int8_t * meta, size_t msize )
{
    return adcontrols::translate_error;
}
