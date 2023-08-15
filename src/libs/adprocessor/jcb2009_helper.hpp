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
#include <adcontrols/mspeakinfo.hpp>
#include <boost/json/fwd.hpp>
#include <boost/json/value_to.hpp>
#include <boost/uuid/uuid.hpp>
#include <memory>
#include <optional>

namespace portfolio {
    class Folium;
}

namespace adcontrols {
    class Peaks;
    class Peak;
    class MassSpectrum;
    class MSPeakInfo;
    class ProcessMethod;
    class Chromatogram;
    class annotation;
}

namespace adprocessor {

    class dataprocessor;
    class generator_property;

    namespace jcb2009_helper {

        struct printer {
            void print( const portfolio::Folium& );
        };

         struct find_peaks {
             std::tuple< double, double, double > tR( const adcontrols::Peak& );
         };

        struct folium_accessor {
            const portfolio::Folium& folium_;
            folium_accessor( const portfolio::Folium& folium );

            std::tuple< generator_property, adcontrols::Peaks > operator()() const;
            adcontrols::Peaks get_peaks() const;
            generator_property get_generator_property() const;
        };

        class find_mass {
        public:
            ~find_mass();
            find_mass( const portfolio::Folium&, const adcontrols::ProcessMethod& );

            std::optional< std::pair< size_t, int > >
            operator()( const adcontrols::MassSpectrum& centroid, int proto );

            std::optional< adcontrols::MSPeakInfo::const_iterator >
            operator()( const adcontrols::MSPeakInfo& pInfo, int proto );

            adprocessor::dataprocessor * dataprocessor();
        private:
            class impl;
            impl * impl_;
        };

    }

}
