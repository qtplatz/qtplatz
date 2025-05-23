/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

#pragma once

#include <adcontrols/datainterpreter.hpp>

namespace adcontrols { class MassSpectrum; class MSCalibration; class MassSpectrometer; }
namespace infitof { class AveragerData; }

namespace infitofspectrometer {

    class u5303a_translator {
    public:
        u5303a_translator();
        static adcontrols::translate_state
        translate( adcontrols::MassSpectrum&
                   , const infitof::AveragerData& rb
                   , size_t idData
                   , const adcontrols::MassSpectrometer& );
    };

}


