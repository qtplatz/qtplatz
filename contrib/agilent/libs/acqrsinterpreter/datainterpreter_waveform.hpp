/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#pragma once

#include <adcontrols/datainterpreter.hpp>
#include "datainterpreter.hpp"

namespace adcontrols {
    class MassSpectrum;
    class TraceAccessor;
}

namespace acqrsinterpreter {
    namespace waveform {

        template< typename result_type >
        class DataInterpreter : public acqrsinterpreter::DataInterpreter {
        public:
            virtual ~DataInterpreter()
                {}
            DataInterpreter()
                {}

            bool compile_header( adcontrols::MassSpectrum&, std::ifstream& ) const override {
                return false;
            }

            bool make_device_text( std::vector< std::pair< std::string, std::string > >&
                                   , const adcontrols::MSProperty& ) const override {
                return false;
            }

            adcontrols::translate_state
            translate( adcontrols::MassSpectrum& ms
                       , const char * data, size_t dsize
                       , const char * meta, size_t msize
                       , const adcontrols::MassSpectrometer& sp
                       , size_t idData
                       , const wchar_t * traceId ) const override;
        
            adcontrols::translate_state
            translate( waveform_types& variant, const int8_t * data, size_t dsize, const int8_t * meta, size_t msize ) override {
                auto wform = std::make_shared< result_type >();
                variant = wform;
                return translate( *wform, data, dsize, meta, msize );
            }
            
        private:
            static adcontrols::translate_state
            translate( result_type&, const int8_t * data, size_t dsize, const int8_t * meta, size_t msize );
        };

    }
}

