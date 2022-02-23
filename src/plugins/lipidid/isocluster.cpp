/**************************************************************************
** Copyright (C) 2022-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2022-2022 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "isocluster.hpp"
#include <adcontrols/isotopecluster.hpp>
#include <adcontrols/chemicalformula.hpp>

using lipidid::isoCluster;

isoCluster::~isoCluster()
{
}

isoCluster::isoCluster()
{
}

std::vector< isoCluster::value_type >
isoCluster::compute( const std::string& formula
                     , const std::string& adduct
                     , double abundance_llimit, double resolving_power )
{
    adcontrols::isotopeCluster isoCalc( abundance_llimit, resolving_power );
    auto cluster = isoCalc( adcontrols::ChemicalFormula::split( formula + " " + adduct), 0 );
    std::vector< value_type > v;
    for ( const auto& iso: cluster )
        v.emplace_back( iso.mass, iso.abundance );
    return v;
}

std::vector< isoCluster::value_type >
isoCluster::compute( const std::string& formula
                     , double abundance_llimit, double resolving_power )
{
    adcontrols::isotopeCluster isoCalc( abundance_llimit, resolving_power );
    auto cluster = isoCalc( adcontrols::ChemicalFormula::split( formula ), 0 );
    std::vector< value_type > v;
    for ( const auto& iso: cluster )
        v.emplace_back( iso.mass, iso.abundance );
    return v;
}
