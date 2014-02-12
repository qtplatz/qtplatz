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

struct molecule {

    struct element {
        const char * symbol;
        size_t count;
        element( const char * _symbol, int _count = 0 ) : symbol(_symbol), count(_count) {
        }
    };

    struct isotope {
        double mass;
        double abundance;
        isotope( double _mass = 0, double _abundance = 0 ) : mass(_mass), abundance(_abundance) {
        }
    };
    
    std::vector< element > elements; // an array of (element&count), ex: C6H6O2 (six carbons, 6 hydrogens and 2 oxigens)
    std::vector< isotope > isotopes; // an array of isotopes of this molecule
};

