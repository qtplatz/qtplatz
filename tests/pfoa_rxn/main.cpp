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

int
main(int argc, char **argv)
{
    // RDKit::RWMOL_SPTR mol_r(RDKit::SmartsToMol( "[C:1](O)(=O)[C:2]" ) );
    RDKit::ROMOL_SPTR water( RDKit::SmilesToMol("O") );
    water->updatePropertyCache();
    RDKit::ROMOL_SPTR pfoa( RDKit::SmilesToMol( "FC(F)(C(F)(F)C(=O)O)C(F)(F)C(F)(F)C(F)(F)C(F)(F)C(F)(F)F" ) );
    pfoa->updatePropertyCache();

    auto reactant = pfoa;

    std::vector<RDKit::MOL_SPTR_VECT> prods_1;
    if ( auto rxn_1 = AllChem::ReactionFromSmarts( "[C:1](O)(=O)[C:2]>>[C-:2].[C:1](=O)(=O)" ) ) {
        prods_1 = rxn_1->runReactants(RDKit::MOL_SPTR_VECT{ reactant } );
        printer("rxn_1:\t")(*rxn_1);
        printer("prod_1:\t")( prods_1 );
        reactant = prods_1[0][0];
    }

    if ( auto rxn_2 = AllChem::ReactionFromSmarts( "[C-:1].[O:2]>>[C-0:1][O:2].[H+]" ) ) {
        auto prods_2 = rxn_2->runReactants( RDKit::MOL_SPTR_VECT{ reactant, water } );
        printer("rxn_2:\t")( *rxn_2 );
        printer("prod_2:\t")( prods_2 );
    }
    RDKit::ROMOL_SPTR minimum( RDKit::SmilesToMol("C(F).C") );
    RDKit::MatchVectType matchVect;
    std::cout << "substructmatch: " << std::boolalpha << RDKit::SubstructMatch( *reactant, *minimum, matchVect )
              << std::endl;

#if 0
    rxn_3 = AllChem.ReactionFromSmarts("[C:1](O)([F:2])>>[C:1](O).[F:2]");
    rxn_4 = AllChem.ReactionFromSmarts("[C:1](F)(F)>>[C:1]=O");
    while ( reactant.HasSubstructMatch(Chem.MolFromSmarts("C(F).C")) ) {
        p1 = rxn_1.RunReactants((reactant, ))[0];
        p2 = rxn_2.RunReactants((p1[0],water,))[0];
        p3 = rxn_3.RunReactants((p2[0],))[0];
        p4 = rxn_4.RunReactants((p3[0], ))[0];
        Chem.SanitizeMol( p1[0] );
        Chem.SanitizeMol( p2[0] );
        Chem.SanitizeMol( p3[0] );
        Chem.SanitizeMol( p4[0] );
        products.append( reactant );
        products.append( p1[0] );
        products.append( p2[0] );
        products.append( p3[0] );
    }
#endif
    return 0;
}
