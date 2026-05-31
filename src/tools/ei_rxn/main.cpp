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

#include <adportable/debug.hpp>
#include <adportable/csv_reader.hpp>
#include <adportable/csv_string_visitor.hpp>
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
#include "resultwriter.hpp"
#include "product_record.hpp"
#include <boost/program_options.hpp>
#include <iostream>

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
#if 0
    , { "alcohol dehydration neutral loss H2O",
        "[C:1]-[C:2]-[O:3]>>[C:1]=[C:2]" }
#else
    , { "alcohol dehydration neutral loss H2O",
        "[C;!R;H1,H2,H3:1]-[C;!R:2]-[OX2H:3]>>[C:1]=[C:2]" }
#endif
    , { "geminal dihalide loss halogen radical",
        "[C:1]([Cl,Br,I:2])[Cl,Br,I:3]>>[C+:1][Cl,Br,I:2].[Cl,Br,I:3]" }
};

std::vector< std::pair<std::string, std::string> > neutral_losses = {
    { "-H2O",                        // name
      "[C:1][C:2][OX2H:3]>>[C:1]=[C:2]"  // alcohol only
    }
    , { "-CO2",
        "[C:1][CX3:2](=O)[OX2H:3]>>[C:1]"
    }
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
            RDKit::MolOps::symmetrizeSSSR( *product );
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

    all_products.emplace_back( boost::make_shared< RDKit::ROMol >( analyte ), "[M]+" );

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


class predict {
public:
    std::pair< std::unique_ptr< RDKit::RWMol >, std::map< std::string, product_record > >
    operator()( const std::string& smiles, const std::vector< Rule >& rules ) {
        using namespace RDKit;
        std::unique_ptr< RDKit::RWMol > analyte( RDKit::SmilesToMol( smiles ) );
        return (*this)( std::move( analyte ), rules );
    }

    std::pair< std::unique_ptr< RDKit::RWMol >, std::map< std::string, product_record > >
    operator()( std::unique_ptr< RDKit::RWMol >&& analyte, const std::vector< Rule >& rules ) {

        auto products = enumerate_products( rules, *analyte, 2 );
        products = make_unique_products( std::move( products ) );

        std::map< std::string, product_record > unique;

        for ( auto& [mol, origin] : products ) {

            if ( not mol ) {
                continue;
            }

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
        // writeResult( output, *analyte, synonyms, unique );
        return { std::move( analyte ), std::move( unique ) };
    }

    static void writeResult( const std::string& output
                             , const RDKit::RWMol& analyte
                             , const std::map< std::string, product_record >& unique
                             , const std::vector< std::string >& synonyms ) {

        if ( output == "-" || std::filesystem::path( output ).extension() != ".db" ) {

            for ( const auto& [key, product]: unique ) {
                auto mass = RDKit::Descriptors::calcExactMW( *product.mol );
                std::cout << std::format( "{:4f}\t{:16s}", mass, RDKit::MolToSmiles( *product.mol ) );
                std::cout << "\torigins: ";
                for ( const auto origin: product.origins )
                    std::cout << origin << ", ";
                std::cout << std::endl;
            }
        } else {
            resultWriter writer;
            if ( writer.open( std::filesystem::path( output ) ) ) {
                writer.write( analyte, synonyms, unique );
            }
        }
    }

};

class smiles_reader {
public:
    ~smiles_reader() {}
    smiles_reader() {}

    ssize_t find_smiles_column( const adportable::csv::list_type& list ) const {
        for ( size_t i = 0; i < list.size(); ++i ) {
            const auto& value = list.at(i);
            if ( value.type() == typeid( std::string ) ) {
                auto str = boost::get< std::string >( value );
                if ( not str.empty() ) {
                    try {
                        std::unique_ptr< RDKit::RWMol > mol( RDKit::SmilesToMol( str ) );
                        if ( mol )
                            return i;
                    } catch ( std::exception& ) {
                        // ignore error
                    }
                }
            }
        }
        return (-1);
    }

    std::vector< std::string >
    make_synonyms( const adportable::csv::list_type& list, ssize_t smiles_column ) const {
        std::vector< std::string > res{};
        for ( size_t i = 0; i < list.size(); ++i ) {
            if ( i != smiles_column )
                res.emplace_back( boost::apply_visitor( adportable::csv::string_visitor(), list[i] ) );
        }
        return res;
    }

    void operator()( std::istream& inf
                     , const std::function<void( std::unique_ptr< RDKit::RWMol >&&
                                                 , std::vector< std::string >&& )> forwarder ) const {
        adportable::csv::csv_reader reader{};
        adportable::csv::list_type list;
        ssize_t smiles_column = (-1);
        size_t rdCount = 0;
        while ( reader.read( inf, list ) && inf.good() ) {
            if ( smiles_column == (-1) )
                smiles_column = find_smiles_column( list );
            if ( smiles_column >= 0 ) {
                auto smiles = boost::get< std::string >( list.at( smiles_column ) );
                auto synonyms = make_synonyms( list, smiles_column );
                std::unique_ptr< RDKit::RWMol > mol( RDKit::SmilesToMol( smiles ) );
                if ( mol ) {
                    forwarder( std::move( mol ), std::move( synonyms ) );
                }
            }
        }
    }
};


int
main(int argc, char **argv)
{
    namespace po = boost::program_options;
    po::variables_map vm;
    po::options_description description( argv[0] );
    {
        description.add_options()
            ( "help,h",      "Display this help message" )
            ( "smiles",       po::value< std::string >(),  "smiles" )
            ( "output,o",     po::value< std::string >()->default_value("-"), "output file, '-' for stdout" )
            ( "synonyms",   po::value< std::vector< std::string > >()->multitoken(), "synonyms" )
            ( "csv", po::value< std::string >(),  "read given csv file" )
            ;
        po::store( po::command_line_parser( argc, argv ).options( description ).run(), vm );
        po::notify(vm);
    }

    if ( vm.count( "help" ) ) {
        std::cerr << description << std::endl;
        std::cerr << std::endl;
        std::cerr << "example usage: "
                  << std::format( "{} --smiles=C1CCOC1 --synonyms THF Tetrahydrofuran\n",  argv[0] );
        return 0;
    }

    std::vector< Rule > rules;
    for ( const auto& rule: ei_rules )
        rules.emplace_back( rule );
    for ( const auto& rule: neutral_losses )
        rules.emplace_back( rule );

    if ( vm.count( "csv" ) ) {
        auto output = vm["output"].as<std::string>();
        // forward functor
        auto callback = [output,&rules](std::unique_ptr< RDKit::RWMol >&& mol
                                        , std::vector< std::string >&& synonyms ){
            static size_t wrtCount = 0;
            try {
                auto [analyte, unique] = predict{}( std::move( mol ), rules );
                predict::writeResult( output, *analyte, unique, synonyms );
            } catch ( std::exception& ex ) {
                ADDEBUG() << "## Exception: " << ex.what();
            }
        };
        //<---

        auto csv = vm[ "csv" ].as<std::string>();
        smiles_reader reader{};
        std::ifstream file;
        if ( csv != "-" )
            file.open( csv );
        std::istream& inf = (csv == "-") ? std::cin : file;
        reader( inf, callback );

    } else {
        std::string smiles = "C1CCOC1";
        std::vector< std::string > synonyms = { "THF", "Tetrahydrofuran" };
        if ( vm.count( "smiles" ) ) {
            smiles = vm[ "smiles" ].as< std::string >();
            synonyms.clear();
        }
        if ( vm.count( "synonyms" ) ) {
            synonyms = vm[ "synonyms" ].as< std::vector< std::string > >();
        }

        auto [analyte, unique ] = predict{}( smiles, rules );
        predict::writeResult( vm["output"].as<std::string>()
                              , *analyte
                              , unique
                              , synonyms );
    }

}
