/**************************************************************************
** Copyright (C) 2010-2024 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2024 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/

#include <adportable/debug.hpp>
#include <GraphMol/DistGeomHelpers/Embedder.h>
#include <GraphMol/FileParsers/FileParsers.h>
#include <GraphMol/FileParsers/GeneralFileReader.h>
#include <GraphMol/FileParsers/MolWriters.h>
#include <GraphMol/GraphMol.h>
#include <GraphMol/ROMol.h>
#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/SmilesParse/SmilesWrite.h>
#include <GraphMol/ForceFieldHelpers/UFF/UFF.h>
#include <boost/program_options.hpp>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <ratio>

namespace po = boost::program_options;

namespace {

    struct print_xyz {
        print_xyz() {}
        void operator()( const std::vector< std::string >& keywords, const RDKit::ROMol& mol, std::ostream& os ) const {
            if ( keywords.empty() ) {
                os << "PM7 XYZ BONDS STATIC\n\n\n";
            } else {
                os << "PM7\t";
                for ( const auto& keyword: keywords )
                    os << keyword << "\t";
                os << "\n\n\n";
            }
            for ( size_t i = 0; i < mol.getNumAtoms(); ++i ) {
                auto atom = mol.getAtomWithIdx( i );
                auto pos = mol.getConformer().getAtomPos( i );
                os << std::format( "{}\t{:.7}\t{:.7}\t{:.7}", atom->getSymbol(), pos.x, pos.y, pos.z ) << std::endl;
            }
        }
    };
}

int
main(int argc, char *argv[])
{
    po::variables_map vm;
    po::options_description description( argv[0] );
    {
        description.add_options()
            ( "help,h",      "Display this help message" )
            ( "smiles",       po::value< std::vector< std::string > >(),  "smiles" )
            ( "output,o",     po::value< std::string >()->default_value("-"), "output file, '-' for stdout" )
            ( "keywords,k",   po::value< std::vector< std::string > >()->multitoken(), "keywords" )
            ;

        // po::positional_options_description p;
        // p.add( "args",  -1 );
        //po::store( po::command_line_parser( argc, argv ).options( description ).positional(p).run(), vm );
        po::store( po::command_line_parser( argc, argv ).options( description ).run(), vm );
        po::notify(vm);
    }

    std::vector< std::shared_ptr< RDKit::ROMol > > mols;

    if ( vm.count( "help" ) || (vm.count( "smiles" ) == 0) ) {
        std::cout << description;
        return 0;
    }

    const auto& keywords = vm[ "keywords" ].as< std::vector< std::string > >();

    // if ( vm.count( "keywords" ) ) {
    //     for ( auto kw: vm[ "keywords" ].as< std::vector< std::string > >() ) {
    //         std::cerr << "keyword: " << kw << std::endl;
    //     }
    //     std::cerr << std::endl;
    // }

    if ( vm.count("smiles") ) {

        using _P = std::shared_ptr< RDKit::ROMol >;

        for ( auto arg: vm[ "smiles" ].as< std::vector< std::string > >() ) {
            if ( auto mol = _P( RDKit::SmilesToMol( arg ) ) ) {
                if ( auto mol_h = _P( RDKit::MolOps::addHs( *mol ) ) ) {
                    RDKit::DGeomHelpers::EmbedMolecule( *mol_h );
                    RDKit::UFF::UFFOptimizeMolecule( *mol_h );
                    if ( vm[ "output" ].as< std::string >() == "-" )
                        print_xyz()(keywords, *mol_h, std::cout );
                    else {
                        std::ofstream ofs( vm[ "output" ].as<std::string>() );
                        print_xyz()(keywords, *mol_h, ofs );
                    }
                }
            }
        }
    }
}
