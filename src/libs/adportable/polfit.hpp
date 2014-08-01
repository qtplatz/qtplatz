// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#include <vector>
#include <cstdlib> // size_t

namespace adportable {

    class polfit {
    public:
        polfit();

        enum WeightingType {
            WEIGHTING_NONE,
            WEIGHTING_INV_C,
            WEIGHTING_INV_C2,
            WEIGHTING_INV_C3,
        };

    public:  
        static bool fit(const double * x, const double *y, size_t npts, int nterms, std::vector<double>& polinomial, double & chisqr, WeightingType mode = WEIGHTING_NONE );
        static bool fit(const double * x, const double *y, size_t npts, int nterms, std::vector<double>& polinomial );
        static double estimate_y( const std::vector<double>& polinomial, double x );
        static double standard_error( const double * x, const double *y, size_t, const std::vector<double>& polinomial );
    };
}

