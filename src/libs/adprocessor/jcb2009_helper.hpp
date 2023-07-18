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

namespace portfolio {
    class Folium;
}

namespace adcontrols {
    class Peaks;
    class Peak;
    class MassSpectrum;
    class MSPeakInfo;
}

namespace adprocessor {

    namespace jcb2009_helper {

        struct printer {
            void print( const portfolio::Folium& );
        };

        struct find_peaks {
            adcontrols::Peaks get( const portfolio::Folium& );
            std::tuple< double, double, double > tR( const adcontrols::Peak&, double divisor = 2.0 );
        };

        class annotator {
        public:
            ~annotator();
            annotator( const portfolio::Folium& );
            void operator()( std::shared_ptr< adcontrols::MassSpectrum > pCentroid );
            void operator()( std::shared_ptr< adcontrols::MSPeakInfo > pInfo );
        private:
            class impl;
            impl * impl_;
        };
    }



}
