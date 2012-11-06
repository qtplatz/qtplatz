/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#include "chopper.hpp"

#include <openbabel/bond.h>
#include <openbabel/mol.h>

using namespace adchem;

Chopper::~Chopper()
{
}

Chopper::Chopper()
{
}

//static
std::vector< int /* idx */ >
Chopper::chop( OpenBabel::OBMol& mol )
{
	std::vector< int > indecies;
	for ( OpenBabel::OBBondIterator it = mol.BeginBonds(); it != mol.EndBonds(); ++it ) {
		if ( ! ( (*it)->IsAromatic() || (*it)->IsDouble() ) )
			indecies.push_back( (*it)->GetIdx() );
	}
	return indecies;
}

std::pair< OpenBabel::OBMol, OpenBabel::OBMol >
Chopper::split( OpenBabel::OBMol& mol, int idx )
{
	using OpenBabel::OBMol;

	OpenBabel::OBBond * bond = mol.GetBond( idx );
	if ( mol.DeleteBond( bond ) ) {
		std::vector< OBMol > sub = mol.Separate();
        if ( sub.size() >= 2 ) {
            if ( sub[0].GetExactMass() > sub[1].GetExactMass() )
				return std::make_pair< OBMol, OBMol >( sub[0], sub[1] );
			else
				return std::make_pair< OBMol, OBMol >( sub[1], sub[0] );
		}
	}
	return std::make_pair< OBMol, OBMol >( mol, OBMol() ); // return empty
}