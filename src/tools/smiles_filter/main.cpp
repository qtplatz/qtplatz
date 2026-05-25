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
#include <boost/program_options.hpp>
#include <boost/spirit/home/x3/support/unused.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <fstream>
#include <variant>

using filter_functor = std::function<void( adportable::csv::list_type&&, std::unique_ptr< RDKit::RWMol >&& )>;

auto null_functor = []( adportable::csv::list_type&&
                        , std::unique_ptr< RDKit::RWMol >&& )->void{};

class smiles_reader {
    filter_functor exclude_;
    filter_functor include_;
    mutable std::tuple< size_t, size_t, size_t > counts_;

public:
    ~smiles_reader() {}
    smiles_reader( filter_functor include = null_functor
                   , filter_functor exclude = null_functor ) : include_( include )
                                                             , exclude_( exclude )
                                                             , counts_{0, 0, 0} {
    }
    void operator()( std::istream& inf,
                     const std::vector< std::function<bool( const RDKit::RWMol& )> > filters ) const {
        adportable::csv::csv_reader reader{};
        adportable::csv::list_type list;
        while ( reader.read( inf, list ) && inf.good() ) {
            for ( const auto& value: list ) {
                if ( value.type() == typeid( std::string ) ) {
                    auto str = boost::get< std::string >( value );
                    if ( not str.empty() ) {
                        try {
                            std::unique_ptr< RDKit::RWMol > mol( RDKit::SmilesToMol( str ) );
                            if ( mol ) {
                                bool include(false);
                                if ( str.find( '.' ) != std::string::npos ) {
                                    std::get<1>(counts_)++;
                                    exclude_( std::move( list ), std::move( mol ) );
                                } else {
                                    for ( auto& filter: filters ) {
                                        if ( not filter( *mol ) ) {
                                            std::get<1>(counts_)++;
                                            exclude_( std::move( list ), std::move( mol ) );
                                        }
                                    }
                                    std::get<0>(counts_)++;
                                    include_( std::move( list ), std::move( mol ) );
                                }
                            }
                        } catch ( std::exception& ex ) {
                            std::get<2>(counts_)++;
                            ADDEBUG() << "Exception for " << str << " :\t" << ex.what();
                        }
                    }
                }
            }
            auto sum = std::apply([](auto... args) { return (args + ...);}, counts_);
            if ( (sum % 10000) == 0 ) {
                std::cerr << std::format( "Total {} recored processed. {} excluded, {} included, and {} error\n"
                                          , sum, std::get<0>( counts_), std::get<1>( counts_ ), std::get<2>(counts_) );
            }
        }
        ADDEBUG() << counts_;
    }
};

struct smiles_size_filter {
    std::pair< double, double > range_;
    smiles_size_filter( std::pair< double, double >&& r = {0, 1000}) : range_( r ) {};
    bool operator()( const RDKit::RWMol& mol ) const {
        auto mass = RDKit::Descriptors::calcExactMW( mol );
        return range_.first <= mass && mass <= range_.second;
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
            ( "files,f",   po::value< std::vector< std::string > >()->multitoken(), "input files, '-' for stdin" )
            ( "upper_mass",   po::value< double >()->default_value( 300 ), "upper_mass" )
            ( "lower_mass",   po::value< double >()->default_value(   0 ), "lower_mass" )
            ;
        po::store( po::command_line_parser( argc, argv ).options( description ).run(), vm );
        po::notify(vm);
    }

    if ( vm.count( "help" ) || (vm.count( "files" ) == 0 ) ) {
        std::cerr << description << std::endl;
        std::cerr << std::endl;
        return 0;
    }

    std::pair< double, double > mass_range{ vm[ "lower_mass" ].as< double >()
                                            , vm.count( "upper_mass" )
                                            ? vm[ "upper_mass" ].as< double >()
                                            : std::numeric_limits<double>::max() };

    const auto output = vm["output"].as< std::string >();
    std::ofstream file;
    if ( output != "-" )
        file.open( output );
    std::ostream& outf = ( output == "-" ) ? std::cout : file;

    auto size_filter = [&]( const RDKit::RWMol& mol ) {
        auto mass = RDKit::Descriptors::calcExactMW( mol );
        return mass_range.first <= mass && mass <= mass_range.second;
    };
    auto charge_filter = []( const RDKit::RWMol& mol ) {
        return RDKit::MolOps::getFormalCharge( mol ) == 0;
    };
    auto atom_filter = []( const RDKit::RWMol& mol ) {
        // C, H, N, O, S, F, Cl, Br, I, Si
        // 6, 1, 7, 8, 16, 9, 17, 35, 53, 14
        std::set< int > atomicNums = { 6, 1, 7, 8, 16, 9, 17, 35, 53, 14 };
        for ( const auto& atom: mol.atoms() ) {
            if ( atomicNums.find( atom->getAtomicNum() ) == atomicNums.end() )
                return false;
        }
        return true;
    };
    auto ringCount_filter = []( const RDKit::RWMol& mol ) {
        return mol.getRingInfo()->numRings() < 6;
    };
    auto oxygenCount_filter = []( const RDKit::RWMol& mol ) {
        return
            std::accumulate( mol.atoms().begin(), mol.atoms().end(), 0
                             , [](const auto a, const auto b){ return a + (b->getAtomicNum() == 8 ? 1 : 0); })
            < 6;
    };
    auto TPSA_filter = []( const RDKit::RWMol& mol ) {
        // Topological Polar Surface Area
        return RDKit::Descriptors::calcTPSA( mol ) <= 90;
    };
    auto HBD_filter = []( const RDKit::RWMol& mol ) {
        // Hydrogen Bond Donors
        return RDKit::Descriptors::calcNumHBD( mol ) < 3;
    };
    auto HBA_filter = []( const RDKit::RWMol& mol ) {
        // Hydrogen Bond Acceptors
        return RDKit::Descriptors::calcNumHBA( mol ) < 5;
    };
    auto heavyAtom_filter = []( const RDKit::RWMol& mol ) {
        return mol.getNumHeavyAtoms() <= 35;
    };

    auto rotBond_filter = []( const RDKit::RWMol& mol ) {
        return RDKit::Descriptors::calcNumRotatableBonds( mol ) <= 12;
    };

    auto singleComponent_filter = []( const std::string& smiles ) {
        return smiles.find('.') == std::string::npos;
    };

    std::vector< std::function< bool(const RDKit::RWMol&) > > filters{
        size_filter
        , atom_filter
        , charge_filter
        , ringCount_filter
        , oxygenCount_filter
        , TPSA_filter
        , HBD_filter
        , HBA_filter
        , heavyAtom_filter
        , rotBond_filter
    };

    auto inclusion = [&]( adportable::csv::list_type&& list, std::unique_ptr< RDKit::RWMol >&& mol ) {
        for ( const auto& value: list ) {
            outf << value << "\t";
        }
        outf << std::endl;
    };

    auto exclusion = [&]( adportable::csv::list_type&& list, std::unique_ptr< RDKit::RWMol >&& mol ) {
        // do nothing
    };

    smiles_reader reader( inclusion, exclusion );
    for ( const auto& file: vm[ "files" ].as< std::vector< std::string > >() ) {
        ADDEBUG() << file;
        if ( file == "-" ) {
            reader( std::cin, filters );
        } else {
            std::ifstream inf( file );
            reader( inf, filters );
        }
    }
}
