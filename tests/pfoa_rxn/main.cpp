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

int
main(int argc, char **argv)
{
    // RDKit::RWMOL_SPTR mol_r(RDKit::SmartsToMol( "[C:1](O)(=O)[C:2]" ) );
    RDKit::ROMOL_SPTR pfoa( RDKit::SmilesToMol( "FC(F)(C(F)(F)C(=O)O)C(F)(F)C(F)(F)C(F)(F)C(F)(F)C(F)(F)F" ) );
    RDKit::ROMOL_SPTR water( RDKit::SmilesToMol("O") );
    RDKit::ROMOL_SPTR core( RDKit::SmilesToMol("C(F).C") );
    for ( const auto& mol: { pfoa, water, core } )
        mol->updatePropertyCache();

    auto reactant = pfoa;

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

    reactant = pfoa;
    RDKit::MOL_SPTR_VECT products;
    RDKit::MatchVectType matchVect;

    while ( RDKit::SubstructMatch( *reactant, *core, matchVect ) ) {
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

    return 0;
}
