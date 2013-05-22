/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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

#include "conversion.hpp"

#include <openbabel/obconversion.h>
#include <openbabel/mol.h>

using namespace adchem;

Conversion::~Conversion()
{
}

Conversion::Conversion()
{
}

Conversion::Conversion( const Conversion& )
{
}

//static
std::string
Conversion::toSVG( const OpenBabel::OBMol& mol )
{
	OpenBabel::OBConversion conv;
	conv.SetOutFormat( "svg" );
	return conv.WriteString( const_cast< OpenBabel::OBMol *>(&mol) );
}

//static
std::string
Conversion::toSMILES( const OpenBabel::OBMol& mol )
{
	OpenBabel::OBConversion conv;
	conv.SetOutFormat( "smiles" );
	return conv.WriteString( const_cast< OpenBabel::OBMol *>(&mol) );
}
