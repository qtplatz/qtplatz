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

#ifndef DATAINTERPRETER_HPP
#define DATAINTERPRETER_HPP

#include <adcontrols/datainterpreter.hpp>

namespace batchproc {

    class DataInterpreter : public adcontrols::DataInterpreter {
    public:
        DataInterpreter();

        adcontrols::translate_state translate( adcontrols::MassSpectrum&
                                               , const char * data, size_t dsize
                                               , const char * meta, size_t msize
                                               , const adcontrols::MassSpectrometer&
                                               , size_t idData
											   , const wchar_t * traceId ) const override;
        
        adcontrols::translate_state translate( adcontrols::TraceAccessor&
                                               , const char * data, size_t dsize
                                               , const char * meta, size_t msize, unsigned long events ) const override;

    private:
        adcontrols::translate_state translate_profile( adcontrols::MassSpectrum&
                                                       , const char * data, size_t dsize
                                                       , const char * meta, size_t msize
                                                       , const adcontrols::MassSpectrometer&
                                                       , size_t idData ) const;

        adcontrols::translate_state translate_processed( adcontrols::MassSpectrum&
                                                         , const char * data, size_t dsize
                                                         , const char * meta, size_t msize
                                                         , const adcontrols::MassSpectrometer&
                                                         , size_t idData ) const;
        
    };

}
    
#endif // DATAINTERPRETER_HPP
