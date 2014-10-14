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

#pragma once

#include "adcontrols_global.h"
#include "element.hpp"
#include <cstdlib>
#include <vector>

// interface for isotope cluster pattern

namespace adcontrols {  

    class isotopeCluster;
    class ChemicalFormula;

	namespace mol {

        struct ADCONTROLSSHARED_EXPORT isotope {
            double mass;
            double abundance;
            isotope( double m = 0, double a = 1.0 ) : mass(m), abundance(a) {}
        };

#if defined _MSC_VER
        template class ADCONTROLSSHARED_EXPORT std::vector < isotope > ;
        template class ADCONTROLSSHARED_EXPORT std::vector < element > ;
#endif

        struct ADCONTROLSSHARED_EXPORT molecule {
            molecule();
            molecule( const molecule& t );

            molecule& operator << (const element&);
            molecule& operator << (const isotope&);

            std::vector< isotope > cluster;
            std::vector< element > elements;
        };

    }
}
