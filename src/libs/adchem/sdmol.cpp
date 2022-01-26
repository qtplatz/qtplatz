/**************************************************************************
** Copyright (C) 2010-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2022 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "sdmol.hpp"
#include "sdfile.hpp"
#include "drawing.hpp"
#include <adportable/debug.hpp>
#include <GraphMol/Depictor/RDDepictor.h>
#include <GraphMol/Descriptors/MolDescriptors.h>
#include <GraphMol/FileParsers/FileParsers.h>
#include <GraphMol/FileParsers/MolSupplier.h>
#include <GraphMol/RDKitBase.h>
#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/SmilesParse/SmilesWrite.h>
#include <GraphMol/Substruct/SubstructMatch.h>
#include <GraphMol/inchi.h>
#include <RDGeneral/Invariant.h>
#include <RDGeneral/RDLog.h>
#include <memory>

using namespace adchem;

SDMol::SDMol() : index_( 0 )
{
}

SDMol::SDMol( const SDMol& t ) : index_    ( t.index_ )
                               , sdfile_   ( t.sdfile_ )
                               , dataItems_( t.dataItems_ )
                               , svg_      ( t.svg_ )
                               , smiles_   ( t.smiles_ )
                               , formula_  ( t.formula_ )

{
    mol_ = std::make_unique< RDKit::ROMol >( *t.mol_ );
}

SDMol&
SDMol::operator = ( const SDMol& t )
{
    index_     = t.index_;
    sdfile_    = t.sdfile_;
    dataItems_ = t.dataItems_;
    svg_       = t.svg_;
    smiles_    = t.smiles_;
    formula_   = t.formula_;
    mass_      = t.mass_;
    mol_       = std::make_unique< RDKit::ROMol >( *t.mol_ );
    return *this;
}


SDMol::SDMol( SDFile * sdfile, size_t idx )
    : index_( idx )
    , sdfile_( sdfile->shared_from_this() )
    , mol_( std::make_unique< RDKit::ROMol >( *sdfile->molSupplier()[ index_ ] ) )
    , dataItems_( SDFile::parseItemText( sdfile->molSupplier().getItemText( index_ ) ) )
    , formula_( RDKit::Descriptors::calcMolFormula( mol(), true, false ) )
    , mass_( RDKit::Descriptors::calcExactMW( *mol_ ) )
{
}

RDKit::ROMol&
SDMol::mol()
{
    return *mol_;
}

const RDKit::ROMol&
SDMol::mol() const
{
    return *mol_;
}

const std::string&
SDMol::svg()
{
    if ( svg_.empty() ) {
        svg_ = adchem::drawing::toSVG( mol() );
    }
    return svg_;
}

const std::string&
SDMol::smiles()
{
    if ( smiles_.empty() ) {
        smiles_    = RDKit::MolToSmiles( mol() );
    }
    return smiles_;
}

const std::string&
SDMol::formula() const
{
    return formula_;
}

double
SDMol::mass() const
{
    return mass_;
}

const std::vector< std::pair< std::string, std::string > >
SDMol::dataItems() const
{
    return dataItems_;
}
