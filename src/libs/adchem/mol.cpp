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

#include "mol.hpp"
#include <openbabel/mol.h>

using namespace adchem;

Mol::~Mol()
{
}

Mol::Mol()
{
}

Mol::Mol( const Mol& t ) : mol_( t.mol_ ? new OpenBabel::OBMol( *t.mol_ ) : 0 )
{
}

Mol::Mol( const OpenBabel::OBMol& mol ) : mol_( new OpenBabel::OBMol( mol ) )
{
}

Mol&
Mol::operator = ( const OpenBabel::OBMol& t )
{
	mol_.reset( new OpenBabel::OBMol( t ) );
	return *this;
}

double
Mol::getExactMass( bool implicitH ) const
{
	if ( mol_ )
		return const_cast< OpenBabel::OBMol& >( *mol_ ).GetExactMass( implicitH );
	return 0;
}

// static
double
Mol::GetExactMass( const OpenBabel::OBMol& mol, bool implicitH )
{
	return const_cast< OpenBabel::OBMol& >(mol).GetExactMass( implicitH );
}

// static
std::string
Mol::GetFormula( const OpenBabel::OBMol& mol )
{
	return const_cast< OpenBabel::OBMol& >(mol).GetFormula();
}

// static
void
Mol::SetAttribute( OpenBabel::OBMol& mol, const std::string& key, const std::string& value )
{
	OpenBabel::OBPairData * data = new OpenBabel::OBPairData();
	data->SetAttribute( key );
    data->SetValue( value );
	mol.SetData( data ); // will be deleted when OBMol's destractor was called
}

std::vector< std::pair< std::string, std::string > >
Mol::attributes( const OpenBabel::OBMol& mol, const std::vector< std::string >& excludes )
{
    using OpenBabel::OBMol;

	std::vector< std::pair< std::string, std::string > > attrs;
	for ( OpenBabel::OBDataIterator it = const_cast<OBMol&>(mol).BeginData(); it != const_cast<OBMol&>(mol).EndData(); ++it ) {
		const OpenBabel::OBGenericData& data = **it;
		if ( data.GetDataType() == OpenBabel::OBGenericDataType::PairData ) {
			const OpenBabel::OBPairData& pair = static_cast<const OpenBabel::OBPairData& >( data );
			std::string key = pair.GetAttribute();
			if ( std::find( excludes.begin(), excludes.end(), key ) == excludes.end() ) {
				std::string value = pair.GetValue();
				attrs.push_back( std::make_pair< std::string, std::string >( key, value ) );
			}
		}
	}
	return attrs;
}
