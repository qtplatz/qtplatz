// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
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
#include <functional>
#include <memory>

namespace adcontrols {

    class MassSpectrum;
    class MassSpectrometer;

    class ADCONTROLSSHARED_EXPORT histogram {
    public:
        static std::shared_ptr< MassSpectrum > make_profile( const MassSpectrum&, const MassSpectrometer& );
        static std::shared_ptr< MassSpectrum > make_profile( const MassSpectrum& );
        static bool is_full_profile( const MassSpectrum& ); // the spectrum length match to digitizer sampling length?
    private:
        static void histogram_to_profile( MassSpectrum&, const MassSpectrometer& );
        static void histogram_to_profile( MassSpectrum& );
        static void histogram_to_profile( MassSpectrum&, std::function< double( double, int, double, double ) > assigner );
    };

}
