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
#include "string.hpp"
#include "mol.hpp"

#include <compiler/disable_unused_parameter.h>
#include <openbabel/obconversion.h>
#include <openbabel/mol.h>


using namespace adchem;

Conversion::~Conversion()
{
}

Conversion::Conversion() : obconversion_( new OpenBabel::OBConversion )
                         , nread_( 0 )
{
}

Conversion::Conversion( const Conversion& t ) : obconversion_( t.obconversion_ )
                                              , nread_( t.nread_ )
{
}

//static
adchem::string
Conversion::toSVG( const Mol& mol )
{
	OpenBabel::OBConversion conv;
	conv.SetOutFormat( "svg" );
    std::string res = conv.WriteString( const_cast< OpenBabel::OBMol *>( mol.obmol() ) );
    return adchem::string( res.c_str() );
}

//static
adchem::string
Conversion::toSMILES( const Mol& mol )
{
	OpenBabel::OBConversion conv;
	conv.SetOutFormat( "smiles" );
	std::string res = conv.WriteString( const_cast< OpenBabel::OBMol *>( mol.obmol() ) );
    return adchem::string( res.c_str() );
}

void
Conversion::informat( const OpenBabel::OBFormat * informat )
{
    obconversion_->SetInFormat( const_cast< OpenBabel::OBFormat *>( informat ) );
}

void
Conversion::open( const char * filename )
{
    filename_ = filename;
    nread_ = 0;
}

bool
Conversion::read( Mol& mol )
{
    OpenBabel::OBMol omol;
    bool success( false );
	if ( nread_++ == 0 )
		success = obconversion_->ReadFile( &omol, filename_ );
    else
        success = obconversion_->Read( &omol );
    mol.obmol( omol );
    return success;
}

unsigned long long
Conversion::tellg() const
{
    std::istream * stream = obconversion_->GetInStream();
    if ( stream )
        return stream->tellg();
    return 0;
}

