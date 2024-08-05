#include <iostream>
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

    RDKit::ChemicalReaction rxn_1;
    rxn_1.addReactantTemplate( RDKit::ROMOL_SPTR( RDKit::SmartsToMol( "[C:1](O)(=O)[C:2]" ) ) );
    rxn_1.addProductTemplate( RDKit::RWMOL_SPTR(RDKit::SmartsToMol( "[C-:2]" ) ) );
    rxn_1.addProductTemplate( RDKit::RWMOL_SPTR(RDKit::SmartsToMol( "[C:1](=O)(=O)" ) ) );

    printer("rxn_1:\t")(rxn_1);

    rxn_1.initReactantMatchers();
    std::vector<RDKit::MOL_SPTR_VECT> prods = rxn_1.runReactants(RDKit::MOL_SPTR_VECT{ pfoa } );
    std::cout << std::endl;
    printer("prod_1:\t")( prods );

    //---------------------------------------------------------
    RDKit::ChemicalReaction rxn_2;
    //rxn_2.addReactantTemplate( RDKit::ROMOL_SPTR( "[C-:1].[O:2]>>[C-0:1][O:2].[H+]" ) );
    rxn_2.addReactantTemplate( RDKit::ROMOL_SPTR( RDKit::SmartsToMol("[C-:1]") ) );
    rxn_2.addReactantTemplate( RDKit::ROMOL_SPTR( RDKit::SmartsToMol("[O:2]") ) );
    rxn_2.addProductTemplate( RDKit::ROMOL_SPTR( RDKit::SmartsToMol("[C-0:1][O:2]") ) );
    rxn_2.addProductTemplate( RDKit::ROMOL_SPTR( RDKit::SmartsToMol("[H+]") ) );
    rxn_2.initReactantMatchers();
    printer("rxn_2:\t")( rxn_2 );

    std::vector<RDKit::MOL_SPTR_VECT> prods2 = rxn_2.runReactants( RDKit::MOL_SPTR_VECT{ prods[0][0], water } );
    printer("prod_2:\t")( prods2 );


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
