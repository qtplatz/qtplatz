/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "document.hpp"
#include "mainwindow.hpp"
#include <adprocessor/dataprocessor.hpp>
#include <adplugin_manager/manager.hpp>
#include <QApplication>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ratio>

#if OPENCV
//# include <cv.h>
# include <opencv2/cvconfig.h>
# include <opencv2/flann/flann.hpp>
# include <opencv2/highgui/highgui.hpp>
#endif

namespace po = boost::program_options;

int
main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    po::variables_map vm;
    po::options_description description( "counting2d" );
    {
        description.add_options()
            ( "help,h",      "Display this help message" )
            ( "args",        po::value< std::vector< std::string > >(),  "input files" )
            ( "directory,C", po::value< std::string >(), "result output directory" )
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

    adplugin::manager::standalone_initialize();

    bool f_directory( false );

    auto cwd = std::filesystem::current_path();

    if ( vm.count( "directory" ) ) {
        std::filesystem::path cdir( vm[ "directory" ].as< std::string >() );
        if ( !std::filesystem::exists( cdir ) )
            std::filesystem::create_directories( cdir );
        if ( std::filesystem::exists( cdir ) && !std::filesystem::is_directory( cdir ) ) {
            std::cerr << "Directory " << cdir << " is not a directory." << std::endl;
            return -1;
        }
        std::filesystem::current_path( cdir );
        f_directory = true;
    }

    counting2d::MainWindow w;
    w.resize( 800, 600 );
    auto doc = counting2d::document::instance();
    w.onInitialUpdate();
    w.show();
    a.processEvents();

    if ( vm.count("args") ) {

        for ( auto& _file: vm[ "args" ].as< std::vector< std::string > >() ) {

            std::filesystem::path file = f_directory ? std::filesystem::canonical( _file ).string() : _file;

            std::string msg;
            auto processor = std::make_shared< adprocessor::dataprocessor >();

            if ( processor->open( file.wstring(), msg ) ) {

                if ( doc->setDataprocessor( processor ) ) {
                    doc->fetch();
                    break;
                }

            } else {
                std::cout << msg << std::endl;
            }
        }
    }

    a.exec();
}
