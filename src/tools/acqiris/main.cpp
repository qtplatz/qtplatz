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

#include "mainwindow.hpp"
#include "acqiris_method.hpp"
#include <QApplication>
#if defined USING_PROTOBUF
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/util/json_util.h>
#endif
#include <boost/serialization/nvp.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <fstream>

namespace po = boost::program_options;

int
main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    po::variables_map vm;
    po::options_description description( "acqiris" );
    {
        description.add_options()
            ( "help,h",    "Display this help message" )
            ( "delay",    po::value<double>()->default_value(  0.0 ), "Delay (us)" )
            ( "width",    po::value<double>()->default_value( 10.0 ), "Waveform width (us)" )
            ( "save",     po::value< std::string >(), "save method to file" )
            ( "load",     po::value< std::string >(), "load method from file" )
            ;
        po::store( po::command_line_parser( argc, argv ).options( description ).run(), vm );
        po::notify(vm);
    }
    if ( vm.count( "help" ) ) {
        std::cout << description;
        return 0;
    }

    aqdrv4::acqiris_method m;
    auto trig = m.mutable_trig();
    auto hor = m.mutable_hor();
    auto ch1 = m.mutable_ch1();

    if ( vm.count( "save" ) ) {
        auto file = vm[ "save" ].as< std::string >();
        std::wofstream of( file );
        boost::archive::xml_woarchive ar( of );
        ar & boost::serialization::make_nvp("aqdrv4", m );
    }
    
    MainWindow w;
    w.resize( 600, 400 );
    w.onInitialUpdate();
    w.show();

    return a.exec();
}
