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

#include "molecule.hpp"

// #include <RDGeneral/Invariant.h>
// #include <GraphMol/RDKitBase.h>
#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/SmilesParse/SmilesWrite.h>
// #include <GraphMol/Substruct/SubstructMatch.h>
#include <GraphMol/Depictor/RDDepictor.h>
// #include <GraphMol/FileParsers/FileParsers.h>
#include <GraphMol/Descriptors/MolDescriptors.h>
// #include <GraphMol/FileParsers/MolSupplier.h>

using namespace adchem;

molecule::~molecule()
{
    delete mol_;
}

molecule::molecule() : mol_(0)
{
}

molecule::molecule( const molecule& t ) : mol_(0)
{
    if ( t.mol_ )
        mol_ = new RDKit::ROMol( *t.mol_ );
}

// static
RDKit::ROMol *
molecule::SmilesToMol( const std::string& smiles )
{
    return RDKit::SmilesToMol( smiles );
}

// static
std::string
molecule::MolToSmiles( const RDKit::ROMol& mol )
{
    return RDKit::MolToSmiles( mol );
}

// static
std::string
molecule::MolToFormula( RDKit::ROMol& mol, bool separateIsotopes, bool abbreviateHIsotopes )
{
    mol.updatePropertyCache( false );
    return RDKit::Descriptors::calcMolFormula( mol, separateIsotopes, abbreviateHIsotopes );
}
