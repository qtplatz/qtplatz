// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2021 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2021 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include <string>
#include <vector>
#include <boost/optional.hpp>

namespace adcontrols {

	namespace mol { class molecule; struct isotope; }

    class ADCONTROLSSHARED_EXPORT isoCluster;

    class isoCluster {
    public:
        isoCluster();
        isoCluster( double abundance_threshold, double resolving_power );

        bool operator()( mol::molecule&, int charge = 0 ) const;

        // charge will be taken from either formula or adduct
        boost::optional< mol::molecule > compute( std::string&& formula, std::string&& adduct = std::string() );

    private:
        //static void merge_peaks( std::vector<isopeak>&, double resolving_power );

        bool merge( mol::isotope&, const mol::isotope& ) const;
        double threshold_daltons_;
        double threshold_abundance_;
        double resolving_power_;
        std::vector< mol::molecule > molecule_;
    };

}
