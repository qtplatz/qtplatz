// -*- C++ -*-
/**************************************************************************
**
** MIT License
** Copyright (c) 2021-2022 Toshinobu Hondo, Ph.D

** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:

** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
**************************************************************************/

#include "allchem.hpp"
#include <adportable/smarts_parser.hpp>

#include <GraphMol/RWMol.h>
#include <GraphMol/ROMol.h>
#include <Geometry/point.h>
#include <RDGeneral/utils.h>
#include <GraphMol/Atom.h>
#include <GraphMol/ChemReactions/Reaction.h>
#include <GraphMol/ChemReactions/ReactionParser.h>
#include <GraphMol/ChemReactions/ReactionPickler.h>
#include <GraphMol/ChemReactions/ReactionRunner.h>
#include <GraphMol/ChemReactions/ReactionUtils.h>
#include <GraphMol/ChemReactions/SanitizeRxn.h>
#include <GraphMol/Descriptors/MolDescriptors.h>
#include <GraphMol/FileParsers/FileParsers.h>
#include <GraphMol/FileParsers/MolSupplier.h>
#include <GraphMol/RDKitBase.h>
#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/SmilesParse/SmilesWrite.h>
#include <GraphMol/inchi.h>

std::optional< RDKit::ChemicalReaction >
AllChem::ReactionFromSmarts( std::string smarts )
{
    using smarts_parser = adportable::smarts_parser;
    RDKit::ChemicalReaction rxn;
    if ( auto ctx = smarts_parser::parse( smarts ) ) {
        for ( const auto& r: std::get<0>(*ctx) )
            rxn.addReactantTemplate( RDKit::ROMOL_SPTR( RDKit::SmartsToMol( r ) ) );
        for ( const auto& p: std::get<1>(*ctx) )
            rxn.addProductTemplate( RDKit::ROMOL_SPTR( RDKit::SmartsToMol( p ) ) );
        rxn.initReactantMatchers();
        return rxn;
    }
    return {};
}
