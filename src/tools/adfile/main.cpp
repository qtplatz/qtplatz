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
#include <adcontrols/datareader.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adfs/sqlite.hpp>
#include <adplugin/plugin.hpp>
#include <adplugin_manager/loader.hpp>
#include <adplugin_manager/manager.hpp>
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
            ( "list-readers", "list data-reader list" )
            ( "rms",          "rms list" )
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

    if ( vm.count("args") ) {
        
        for ( auto& fname: vm[ "args" ].as< std::vector< std::string > >() ) {
            
            boost::filesystem::path path( fname );
            if ( path.extension() == ".adfs" ) {

                std::cout << path.string() << std::endl;
                
                if ( auto file = adcontrols::datafile::open( path.wstring(), false ) ) {
                    tools::dataprocessor processor;
                    file->accept( processor );
                    if ( processor.raw() ) {
                        for ( auto reader: processor.raw()->dataReaders() ) {
                            if ( vm.count( "list-readers" ) ) {
                                std::cout << reader->objtext() << ", " << reader->display_name() << ", " << reader->objuuid();
                            }
                            if ( vm.count( "rms" ) ) {
                                if ( ( reader->objtext() == "1.u5303a.ms-cheminfo.com" ) || 
                                     ( reader->objtext() == "tdcdoc.waveform.1.u5303a.ms-cheminfo.com" ) ) {

                                    for ( auto it = reader->begin(); it != reader->end(); ++it ) {
                                        if ( auto ms = reader->readSpectrum( it ) ) {
                                            
                                            double tic(0), dbase(0), rms(0);
                                            const double * intensities = ms->getIntensityArray();
                                            
                                            std::tie(tic, dbase, rms) = adportable::spectrum_processor::tic( ms->size(), intensities, 5 );
                                            size_t beg = ms->size() > 100 ? 100 : 0;
                                            size_t end = ms->size() > (beg + 10) ? beg + 10 : ms->size();
                                            auto mm = std::minmax_element( intensities + beg, intensities + end );

                                            std::cout << boost::format("%5d\t%10.4f\t%16.3f\t%12.4f\t%12.4f\tdelta(p-p)=\t%8.5f")
                                                % it->rowid()
                                                % it->time_since_inject()
                                                % tic
                                                % dbase
                                                % rms
                                                % (*mm.second - *mm.first)
                                                      << std::endl;                                            
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

