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
            ( "list-readers", "list data-reader list" )
            ( "device-data",  "list device meta data" )
            ( "rms",          "rms list" )
            ( "pp",           po::value< double >(), "find p-p delta larger than value" )
            ( "at",           po::value< uint32_t >()->default_value(100), "rms start point in the waveform" )
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

    double pp_threshold(0);
    bool find_pp( false );

    if ( vm.count( "pp" ) ) {
        pp_threshold = vm[ "pp" ].as< double >();
        find_pp = true;
    }

    const uint32_t startPoint = vm[ "at" ].as< uint32_t >();

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
                                std::cout << reader->objtext() << ", " << reader->display_name() << ", " << reader->objuuid() << std::endl;
                            }
                            
                            if ( vm.count( "rms" ) || vm.count("device-data" ) ) {
                                if ( ( reader->objtext() == "1.u5303a.ms-cheminfo.com" ) || // u5303a waveform (either average, digitizer mode)
                                     ( reader->objtext() == "tdcdoc.waveform.1.u5303a.ms-cheminfo.com" ) ) { // software averaged waveform

                                    std::cout << "#" << reader->objtext() << std::endl;
                                    std::cout << "#rowid\tretention-time(s)\ttic\t\tdbase\t\trms\t\tdelta(p-p)" << std::endl;

                                    for ( auto it = reader->begin(); it != reader->end(); ++it ) {
                                        if ( auto ms = reader->readSpectrum( it ) ) {
                                            
                                            if ( vm.count( "device-data" ) ) {
                                                acqrscontrols::u5303a::device_data device_data;
                                                if ( adportable::binary::deserialize<>()( device_data
                                                                                          , ms->getMSProperty().deviceData()
                                                                                          , ms->getMSProperty().deviceDataSize() ) ) {
                                                    std::cout << device_data.ident_.Identifier()
                                                              << ",\t" << device_data.ident_.Revision()
                                                              << ",\t" << device_data.ident_.Vendor()
                                                              << ",\t" << device_data.ident_.Description()
                                                              << ",\t" << device_data.ident_.InstrumentModel()
                                                              << ",\t" << device_data.ident_.FirmwareRevision()
                                                              << ",\t" << device_data.ident_.SerialNumber()
                                                              << ",\t" << device_data.ident_.Options()
                                                              << ",\t" << device_data.ident_.IOVersion()
                                                              << ",\t\t" << device_data.meta_.initialXTimeSeconds
                                                              << ",\t" << device_data.meta_.actualPoints
                                                              << ",\t" << device_data.meta_.flags
                                                              << ",\t" << device_data.meta_.actualAverages
                                                              << ",\t" << device_data.meta_.actualRecords
                                                              << ",\t" << device_data.meta_.initialXOffset
                                                              << ",\t" << device_data.meta_.xIncrement
                                                              << ",\t" << device_data.meta_.scaleFactor
                                                              << ",\t" << device_data.meta_.scaleOffset
                                                              << ",\t" << device_data.meta_.dataType
                                                              << ",\t" << device_data.meta_.protocolIndex
                                                              << ",\t" << device_data.meta_.channelMode
                                                              << std::endl;
                                                }
                                            }

                                            if ( vm.count( "rms" ) ) {
                                                double epoch_time = double( ms->getMSProperty().timeSinceEpoch() ) * 1.0e-9;
                                                double tic(0), dbase(0), rms(0);
                                                const double * intensities = ms->getIntensityArray();
                                            
                                                std::tie(tic, dbase, rms) = adportable::spectrum_processor::tic( ms->size(), intensities, 5 );

                                                // const size_t samplePoints = 10;
                                                std::vector < double > ppv;
                                                for ( size_t samplePoints: { 10, 100, 1000 } ) {
                                                    size_t beg = ms->size() > startPoint ? startPoint : 0;
                                                    size_t end = ms->size() > (beg + samplePoints) ? beg + samplePoints : ms->size();
                                                    auto mm = std::minmax_element( intensities + beg, intensities + end );
                                                    //double pp = *mm.second - *mm.first;
                                                    ppv.emplace_back( *mm.second - *mm.first );
                                                }
                                                double pp = ppv[0];

                                                std::cout << boost::format("%5d\t%10.4f\t%10.1f\t%16.3f\t%12.4f\t%12.4f\t%8.5f")
                                                    % it->rowid()
                                                    % it->time_since_inject()
                                                    % epoch_time
                                                    % tic
                                                    % dbase
                                                    % rms
                                                    % pp
                                                          << boost::format( "\t%8.5f\t%8.5f" ) % ppv[1] % ppv[2];
                                                
                                                if ( find_pp && pp > pp_threshold )
                                                    std::cout << "\t *** '>' " << pp_threshold;
                                                std::cout << std::endl;
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
}

