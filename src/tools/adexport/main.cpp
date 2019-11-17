/**************************************************************************
** Copyright (C) 2010-2020 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2020 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "export.hpp"
#include <adportable/debug.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <ratio>
#include <QApplication>

namespace po = boost::program_options;

int
main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    po::variables_map vm;
    po::options_description description( argv[ 0 ] );
    {
        description.add_options()
            ( "help,h",      "Display this help message" )
            ( "args",         po::value< std::vector< std::string > >(),  "input files" )
            ( "output,o",        po::value< std::string >()->default_value( "output.adfs" ), "import from text file to adfs" )
            ( "counting_output", po::value< std::string >()->default_value( "output.adfs" ), "output file for counting results" )
            ( "polarity",     po::value< std::string >()->default_value( "POS" ), "threshold polarity{POS|NEG}" )
            ;
        po::positional_options_description p;
        p.add( "args",  -1 );
        po::store( po::command_line_parser( argc, argv ).options( description ).positional(p).run(), vm );
        po::notify(vm);
    }

    if ( vm.count( "help" ) ) {
        std::cout << description;
        return 0;
    }

    // adplugin::manager::standalone_initialize();

    auto filelist = vm[ "args" ].as< std::vector< std::string > >();
    if ( filelist.empty() )
        return 0;

    for ( auto& fname: filelist ) {
        
        adexport::Export exporter;

        if ( exporter.open( boost::filesystem::path( fname ) ) ) {
            if ( exporter.loadFolders() ) {
                for ( const auto& file: exporter.folders() )
                    exporter.out( file, std::cout );
            }
        } else {
            ADDEBUG() << fname << " cannot be opened.";
        }

    }
}
