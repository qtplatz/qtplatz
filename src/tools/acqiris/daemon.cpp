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

#include "daemon.hpp"
#include "tcp_server.hpp"
#include "task.hpp"
#include "log.hpp"
#include <acqrscontrols/acqiris_method.hpp>
#include <acqrscontrols/acqiris_protocol.hpp>
#include <acqrscontrols/acqiris_waveform.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <fstream>
#include <sys/wait.h>

int daemon::__debug_mode__;
int daemon::__pidParent__;
int daemon::__pidChild__;

class daemon *
daemon::instance()
{
    static daemon __instance;
    return &__instance;
}

int
daemon::main( int argc, char * argv[] )
{
    namespace po = boost::program_options;

    po::variables_map vm;
        
    po::options_description desc("options");
    desc.add_options()
        ( "help",    "print help message" )
        ( "version", "print version number" )
        ( "recv",      po::value< std::string >()->default_value( "0.0.0.0" ), "For IPv4 0.0.0.0, IPv6, try 0::0" )
        ( "port",    po::value< std::string >()->default_value( "8010" ), "aqdrv4 port numer" )
        ( "debug",   "debug" )
        ;
    po::store( po::command_line_parser( argc, argv ).options( desc ).run(), vm );
    po::notify( vm );

    if ( vm.count( "help" ) ) {
        std::cout << desc;
        return 0;
    }
    if ( vm.count( "version" ) ) {
        std::cout << VERSION << std::endl;
        return 0;
    }
    __debug_mode__ = vm.count( "debug" );

#if ! defined WIN32
    if ( !vm.count( "debug" ) ) {
        int fd = open( PID_NAME, O_RDWR|O_CREAT, 0644 );
        if ( fd < 0 ) {
            std::cerr << "Can't open " PID_NAME << std::endl;                
            exit(1);
        }
        int lock = lockf( fd, F_TLOCK, 0 );
        if ( lock < 0 ) {
            std::cerr << "Process " << argv[0] << " already running" << std::endl;
        }
        std::ostringstream o;
        o << getpid() << std::endl;
        ssize_t res = write( fd, o.str().c_str(), o.str().size() );
        (void)res;
    }
#endif        

    daemon::instance()->exec( vm["recv"].as< std::string >(), vm["port"].as< std::string >() );

    return 0;
}

daemon::daemon()
{
    auto m = std::make_shared< acqrscontrols::aqdrv4::acqiris_method >();
    auto trig = m->mutable_trig();
    auto hor = m->mutable_hor();
    auto ch1 = m->mutable_ch1();
    auto ext = m->mutable_ext();
    set_acqiris_method( m );
}

daemon::~daemon()
{
}

void
daemon::exec( const std::string& addr, const std::string& port )
{
    acqiris::server::tcp_server s( addr, port );
            
    task::instance()->connect_acqiris_method_adapted(
        [&]( std::shared_ptr< acqrscontrols::aqdrv4::acqiris_method > m ) {
            if ( auto data = acqrscontrols::aqdrv4::protocol_serializer::serialize( *m ) )
                s.post( data );
        });
            
    task::instance()->connect_replyTemperature( [&]( int temp ){
            auto data = std::make_shared< acqrscontrols::aqdrv4::acqiris_protocol >();
            data->preamble().clsid = acqrscontrols::aqdrv4::clsid_temperature;
            *data << int32_t( temp );
            s.post( data );
        });
            
    task::instance()->connect_push( [&]( std::shared_ptr< acqrscontrols::aqdrv4::waveform > p ){
            s.post( p );
        });

    if ( task::instance()->digitizer_initialize() && task::instance()->initialize() ) {
                
        task::instance()->prepare_for_run( daemon::instance()->acqiris_method(), acqrscontrols::aqdrv4::allMethod );
        s.run();

        task::instance()->finalize();
    }
}

// tcp client -> server (this)
void
daemon::prepare_for_run( std::shared_ptr< acqrscontrols::aqdrv4::acqiris_method > p, acqrscontrols::aqdrv4::SubMethodType t )
{
    task::instance()->prepare_for_run( p, t );
}

// tcp client -> server (this)
void
daemon::eventOut( uint32_t e )
{
}

