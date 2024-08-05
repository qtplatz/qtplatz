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
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>
#include <string>
#include "../../src/libs/adportable/smarts_parser.hpp"
// reaction  ::=  reactants ">>" products
// reactants ::=  molecules
// products  ::=  molecules
// molecules ::=  molecule
//                molecules "." molecule
// molecule  ::=  a valid SMARTS string without "." characters
//                 "(" a valid SMARTS string without "." characters ")"

#define BOOST_SPIRIT_X3_DEBUG

struct AllChem {
    std::optional< RDKit::ChemicalReaction >
    ReactionFromSmarts( std::string smarts ) const {
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
};

int
main(int argc, char **argv)
{
    // using smarts_parser = adportable::smarts_parser;
    // if ( auto ctx = smarts_parser::parse( "[S:2](O)(=O)([C:1])>>[S:2](O)(=O)O.[C:1]*") ) {
    //     for ( const auto& r: std::get<0>(*ctx) )
    //         std::cout << "reactant: " << r << std::endl;
    //     for ( const auto& p: std::get<1>(*ctx) )
    //         std::cout << "product: " << p << std::endl;
    // }

    RDKit::ROMOL_SPTR water( RDKit::SmilesToMol("O") );
    RDKit::ROMOL_SPTR pfoa( RDKit::SmilesToMol( "FC(F)(C(F)(F)C(=O)O)C(F)(F)C(F)(F)C(F)(F)C(F)(F)C(F)(F)F" ) );
    water->updatePropertyCache();
    pfoa->updatePropertyCache();

    std::cout << RDKit::MolToSmiles( *pfoa ) << "\t" << RDKit::Descriptors::calcExactMW( *pfoa ) << std::endl;

    RDKit::MOL_SPTR_VECT reacts;

    // "rxn_1 = AllChem.ReactionFromSmarts('[S:2](O)(=O)([C:1])>>[S:2](O)(=O)O.[C:1]*')\n",
#if 0
    auto rxn_a = AllChem().ReactionFromSmarts( "[S:2](O)(=O)([C:1])>>[S:2](O)(=O)O.[C:1]*" );
    if ( rxn_a ) {
        RDKit::MOL_SPTR_VECT reacts = { pfoa };
        auto products = rxn_a->runReactants( RDKit::MOL_SPTR_VECT { pfoa } );
        for ( auto prod: products ) {
            size_t id(0);
            for ( auto p: prod ) {
                p->updatePropertyCache();
                RDKit::Descriptors::calcExactMW( *p );
                std::cout << "\t" << ++id << ":\t" << RDKit::MolToSmiles( *p )
                          << "\t" << RDKit::Descriptors::calcExactMW( *p )
                          << std::endl;
            }
        }
        reacts.emplace_back( products[0][0] );
    }
#else
    reacts = { pfoa };
    RDKit::ChemicalReaction rxn_1;
    rxn_1.addReactantTemplate( RDKit::ROMOL_SPTR( RDKit::SmartsToMol( "[S:2](O)(=O)([C:1])" ) ) );
    rxn_1.addProductTemplate( RDKit::ROMOL_SPTR( RDKit::SmartsToMol( "[S:2](O)(=O)O" ) ) );
    rxn_1.addProductTemplate( RDKit::ROMOL_SPTR( RDKit::SmartsToMol( "[C:1]*" ) ) );
    rxn_1.initReactantMatchers();

    reacts.clear();
    reacts.emplace_back( pfoa );

    std::vector<RDKit::MOL_SPTR_VECT> prods = rxn_1.runReactants( reacts );
    std::cout << "prods.size = " << prods.size() << std::endl;
    for ( auto prod: prods ) {
        size_t id(0);
        for ( auto p: prod ) {
            p->updatePropertyCache();
            RDKit::Descriptors::calcExactMW( *p );
            std::cout << "\t" << ++id << ":\t" << RDKit::MolToSmiles( *p )
                      << "\t" << RDKit::Descriptors::calcExactMW( *p )
                      << std::endl;
        }
    }
#endif
    //---------------------------------------------------------
    RDKit::ChemicalReaction rxn_2;
    // rxn_2.addReactantTemplate( RDKit::ROMOL_SPTR( "[C-:1].[O:2]>>[C-0:1][O:2].[H+]" ) );
    rxn_2.addReactantTemplate( RDKit::ROMOL_SPTR( RDKit::SmartsToMol("[C-:1]") ) );
    rxn_2.addReactantTemplate( RDKit::ROMOL_SPTR( RDKit::SmartsToMol("[O:2]") ) );
    rxn_2.addProductTemplate( RDKit::ROMOL_SPTR( RDKit::SmartsToMol("[C-0:1][O:2]") ) );
    rxn_2.addProductTemplate( RDKit::ROMOL_SPTR( RDKit::SmartsToMol("[H+]") ) );
    rxn_2.initReactantMatchers();
    reacts.emplace_back( water );
    std::vector<RDKit::MOL_SPTR_VECT> prods2 = rxn_2.runReactants( reacts );
    reacts.clear();
    for ( auto prod: prods2 ) {
        size_t id(0);
        for ( auto p: prod ) {
            p->updatePropertyCache();
            RDKit::Descriptors::calcExactMW( *p );
            std::cout << "\t" << ++id << ":\t" << RDKit::MolToSmiles( *p )
                      << "\t" << RDKit::Descriptors::calcExactMW( *p )
                      << std::endl;
        }
        // reacts.emplace_back( prods[0][0] );
    }
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
