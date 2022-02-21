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

#include "mol.hpp"
#include <adcontrols/chemicalformula.hpp>

using lipidid::mol;
using lipidid::moldb;

mol::~mol()
{
}

mol::mol() : mass_(0)
{
}

mol::mol( const mol& t ) : mol_( t.mol_ )
                         , mass_( t.mass_ )
{
}

mol::mol( std::tuple< size_t, std::string, std::string, std::string, double >&& t )
    : mol_( t )
    , mass_( adcontrols::ChemicalFormula().getMonoIsotopicMass( std::get< 1 >( t ) ) )
{
}

/////

moldb&
moldb::instance()
{
    static moldb __instance;
    return __instance;
}

std::shared_ptr< const mol >
moldb::find( const std::string& InChIKey ) const
{
    auto it = mols_.find( InChIKey );
    if ( it != mols_.end() )
        return it->second;
    return {};
}

moldb&
moldb::operator << ( std::shared_ptr< const mol > mol )
{
    mols_[ mol->inchikey() ] = mol;
    return *this;
}

double
moldb::logP( const std::string& InChIKey )
{
    if ( auto mol = instance().find( InChIKey ) )
        return mol->logP();
    return 0;
}
