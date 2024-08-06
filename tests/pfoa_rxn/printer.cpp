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

#include "printer.hpp"
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

const printer&
printer::operator()( const RDKit::ChemicalReaction& rxn, std::ostream& out ) const
{
    out << heading_;
    size_t n = rxn.getNumReactantTemplates();
    for ( const auto& r: rxn.getReactants() ) {
        out << RDKit::MolToSmiles( *r );
        if ( --n )
            out << ".";
    }
    out << ">>";
    n = rxn.getNumProductTemplates();
    for ( const auto& p: rxn.getProducts() ) {
        out << RDKit::MolToSmiles( *p );
        if ( --n )
            out << ".";
    }
    out << std::endl;
    return *this;
}

const printer&
printer::operator()( const RDKit::ROMol& m, std::ostream& out ) const
{
    out << heading_;
    out << RDKit::MolToSmiles( m ) << std::endl;
    return *this;
}

const printer&
printer::operator()( const RDKit::MOL_SPTR_VECT& vect, std::ostream& out ) const
{
    int id{0};

    for ( const auto& m: vect ) {
        m->updatePropertyCache();
        out << heading_;
        out << "[" << id++ << "]\t{"
            << RDKit::MolToSmiles( *m ) << "}\t"
            << "\t" << RDKit::Descriptors::calcExactMW( *m )
            << std::endl;
    }
    return *this;
}


const printer&
printer::operator()( const std::vector<RDKit::MOL_SPTR_VECT>& vect, std::ostream& out ) const
{
    std::pair< int, int > id{0,0};

    for ( const auto& v: vect ) {
        for ( const auto& m: v ) {
            m->updatePropertyCache();
            out << heading_;
            out << "[" << std::get<0>(id) << ", " << std::get<1>(id) << "]\t{"
                << RDKit::MolToSmiles( *m ) << "}\t"
                << "\t" << RDKit::Descriptors::calcExactMW( *m )
                << std::endl;
            std::get<1>(id)++;
        }
        std::get<0>(id)++;
        out << std::endl;
    }
    return *this;
}
