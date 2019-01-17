/**************************************************************************
** Copyright (C) 2010-2019 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2019 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include <adplugins/adtextfile/dialog.hpp>
#include <adplugins/adtextfile/txtspectrum.hpp>
#include <adplugins/adtextfile/time_data_reader.hpp>
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
#include <adportable/textfile.hpp>
#include <adportable/utf.hpp>
#include <adportfolio/portfolio.hpp>
#include <adportfolio/folder.hpp>
#include <adportfolio/folium.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <ratio>
#include <QApplication>
#include <QMessageBox>
#include <QString>
#include <QStringList>

namespace po = boost::program_options;

int
main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    po::variables_map vm;
    po::options_description description( "adimport" );
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
    auto filelist = vm[ "args" ].as< std::vector< std::string > >();
    if ( filelist.empty() )
        return 0;

    //--- determine file type
    adtextfile::Dialog dlg;
    do {
        QStringList models;
        for ( auto& model: adcontrols::MassSpectrometer::get_model_names() )
            models << QString::fromStdWString( model );
        dlg.setDataInterpreterClsids( models );

        boost::filesystem::path path( filelist[0] );
        dlg.setDataType( adtextfile::Dialog::data_spectrum );

        boost::filesystem::ifstream in( path );
        if ( in.fail() ) {
            ADDEBUG() << "Cannot open fil: " << path.string();
            return 1;
        }
        typedef boost::char_separator<char> separator;
        typedef boost::tokenizer< separator > tokenizer;
        separator sep( ", \t", "", boost::drop_empty_tokens );
        std::string line;
        size_t nlines = 50;
        while ( nlines-- && adportable::textfile::getline( in, line ) ) {
            tokenizer tokens( line, sep );
            QStringList list;
            for ( tokenizer::iterator it = tokens.begin(); it != tokens.end(); ++it )
                list << QString::fromStdString( *it );
            dlg.appendLine( list );
        }
    } while ( 0 );

    QApplication::changeOverrideCursor( Qt::ArrowCursor );
    dlg.show();
    double accelVoltage(0);
    double length(0);
    double tDelay(0);
    std::string model;

    if ( dlg.exec() ) {

        QApplication::changeOverrideCursor( Qt::WaitCursor );

        if ( dlg.hasScanLaw() ) {
            accelVoltage = dlg.acceleratorVoltage();
            length = dlg.length();
            tDelay = dlg.tDelay();
            model = dlg.dataInterpreterClsid().toStdString();
        }

        if ( dlg.dataType() != adtextfile::Dialog::data_spectrum ) {
            return 1;
            // TXTSpectrum txt;
            // if ( txt.load( filename, dlg ) && prepare_portfolio( txt, filename, portfolio ) ) {
            //     processedDataset_.reset( new adcontrols::ProcessedDataset );
            //     processedDataset_->xml( portfolio.xml() );
            //     return true;
            // }
        }
    }

    // -- end file type determination

    if ( vm.count("args") ) {

        for ( auto& fname: vm[ "args" ].as< std::vector< std::string > >() ) {

            boost::filesystem::path path( fname );

            ADDEBUG() << fname;
            if ( auto file = adcontrols::datafile::open( path.wstring(), false ) ) {

            }


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
