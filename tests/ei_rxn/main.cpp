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
#include "drawer.hpp"
#include "printer.hpp"
#include "html.hpp"

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


std::vector<std::pair<std::string, std::string>> ei_rules = {
    { "C-C cleavage cation left",
      "[C:1]-[C:2]>>[C+:1].[C:2]" },

    { "C-C cleavage cation right",
      "[C:1]-[C:2]>>[C:1].[C+:2]" },

    { "alpha cleavage next to O/N/S",
      "[C:1]-[C:2]-[O,N,S:3]>>[C+:1].[C:2]-[O,N,S:3]" },

    { "carbonyl alpha cleavage acyl ion",
      "[C:1]-[C:2](=[O:3])-[C:4]>>[C:1].[C+:2]=[O:3].[C:4]" },

    { "C-O cleavage oxonium",
      "[C:1]-[O:2]-[C:3]>>[C:1].[O+:2]-[C:3]" },

    { "loss HX",
      "[C:1]-[C:2]([F,Cl,Br,I:3])>>[C:1]=[C:2].[F,Cl,Br,I:3]" },

    { "C-X cleavage",
      "[C:1]-[F,Cl,Br,I:2]>>[C+:1].[F,Cl,Br,I:2]" },

    { "perfluoro C-C cleavage left",
      "[C:1]([F])([F])-[C:2]([F])([F])>>[C+:1]([F])[F].[C:2]([F])([F])" },

    { "perfluoro C-C cleavage right",
      "[C:1]([F])([F])-[C:2]([F])([F])>>[C:1]([F])([F]).[C+:2]([F])[F]" }
};

class Rule {
    std::string name_;
    std::string smarts_;
    std::optional< RDKit::ChemicalReaction > rxn_;
public:
    Rule() {}
    Rule( const std::pair< std::string, std::string >& rule )
        : name_( std::get<0>(rule) )
        , smarts_( std::get<1>(rule) )
        , rxn_( AllChem::ReactionFromSmarts( smarts_ ) ) {
    }
    Rule( const Rule& t ) : name_(t.name_)
                          , smarts_(t.smarts_)
                          , rxn_( t.rxn_ ) {
    }
    const std::string& name() const { return name_; }
    const std::string& smarts() const { return smarts_; }
    const std::optional< RDKit::ChemicalReaction >& rxn() const { return rxn_; }
};

constexpr const int max_depth = 10;

using product_type = std::pair< RDKit::ROMOL_SPTR, std::string >;
using product_list = std::vector< product_type >;

product_list
apply_rule( const Rule& rule, const RDKit::ROMol& reactant )
{
    product_list products;

    if ( !rule.rxn() )
        return products;

    auto reactant_sptr =
        boost::make_shared< RDKit::ROMol >( reactant );

    auto product_sets =
        rule.rxn()->runReactants( RDKit::MOL_SPTR_VECT{ reactant_sptr } );

    for ( const auto& product_set: product_sets ) {
        for ( const auto& product: product_set ) {
            products.emplace_back( product, rule.name() );
        }
    }

    return products;
}

product_list
apply_rules( const std::vector< Rule >& rules,
             const RDKit::ROMol& analyte )
{
    product_list products;

    for ( const auto& rule: rules ) {
        auto result = apply_rule( rule, analyte );

        products.insert( products.end(),
                         std::make_move_iterator( result.begin() ),
                         std::make_move_iterator( result.end() ) );
    }

    return products;
}

product_list
enumerate_products( const std::vector< Rule >& rules,
                    const RDKit::ROMol& analyte,
                    std::size_t generations )
{
    product_list all_products;

    std::vector< RDKit::ROMOL_SPTR > current {
        boost::make_shared< RDKit::ROMol >( analyte )
    };

    for ( std::size_t gen = 0; gen < generations; ++gen ) {

        std::vector< RDKit::ROMOL_SPTR > next;

        for ( const auto& mol: current ) {

            auto products = apply_rules( rules, *mol );

            for ( const auto& [ product, rule_name ]: products ) {
                all_products.emplace_back( product, rule_name );
                next.emplace_back( product );
            }
        }

        current = std::move( next );
    }

    return all_products;
}

product_list
make_unique_products( product_list&& products )
{
    std::unordered_set< std::string > seen;

    product_list unique;

    for ( auto& [mol, tag] : products ) {

        if ( !mol )
            continue;

        try {
            auto smiles = RDKit::MolToSmiles( *mol, true );

            if ( seen.insert( smiles ).second ) {
                unique.emplace_back(
                    std::move( mol ),
                    std::move( tag )
                );
            }
        } catch ( const std::exception& ex ) {
            std::cout << "MolToSmiles failed: " << ex.what() << std::endl;
        }
    }

    return unique;
}

struct product_record {
    RDKit::ROMOL_SPTR mol;
    std::string smiles;
    std::set< std::string > origins;
};

int
main(int argc, char **argv)
{
    // C1CCOC1
    using namespace RDKit;
    auto __thf = "C1CCOC1"_smiles;

    std::vector< Rule > rules;
    for ( const auto& rule: ei_rules )
        rules.emplace_back( rule );

    auto products = enumerate_products( rules, *__thf, 2 );
    products = make_unique_products( std::move( products ) );

    // for ( const auto& product: products ) {
    //     printer(std::format("{}:\t", product.second ))( *product.first );
    // }

    std::map< std::string, product_record > unique;

    for ( auto& [mol, origin] : products ) {

        if ( not mol )
            continue;

        auto key = RDKit::MolToSmiles( *mol, true );

        auto it = unique.find( key );

        if ( it == unique.end() ) {
            product_record rec;
            rec.mol = mol;
            rec.smiles = key;
            rec.origins.insert( origin );
            unique.emplace( key, std::move( rec ) );
        } else {
            it->second.origins.insert( origin );
        }
    }

    std::cout << "==================================" << std::endl;
    for ( const auto& [key, product]: unique ) {
        auto mass = RDKit::Descriptors::calcExactMW( *product.mol );
        std::cout << std::format( "{:4f}\t{:16s}", mass, RDKit::MolToSmiles( *product.mol ) );
        std::cout << "\torigins: ";
        for ( const auto origin: product.origins )
            std::cout << origin << ", ";
        std::cout << std::endl;
    }
}
