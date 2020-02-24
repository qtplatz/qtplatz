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

#include "chemicalformula.hpp"
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/element.hpp>
#include <adcontrols/isotopes.hpp>
#include <adportable/debug.hpp>
#include <boost/python.hpp>

using namespace py_module;

ChemicalFormula::ChemicalFormula()
{
}

ChemicalFormula::ChemicalFormula( const std::string& formula ) : formula_( formula )
{
}

ChemicalFormula::ChemicalFormula( const ChemicalFormula& t )
{
}

ChemicalFormula::~ChemicalFormula()
{
}

std::string
ChemicalFormula::formula() const
{
    return formula_;
}

double
ChemicalFormula::monoIsotopicMass() const
{
    return adcontrols::ChemicalFormula().getMonoIsotopicMass( formula_ );
}

double
ChemicalFormula::electronMass() const
{
    return adcontrols::ChemicalFormula().getElectronMass();
}

std::string
ChemicalFormula::standardFormula() const
{
    return adcontrols::ChemicalFormula().standardFormula( formula_ );
}

std::string
ChemicalFormula::formatFormula( bool richText ) const
{
    return adcontrols::ChemicalFormula::formatFormula( formula_, richText );
}

std::vector< boost::python::tuple >
ChemicalFormula::composition() const
{
    std::vector< boost::python::tuple > a;

    int charge;
    std::vector< adcontrols::mol::element > comp;
    if ( adcontrols::ChemicalFormula::getComposition( comp, formula_, charge ) ) {
        for ( const auto& c: comp ) {
            a.emplace_back( boost::python::make_tuple( std::string(c.symbol())
                                                       , c.count() ) );
        }
        const_cast< ChemicalFormula * >(this)->charge_ = charge;
    }

    return a;
}

std::vector< boost::python::dict >
ChemicalFormula::composition_dict() const
{
    std::vector< boost::python::dict > a;

    int charge;
    std::vector< adcontrols::mol::element > comp;
    if ( adcontrols::ChemicalFormula::getComposition( comp, formula_, charge ) ) {
        for ( const auto& c: comp ) {
            boost::python::dict dict;
            dict[ "symbol" ] = std::string( c.symbol() );
            dict[ "name" ] = std::string( c.name() );
            dict[ "atomicNumber" ] = c.atomicNumber();
            dict[ "valence" ] = c.valence();
            dict[ "count" ] = c.count();
            dict[ "mass" ] = c.monoIsotopicMass( c );

            // std::vector< boost::python::tuple > isotopes;
            // for ( const auto& i: c.isotopes() )
            //     isotopes.emplace_back( boost::python::make_tuple( i.mass, i.abundance ) );

            // dict[ "isotopes" ] = isotopes;

            a.emplace_back( dict );
        }
        const_cast< ChemicalFormula * >(this)->charge_ = charge;
    }
    return a;
}

int
ChemicalFormula::charge() const
{
    if ( charge_ )
        return *charge_;

    int charge(0);
    std::vector< adcontrols::mol::element > comp;
    adcontrols::ChemicalFormula::getComposition( comp, formula_, charge );
    const_cast< ChemicalFormula * >(this)->charge_ = charge;

    return charge;
}
