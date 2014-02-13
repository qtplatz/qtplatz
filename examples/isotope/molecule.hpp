/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include <cstdint>
#include <vector>
#include "element.hpp"

// namespace 'mol' describes about a molecule
// namespace 'toe' describes about atoms with respect to table of element
// class 'element' in global spece uses both in molecule and table-of-element

namespace mol {
    struct isotope {
        double mass;
        double abundance;
        isotope( double m = 0, double a = 1.0 ) : mass(m), abundance(a) {}
    };
}

struct molecule {
    std::vector< element > elements; // an array of (element&count), ex: C6H6O2 (six carbons, 6 hydrogens and 2 oxigens)
    std::vector< mol::isotope > cluster;
};

