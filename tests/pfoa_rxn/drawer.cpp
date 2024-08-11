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

#include "drawer.hpp"
#include <Geometry/point.h>
#include <GraphMol/Atom.h>
#include <GraphMol/Bond.h>
#include <GraphMol/Depictor/RDDepictor.h>
#include <GraphMol/Descriptors/MolDescriptors.h>
#include <GraphMol/FileParsers/FileParsers.h>
#include <GraphMol/FileParsers/MolSupplier.h>
#include <GraphMol/MolDraw2D/MolDraw2DSVG.h>
#include <GraphMol/RDKitBase.h>
#include <GraphMol/ROMol.h>
#include <GraphMol/RWMol.h>
#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/SmilesParse/SmilesWrite.h>
#include <GraphMol/inchi.h>
#include <RDGeneral/utils.h>
#include <RDGeneral/versions.h>
#include <sstream>

std::vector<int>
get_all_hit_bonds(const RDKit::ROMol& mol, const std::vector<int>& hit_atoms) {
    std::vector<int> hit_bonds;
    for (int i : hit_atoms) {
        for (int j : hit_atoms) {
            if (i > j) {
                if ( const RDKit::Bond *bnd = mol.getBondBetweenAtoms(i, j) ) {
                    hit_bonds.emplace_back(bnd->getIdx());
                }
            }
        }
    }
    return hit_bonds;
}

drawer::~drawer()
{
}

drawer::drawer()
{
}

const void
drawer::moltosvg( const RDKit::ROMol& mol, std::ostream& out ) const
{
    RDKit::ROMol mol1( mol );
    RDDepict::compute2DCoords( mol1 );

    RDKit::MolDraw2DSVG svg_drawer( 300, 300, out );
    // moldrawer.drawOptions().backgroundColour = background;
    svg_drawer.drawMolecule( mol1 );
    svg_drawer.finishDrawing();
}

const void
drawer::moltosvg( const RDKit::ROMol& mol, const RDKit::ROMol& sss, std::ostream& out ) const
{
    RDKit::MolDraw2DSVG svg_drawer( 300, 300, out );

    std::vector< RDKit::MatchVectType > matchVect;
    if ( RDKit::SubstructMatch( mol, sss, matchVect ) ) { // find all match
        std::vector< int > atoms;
        for ( auto& t: matchVect ) {
            for ( auto& q: t )
                atoms.emplace_back( q.second );
        }
        auto bonds = get_all_hit_bonds( mol, atoms );
        svg_drawer.drawMolecule( mol, &atoms, &bonds );
    } else {
        svg_drawer.drawMolecule( mol );
    }
    svg_drawer.finishDrawing();
}

const void
drawer::moltosvg( const RDKit::ROMol& mol, std::unique_ptr< RDKit::ROMol >&& sss, std::ostream& out ) const
{
    moltosvg( mol, *sss, out );
}

std::string
drawer::toSvg( const RDKit::ROMol& mol )
{
    std::ostringstream o;
    drawer().moltosvg( mol, o );
    return o.str();
}

std::string
drawer::toSvg( const RDKit::ROMol& mol, const RDKit::ROMol& sss )
{
    std::ostringstream o;
    drawer().moltosvg( mol, sss, o );
    return o.str();
}
