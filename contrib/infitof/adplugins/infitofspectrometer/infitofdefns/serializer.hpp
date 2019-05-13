/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
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

#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace infitof {

    class AveragerData;
    class ProcessedData;

    class SpectrumProcessedData; // trace

    class ControlMethod;
    
    class serializer {
    public:
        serializer();

        static bool serialize( const AveragerData&, std::string& );
        static bool deserialize( AveragerData&, const char * pdevice, std::size_t octets );

        static bool serialize( const std::vector< SpectrumProcessedData >&, std::string& );
        static bool deserialize( std::vector< SpectrumProcessedData >&, const char * pdevice, std::size_t octets );

        static bool serialize( const ControlMethod&, std::string& );
        static bool deserialize( ControlMethod&, const char * pdevice, std::size_t octets );

        static void debug_dump( const AveragerData&, const char * msg, int nData );
    };

}

