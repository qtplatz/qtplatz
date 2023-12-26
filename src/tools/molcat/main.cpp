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
#include <GraphMol/FileParsers/GeneralFileReader.h>
#include <GraphMol/FileParsers/FileParsers.h>
#include <GraphMol/FileParsers/MolWriters.h>
#include <GraphMol/GraphMol.h>
#include <GraphMol/ROMol.h>
#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/SmilesParse/SmilesWrite.h>
#include <boost/program_options.hpp>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <ratio>

namespace po = boost::program_options;

int
main(int argc, char *argv[])
{
    po::variables_map vm;
    po::options_description description( argv[0] );
    {
        description.add_options()
            ( "help,h",      "Display this help message" )
            ( "args",         po::value< std::vector< std::string > >(),  "input files" )
            ( "smiles",       "output as smiles" )
            ( "output,o",     po::value< std::string >()->default_value("-"), "output file, '-' for stdout" )
            ;

        po::positional_options_description p;
        p.add( "args",  -1 );
        po::store( po::command_line_parser( argc, argv ).options( description ).positional(p).run(), vm );
        po::notify(vm);
    }

    std::vector< RDKit::RWMol > mols;

    if ( vm.count( "help" ) || (vm.count( "args" ) == 0) ) {
        std::cout << description;
        return 0;
    }

    if ( vm.count("args") ) {

        typedef std::unique_ptr< RDKit::RWMol > _P;

        for ( auto infile: vm[ "args" ].as< std::vector< std::string > >() ) {

            std::filesystem::path path( infile );
            std::ifstream ifs( path );
            if ( ifs.is_open() ) {
                const std::string content( (std::istreambuf_iterator<char>(ifs) ),
                                           (std::istreambuf_iterator<char>() ) );

                if ( path.extension() == ".mol" ) {
                    if ( auto pmol = _P( RDKit::MolBlockToMol( content ) ) ) {
                        mols.emplace_back( *pmol );
                    }
                }
            }
        }
    }

    if ( not mols.empty() ) {
        std::unique_ptr< RDKit::MolWriter > writer;
        if ( vm.count("smiles" ) )
            writer = std::make_unique< RDKit::SmilesWriter >( vm[ "output" ].as< std::string >() );
        else
            writer = std::make_unique< RDKit::SDWriter >( vm[ "output" ].as< std::string >() );

        if ( writer ) {
            for ( const auto mol: mols )
                writer->write( mol );
        }
    }
}
