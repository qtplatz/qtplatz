
/**************************************************************************
** Copyright (C) 2010-2018 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2018 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "dataprocessor.hpp"
#include <acqrscontrols/u5303a/waveform.hpp>
#include <adcontrols/datareader.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adfs/sqlite.hpp>
#include <adfs/fs.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/folder.hpp>
#include <adplugin/plugin.hpp>
#include <adplugin_manager/loader.hpp>
#include <adplugin_manager/manager.hpp>
#include <adportable/binary_serializer.hpp>
#include <adportable/debug.hpp>
#include <adportable/spectrum_processor.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <ratio>

namespace po = boost::program_options;

int
main(int argc, char *argv[])
{
    po::variables_map vm;
    po::options_description description( "counting" );
    {
        description.add_options()
            ( "help,h",      "Display this help message" )
            ( "args",         po::value< std::vector< std::string > >(),  "input files" )
            ( "output,o",     "import from text file to adfs" )
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

    ::adfs::filesystem fs;
    
    boost::filesystem::path outfile( "output.adfs" );
    if ( vm.count( "output" ) )
        outfile = boost::filesystem::path( vm[ "output" ].as< std::string >() );

    if ( boost::filesystem::exists( outfile ) ) {
        if ( ! fs.mount( outfile ) )
            return 1;
    } else {
        if ( ! fs.create( outfile ) )
            return 1;
    }
    
	auto folder = fs.addFolder( L"/Processed/Spectra" );

    if ( vm.count("args") ) {
        
        for ( auto& fname: vm[ "args" ].as< std::vector< std::string > >() ) {
            
            boost::filesystem::path path( fname );

            ADDEBUG() << fname;
            
            // if ( path.extension() == ".adfs" ) {

            //     std::cout << path.string() << std::endl;
                
            //     if ( auto file = adcontrols::datafile::open( path.wstring(), false ) ) {
            //         tools::dataprocessor processor;
            //         file->accept( processor );
            //         if ( processor.raw() ) {
            //             for ( auto reader: processor.raw()->dataReaders() ) {
            //                 if ( vm.count( "list-readers" ) ) {
            //                     std::cout << reader->objtext() << ", " << reader->display_name() << ", " << reader->objuuid() << std::endl;
            //                 }
            //             }
            //         }
            //     }
            // }
        }
    }
}

