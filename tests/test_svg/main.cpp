/**************************************************************************
** Copyright (C) 2016-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016-2022 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "mainwindow.hpp"
#include "document.hpp"
#include <QApplication>
#include <boost/program_options.hpp>
#include <iostream>
#include <fstream>
#include <future>

namespace po = boost::program_options;

int
main( int argc, char * argv [] )
{
    QApplication a( argc, argv );

    po::variables_map vm;
    po::options_description description( argv[0] );
    {
        description.add_options()
            ( "help,h",    "Display this help message" )
            ( "load",      po::value< std::string >(), "load method from file" )
            ( "inchikey",  po::value< std::string >()->default_value( "CROAPHWLZZGVPY-UHFFFAOYSA-N" ), "InChIKey" )
            ;
        po::store( po::command_line_parser( argc, argv ).options( description ).run(), vm );
        po::notify(vm);
    }
    if ( vm.count( "help" ) ) {
        std::cout << description;
        return 0;
    }

    if ( vm.count( "inchikey" ) )
        document::instance()->setInChIKey( vm[ "inchikey" ].as< std::string >() );

    MainWindow w;
    w.resize( 800, 600 );
    w.onInitialUpdate();
    w.show();

    QCoreApplication::processEvents();
    a.exec();

    return 0;
}
