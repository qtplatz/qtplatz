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

mol::mol( std::tuple< size_t, std::string, std::string, std::string >&& t )
    : mol_( t )
    , mass_( adcontrols::ChemicalFormula().getMonoIsotopicMass( std::get< 1 >( t ) ) )
{
}