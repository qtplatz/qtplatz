/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "adprocessor_global.hpp"
#include <memory>

namespace adcontrols {
    class ProcessMethod;
    class CentroidMethod;
    class MassSpectrum;
    class MSPeakInfo;
}

namespace adprocessor {

    class centroid_processor {
    public:
        ~centroid_processor();
        centroid_processor();
        centroid_processor( const adcontrols::ProcessMethod& );

        void setup( const adcontrols::ProcessMethod& );
        const adcontrols::CentroidMethod& centroidMethod() const;

        std::pair< std::shared_ptr< adcontrols::MassSpectrum >
                   , std::shared_ptr< adcontrols::MSPeakInfo > > operator()( const adcontrols::MassSpectrum& ) const;

    private:
        class impl;
        std::unique_ptr< impl > impl_;
    };

}
