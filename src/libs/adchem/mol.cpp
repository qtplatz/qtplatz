/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
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

#include "mol.hpp"

#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/SmilesParse/SmilesWrite.h>
#include <GraphMol/Depictor/RDDepictor.h>
#include <GraphMol/Descriptors/MolDescriptors.h>
#include <INCHI-API/inchi.h>

using namespace adchem;

mol::~mol()
{
}

mol::mol()
{
}

mol::mol( const mol& t ) : mol_( std::make_unique< RDKit::ROMol >( *t.mol_ ) )
{
}

mol::mol( const std::string& text, inputType typ )
{
    switch( typ ) {
    case SMILES:
        mol_ = std::unique_ptr< RDKit::ROMol >( RDKit::SmilesToMol( text ) );
        break;
    case INCHI:
        RDKit::ExtraInchiReturnValues rv;
        mol_ = std::unique_ptr< RDKit::ROMol >( RDKit::InchiToMol( text, rv ) ); // sanitize, remove hidrogen
        break;
    }
}

//static
std::string
mol::smiles( const RDKit::ROMol& m )
{
    return RDKit::MolToSmiles( m );
}

//static
std::string
mol::formula( const RDKit::ROMol& m, bool separateIsotopes, bool abbreviateHIsotopes )
{
    //m.updatePropertyCache( false );
    return RDKit::Descriptors::calcMolFormula( m, separateIsotopes, abbreviateHIsotopes );    
}

std::string
mol::formula() const
{
    if ( mol_ ) {
        mol_->updatePropertyCache( false );
        return RDKit::Descriptors::calcMolFormula( *mol_ );
    }
    return std::string();
}

std::string
mol::smiles() const
{
    if ( mol_ )
        return RDKit::MolToSmiles( *mol_ );
    return std::string();
}

std::string
mol::InChI() const
{
    RDKit::ExtraInchiReturnValues rv;
    if ( mol_ )
        return RDKit::MolToInchi( *mol_, rv );
    return std::string();    
}

// static
std::string
mol::InChIToInChIKey( const std::string& inchi )
{
    RDKit::InchiToInchiKey( inchi );
}
