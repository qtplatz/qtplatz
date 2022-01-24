/**************************************************************************
** Copyright (C) 2020 MS-Cheminformatics LLC
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

#include "isotopecluster.hpp"
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/isotopecluster.hpp>
#include <adcontrols/element.hpp>
#include <adcontrols/isotopes.hpp>
#include <adportable/debug.hpp>
#include <boost/python.hpp>

using namespace py_module;

IsotopeCluster::IsotopeCluster( const std::string& formula ) : formula_( formula )
                                                             , resolvingPower_( 10000 )
                                                             , charge_( 1 )
{
    int charge;
    std::vector< adcontrols::mol::element > comp;
    if ( adcontrols::ChemicalFormula::getComposition( comp, formula_, charge ) ) {
        charge_ = charge;
    }
}

IsotopeCluster::IsotopeCluster( const IsotopeCluster& t ) : formula_( t.formula_ )
                                                          , charge_( t.charge_ )
                                                          , resolvingPower_( t.resolvingPower_ )
{
}

IsotopeCluster::~IsotopeCluster()
{
}

void
IsotopeCluster::setResolvingPower( double value )
{
    resolvingPower_ = value;
}

double
IsotopeCluster::resolvingPower() const
{
    return resolvingPower_;
}

void
IsotopeCluster::setCharge( int value )
{
    charge_ = value;
}

int
IsotopeCluster::charge() const
{
    return charge_;
}

std::string
IsotopeCluster::formula() const // formula, charge
{
    return formula_;
}

std::vector< boost::python::tuple >
IsotopeCluster::compute() const // mass,abundance,index
{
    adcontrols::isotopeCluster cluster( 1.0e-8, resolvingPower_ );

    auto pks = cluster( adcontrols::ChemicalFormula::split( formula_ ), charge_ );

    std::vector< boost::python::tuple > a;

    for ( const auto& pk: pks )
        a.emplace_back( boost::python::make_tuple( pk.mass, pk.abundance ) );

    return a;
}
