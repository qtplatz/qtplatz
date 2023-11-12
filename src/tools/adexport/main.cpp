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
#include <adcontrols/datainterpreterbroker.hpp>
#include <adcontrols/datareader.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adplugin_manager/loader.hpp>
#include <adplugin_manager/manager.hpp>
#include <adportable/debug.hpp>
#include <adprocessor/dataprocessor.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <filesystem>
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
            ( "help,h",        "Display this help message" )
            ( "args",          po::value< std::vector< std::string > >(),  "input files" )
            ( "output,o",      po::value< std::string >(),                 "output file name (.xml)" )
            ( "list-readers",  "list data readers" )
            ( "device-data",   "list device meta data" )
            ;
        po::positional_options_description p;
        p.add( "args",  -1 );
        po::store( po::command_line_parser( argc, argv ).options( description ).positional(p).run(), vm );
        po::notify(vm);
    }

    if ( vm.count( "help" ) || argc == 1 ) {
        std::cout << description;
        return 0;
    }

    adplugin::manager::standalone_initialize();

    auto filelist = vm[ "args" ].as< std::vector< std::string > >();
    if ( filelist.empty() )
        return 0;

    for ( auto& fname: vm[ "args" ].as< std::vector< std::string > >() ) {

        std::filesystem::path path( fname );
        std::error_code ec;
        if ( ( path.extension() == ".adfs" ) && std::filesystem::exists( path, ec ) ) {

            if ( auto dp = std::make_shared< adprocessor::dataprocessor >() ) {
                std::string msg;
                if ( dp->open( path.string(), msg ) ) {
                    if ( vm.count( "list-readers" ) ) {
                        if ( auto raw = dp->rawdata() ) {
                            for ( auto reader: raw->dataReaders() )
                                std::cout << reader->objtext() << "\t'" << reader->display_name() << "'\t" << reader->objuuid() << std::endl;
                        }
                    }
                    if ( vm.count( "device-data" ) ) {
                        if ( auto raw = dp->rawdata() ) {
                            for ( auto reader: raw->dataReaders() ) {
                                if ( auto ms = reader->readSpectrum( reader->begin() ) ) {
                                    if ( auto sp = reader->massSpectrometer() ) {
                                        if ( auto interpreter
                                             = adcontrols::DataInterpreterBroker::make_datainterpreter( sp->dataInterpreterUuid() ) ) {
                                            std::vector < std::pair< std::string, std::string > > textv;
                                            interpreter->make_device_text( textv, ms->getMSProperty() );
                                            std::cout << reader->objtext() << "\t'" << reader->display_name() << "'\t" << reader->objuuid() << std::endl;
                                            for ( const auto& value: textv )
                                                std::cout << value.first << "\t'" << value.second << std::endl;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    //
                }
            }
        }
    }
}
