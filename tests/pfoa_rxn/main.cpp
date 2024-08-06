#include <algorithm>
#include <fstream>
#include <iostream>
#include <Geometry/point.h>
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
#include <GraphMol/MolDraw2D/MolDraw2DSVG.h>
#include <GraphMol/RDKitBase.h>
#include <GraphMol/ROMol.h>
#include <GraphMol/RWMol.h>
#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/SmilesParse/SmilesWrite.h>
#include <GraphMol/Substruct/SubstructMatch.h>
#include <GraphMol/inchi.h>
#include <RDGeneral/utils.h>
#include <boost/spirit/home/x3.hpp>
#include "allchem.hpp"
#include "printer.hpp"

std::vector<int> get_all_hit_bonds(RDKit::ROMol &mol,
                                   const std::vector<int> &hit_atoms) {
  std::vector<int> hit_bonds;
  for (int i : hit_atoms) {
    for (int j : hit_atoms) {
      if (i > j) {
          RDKit::Bond *bnd = mol.getBondBetweenAtoms(i, j);
          if (bnd) {
              hit_bonds.emplace_back(bnd->getIdx());
        }
      }
    }
  }
  return hit_bonds;
}


int
main(int argc, char **argv)
{
    using namespace RDKit;
    auto __pfoa  = "FC(F)(C(F)(F)C(=O)O)C(F)(F)C(F)(F)C(F)(F)C(F)(F)C(F)(F)F"_smiles;
    auto __water = "O"_smiles;
    auto __sss  = "C(F).C"_smiles;

    for ( const auto& mol: { __pfoa.get(), __water.get(), __sss.get() } )
        mol->updatePropertyCache();

    std::vector< RDKit::ChemicalReaction > rxns;
    for ( const auto& smarts:
              { "[C:1](O)(=O)[C:2]>>[C-:2].[C:1](=O)(=O)"
                , "[C-:1].[O:2]>>[C-0:1][O:2].[H+]"
                , "[C:1](O)([F:2])>>[C:1](O).[F:2]"
                , "[C:1](O)([F:2])>>[C:1]([O])(=O).[F:2]" } ) {
        if ( auto rxn = AllChem::ReactionFromSmarts( smarts ) )
            rxns.emplace_back( std::move( *rxn ) );
        else
            std::cerr << "Got an error for " << smarts << std::endl;
    }

    std::cout << "----------------------------------------" << std::endl;
    for ( const auto& rxn: rxns )
        printer("rxn: ")(rxn);
    std::cout << "----------------------------------------" << std::endl;

    auto reactant = boost::make_shared< RDKit::ROMol >( *__pfoa );
    auto water = boost::make_shared< RDKit::ROMol >( *__water );
    // auto reactant = pfoa;
    RDKit::MOL_SPTR_VECT products;
    RDKit::MatchVectType match;

    while ( RDKit::SubstructMatch( *reactant, *__sss, match ) ) { // find one match
        auto p1 = rxns[0].runReactants( RDKit::MOL_SPTR_VECT{ reactant } );
        auto p2 = rxns[1].runReactants( RDKit::MOL_SPTR_VECT{ p1[0][0], water } );
        auto p3 = rxns[2].runReactants( RDKit::MOL_SPTR_VECT{ p2[0][0] } );
        auto p4 = rxns[3].runReactants( RDKit::MOL_SPTR_VECT{ p3[0][0] } );
        reactant = p4[0][0];
        products.emplace_back( std::move( p1[0][0] ) );
        products.emplace_back( std::move( p2[0][0] ) );
        products.emplace_back( std::move( p3[0][0] ) );
        products.emplace_back( std::move( p4[0][0] ) );
    }

    printer()( products );

    std::ofstream of( "draw.html" );
    std::vector< RDKit::MatchVectType > matchVect;
    if ( RDKit::SubstructMatch( *__pfoa, *__sss, matchVect ) ) { // find all match
        std::vector< int > hit_atoms;
        for ( auto& t: matchVect ) {
            for ( auto& q: t )
                hit_atoms.emplace_back( q.second );
        }
        RDKit::MolDraw2DSVG drawer( 300, 300, of );
        auto bonds = get_all_hit_bonds( *__pfoa, hit_atoms );
        drawer.drawMolecule( *__pfoa, &hit_atoms, &bonds );
        drawer.finishDrawing();
    }
}
