// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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

#pragma once

#include "adcontrols_global.h"
#include <fstream>
#include <vector>
#include <string>

namespace adcontrols {

    class MassSpectrometer;
    class MassSpectrum;
    class TraceAccessor;
    class MSCalibration;
    class MSProperty;
    class lockmass;

    enum translate_state {
        translate_error
        , translate_complete      // all spectra has been packed
        , translate_indeterminate // part of protocol acquisition has been packed into target
        , no_interpreter          // data interpreter not installed
        , no_more_data
    };

    class ADCONTROLSSHARED_EXPORT DataInterpreter {
    public:
        DataInterpreter(void);
        ~DataInterpreter(void);

        virtual translate_state
            translate( MassSpectrum&
                       , const char * data, size_t dsize
                       , const char * meta, size_t msize 
                       , const MassSpectrometer&
                       , size_t idData
                       , const wchar_t * traceId ) const = 0;
        
        virtual translate_state
            translate( TraceAccessor&
                       , const char * data, size_t dsize
                       , const char * meta, size_t msize, unsigned long events ) const = 0;

        virtual bool compile_header( MassSpectrum&, std::ifstream& ) const { return false; }

        virtual bool make_device_text( std::vector< std::pair< std::string, std::string > >&, const MSProperty& ) const { return false; }

        virtual bool has_lockmass() const { return false; }
        virtual bool lockmass( adcontrols::lockmass& ) const { return false; }
    };

}
