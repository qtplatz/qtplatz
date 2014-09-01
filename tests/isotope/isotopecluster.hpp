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

#ifndef ISOTOPE_HPP
#define ISOTOPE_HPP

#include "molecule.hpp"

class isotopecluster {
public:
    isotopecluster( double limit_daltons = 0.5e-7 );
    bool operator()( molecule& mol ) const;
    // todo: cluster method to be added
private:
    bool marge( mol::isotope&, const mol::isotope& ) const;

    double threshold_daltons_;
    double threshold_abandance_;
};

#endif // ISOTOPE_HPP
