/**************************************************************************
** Copyright (C) 2010-2021 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2021 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include <libdgpio/pio.hpp>
#include <adportable/float.hpp>
#include <adportable/debug.hpp>
#include <aqmd3/ppio.hpp>
#include <aqmd3/aqmd3.hpp>
#include <aqmd3/findresource.hpp>
#include <aqmd3/configfile.hpp>
#include <aqmd3/digitizer.hpp>
#include <aqmd3/configfile.hpp>
#include <aqmd3controls/identify.hpp>
#include <aqmd3controls/method.hpp>
#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <cstdlib>
#include <cstring>
#include <chrono>
#include <ostream>
#include <iostream>
#include <memory>
#include <ratio>
#include <stdexcept>
#include <thread>
#include <vector>

namespace po = boost::program_options;

int __verbose__ = 5;

class execStatistics {
    execStatistics( const execStatistics& ) = delete; // non copyable

public:
    double last_;
    size_t deadCount_;
    size_t dataCount_;
    double rate_;
    std::chrono::system_clock::time_point tp_;
    std::vector< std::pair< double, double > > exceededTimings_;

    execStatistics() : last_( 0 )
                     , deadCount_( 0 )
                     , dataCount_( 0 )
                     , rate_( 0 )
                     , tp_( std::chrono::system_clock::now() ) {
    }

    inline double difference_from_last( double seconds ) {
        return seconds - last_;
    }

    static execStatistics& instance() {

        // lasy initialized singleton

        static execStatistics __instance;
        return __instance;

    }
};

std::ostream& operator << ( std::ostream& out, const execStatistics& t )
{
    uint64_t ns = std::chrono::duration_cast< std::chrono::nanoseconds >( std::chrono::system_clock::now() - t.tp_ ).count();
    double s = double(ns) * 1.0e-9;

    out << "Took " << s << " seconds; " << double(t.dataCount_) / s << "Hz" << std::endl;
    out << t.deadCount_ << " DEAD data received / " << t.dataCount_ << " waveforms read" << std::endl;
    out << t.exceededTimings_.size() << " waveforms has the trigger timing exceeded from external trig rate of " << t.rate_ << std::endl;

    for ( auto& pair: t.exceededTimings_ )
        out << "waveform at " << pair.first << " interval: " << pair.second << std::endl;
    return out;
}

#if defined __linux

#include <csignal>

static void sigint(int num )
{
    std::cout << "************* got signal " << num << " ***************" << std::endl;

    std::cout << execStatistics::instance() << std::endl;

    std::cout << "Merci, Salut" << std::endl;
    exit( num );
}

#endif

int pkd_main( std::shared_ptr< aqmd3::AqMD3 >, const aqmd3controls::method&, size_t replicates );

const std::vector< std::string > ModelSA = { "SA220P", "SAS220E", "SA217P", "SA217E" };

int
main( int argc, char * argv [] )
{
    bool success( false ), simulated( false );
    bool TSR_enabled( false );

    po::variables_map vm;
    po::options_description description( "diag_aqmd3" );
    {
        description.add_options()
            ( "help,h",    "Display this help message" )
            ( "tsr,t",     "TSR enable" )
            ( "pkd",       "PKD enable" )
            ( "avg",       "AVG enable" )
            ( "pkd-amplitude",    "PKD amplitude accumulation enabled" )
            ( "records,r",  po::value<int>()->default_value( 1 ),       "Number of records" )
            ( "average,a",  po::value<int>()->default_value( 0 ),       "Number of average" )
            ( "invert-signal", po::value<bool>()->default_value( true ),"Invert signal {true/false}" )
            ( "rising-delta", po::value<int>()->default_value( 20 ),    "PKD Rising delta" )
            ( "falling-delta", po::value<int>()->default_value( 20 ),   "PKD Falling delta" )
            ( "mode,m",     po::value<int>()->default_value( 0 ),       "=0 Normal(digitizer); =2 Averager" )
            ( "delay,d",    po::value<double>()->default_value( 0.0 ),  "Delay (us)" )
            ( "width,w",    po::value<double>()->default_value( 10.0 ), "Waveform width (us)" )
            ( "replicates", po::value<int>()->default_value( 1 ),       "Number of triggers to acquire waveforms" )
            ( "rate",       po::value<double>()->default_value( 1.0 ),  "Expected trigger interval in millisecond (trigger drop/nodrop validation)" )
            ( "pxi",        po::value< std::string >(),                 "Resource name such as 'PXI8::0::0::INSTR'")
            ( "find",       "Find resource" )
            ( "config",       "show config" )
            ( "force-config", "Force create aqmd3.ini file (for debugging)" )
            ( "reset-config", "clear config" )
            ( "verbose",    po::value<int>()->default_value( 5 ),       "Verbose 0..9" )
            ;
        po::store( po::command_line_parser( argc, argv ).options( description ).run(), vm );
        po::notify(vm);
    }

    if ( vm.count( "help" ) ) {
        std::cout << description;
        return 0;
    }

    if ( vm.count( "tsr" ) ) {
        TSR_enabled = true;
    }

    if ( vm.count( "verbose" ) )
        __verbose__ = vm[ "verbose" ].as< int >();

    aqmd3controls::method method;
    method.setChannels( 0x01 );
    method.setMode( static_cast<aqmd3controls::method::DigiMode>( vm[ "mode" ].as<int>() ) );
    method.device_method().front_end_range = 1.0;  // V
    method.device_method().front_end_offset = 0.0; // V
    method.device_method().ext_trigger_level = 1.0;
    method.device_method().samp_rate = vm.count( "pkd" ) ? 1.6e9 : 3.2e9;
    method.device_method().invert_signal = vm[ "invert-signal" ].as< bool >();

    // MultiRecords or single record
    method.device_method().nbr_records = vm[ "records" ].as<int>();
    method.device_method().nbr_of_averages = vm[ "average" ].as<int>();    // 0 for digitizer

    if ( vm.count( "pkd" ) && method.device_method().nbr_of_averages == 0 )
        method.device_method().nbr_of_averages = 2;

    if ( ( method.device_method().nbr_of_averages > 8 ) && ( method.device_method().nbr_of_averages % 8 ) ) {
        std::cout << "Number of waveforms to be averaged must be fold of 8 or less than 8" << std::endl;
        method.device_method().nbr_of_averages &= ~07;
    }

    // delay (seconds)
    method.device_method().delay_to_first_sample_
        = method.device_method().digitizer_delay_to_first_sample
        = vm[ "delay" ].as<double>() / std::micro::den;

    // data length
    uint32_t width = uint32_t( ( vm[ "width" ].as<double>() / std::micro::den ) * method.device_method().samp_rate + 0.5 );
    method.device_method().digitizer_nbr_of_s_to_acquire = method.device_method().nbr_of_s_to_acquire_ = width;

    // TSR
    method.device_method().TSR_enabled = vm.count( "tsr" );

    // PKD
    method.device_method().pkd_enabled = vm.count( "pkd" );
    method.device_method().pkd_rising_delta = vm[ "rising-delta" ].as< int >();
    method.device_method().pkd_falling_delta = vm[ "falling-delta" ].as< int >();
    method.device_method().pkd_amplitude_accumulation_enabled = vm.count( "pkd-amplitude" );

    if ( method.device_method().TSR_enabled && method.device_method().pkd_enabled ) {
        std::cerr << "TSR & PKD can't be set together\n";
        return 0;
    }

    const size_t replicates = vm[ "replicates" ].as<int>();

#if defined __linux
    signal( SIGINT, &sigint );
    signal( SIGQUIT, &sigint );
    signal( SIGABRT, &sigint );
    signal( SIGHUP, &sigint );
    signal( SIGKILL, &sigint );
#endif

    if ( vm.count( "config" ) ) {
        aqmd3::configFile file;
        std::cout << "\n"
                  << "aqmd3::configFile: " << file.inifile_ << std::endl;
        if ( auto res = file.loadResource() ) {
            std::cout << *res << std::endl;
        } else {
            std::cout << "file '" << file.inifile_ << "' does not exists." << std::endl;
        }
        return 0;
    }

    if ( vm.count( "force-config" ) ) {
        aqmd3::configFile file;
        if ( auto res = file.loadResource() ) {
            // do nothing -- it should be saved in findResource() operator
        } else {
            file.saveResource( "PXI40::0::0::INSTR" );
        }
        return 0;
    }

    if ( vm.count( "reset-config" ) ) {
        aqmd3::configFile().remove_all();
        return 0;
    }

    execStatistics::instance().rate_ = vm[ "rate" ].as<double>() * 1.0e-3 * 1.2; // milliseconds -> seconds + 20%
    std::string pxi;
    if ( vm.count( "pxi" ) )
        pxi = vm[ "pxi" ].as< std::string >();

    ppio pp;
    dgpio::pio dgpio;

    if ( ! dgpio.open() )
        std::cerr << "dgpio open failed -- ignored." << std::endl;

    if ( auto md3 = std::make_shared< aqmd3::AqMD3 >() ) {

        if ( auto p = getenv( "AcqirisOption" ) ) {
            if ( p && std::strcmp( p, "simulate" ) == 0 ) {
                const char * strInitOptions = "Simulate=true, DriverSetup= Model=U5303A";
                simulated = true;
                success = ( md3->initWithOptions( "PXI40::0::0::INSTR", VI_FALSE, VI_TRUE, strInitOptions ) == VI_SUCCESS );
            }
        }
        if ( vm.count( "find" ) ) {
            if ( auto res = aqmd3::findResource()( md3 ) ) {
                ADDEBUG() << "FOUND: " << *res;
            } else {
                ADDEBUG() << "device not found";
            }
            return 0;
        }

        if ( ! simulated ) {
            if ( vm.count( "pxi" ) ) {
                success = aqmd3::findResource()( md3, vm[ "pxi" ].as< std::string >() );
            } else {
                if ( auto res = aqmd3::findResource()( md3 ) )
                    success = true;
            }
        }

        if ( success ) {

            method.device_method().TSR_enabled = TSR_enabled;

            auto ident = std::make_shared< aqmd3controls::identify >();
            md3->Identify( ident );

            std::cout << "Identifier:       " << ident->Identifier() << std::endl
                      << "Revision:         " << ident->Revision() << std::endl
                      << "Vendor:           " << ident->Vendor() << std::endl
                      << "Description:      " << ident->Description() << std::endl
                      << "InstrumentModel:  " << ident->InstrumentModel() << std::endl
                      << "Options:          " << ident->Options() << std::endl
                      << "FirmwareRevision: " << ident->FirmwareRevision() << std::endl
                      << "SerialNumber:     " << ident->SerialNumber() << std::endl
                      << "IOVersion:        " << ident->IOVersion() << std::endl
                      << "NbrADCBits:       " << ident->NbrADCBits() << std::endl;

            if ( vm.count( "find" ) ) {
                return 0;
            }

            ADDEBUG() << "## Constract device method ##";
            if ( std::find( ModelSA.begin(), ModelSA.end(), ident->InstrumentModel() ) != ModelSA.end() ) {
                if ( !( adportable::compare<double>::is_equal(method.device_method().front_end_range, 0.5) ||
                        adportable::compare<double>::is_equal(method.device_method().front_end_range, 2.5) ) )
                    method.device_method().front_end_range = 0.5;
                method.device_method().samp_rate = 2.0e9; // PKD only works with 2GSPS
                uint32_t width = uint32_t( ( vm[ "width" ].as<double>() / std::micro::den ) * method.device_method().samp_rate + 0.5 );
                method.device_method().digitizer_nbr_of_s_to_acquire = method.device_method().nbr_of_s_to_acquire_ = width;
            } else {
                ADDEBUG() << "No SAxxx model found: " << ident->InstrumentModel();
                for ( auto& model: ModelSA )
                    ADDEBUG() << model;
            }

            using aqmd3::AqMD3;
            using aqmd3::attribute;

            int32_t count(0);
            // md3->clog( aqmd3::attribute< aqmd3::control_io_count >::get(*md3, "ControlIO", count ), __FILE__,__LINE__ );
            if ( auto ccount = aqmd3::attribute< aqmd3::control_io_count >::value( *md3, "ControlIO" ) ) {
                count = ccount.get();
                std::cout << "Control IO Count: " << count << std::endl;
            }

            if ( __verbose__ > 5 ) {
                ViStatus rcode;
                ViChar name [ 256 ];
                for ( int i = 1; i <= count; ++i ) {
                    if ( AqMD3_GetControlIOName( md3->session(), i, 256, name ) == 0 ) {
                        std::cout << "\tControlIO Name: " << name << std::endl;
                        if ( auto res = aqmd3::attribute< aqmd3::control_io_signal >::value( *md3, name ) )
                            std::cout << "\tControlIO name: " << name << "\t" << res.get() << std::endl;
                        if ( auto sig = aqmd3::attribute< aqmd3::control_io_available_signals >::value( *md3, name ) )
                            std::cout << "\tAvilable Signals: " << sig.get() << std::endl;
                    }
                }
            }

            std::cout << "## External1" << std::endl;
            md3->clog( attribute< aqmd3::active_trigger_source >::set( *md3, std::string( "External1" ) ), __FILE__,__LINE__ );
            md3->clog( attribute< aqmd3::trigger_level >::set( *md3, "External1", method.device_method().ext_trigger_level ), __FILE__,__LINE__ );
            ViStatus rcode;
            if ( auto value = attribute< aqmd3::trigger_level >::value( *md3, rcode, "External1" ) )
                std::cout << "\ttrigger level: " << value.get() << std::endl;
            else
                md3->clog( rcode, __FILE__, __LINE__ );

            md3->clog( attribute< aqmd3::trigger_slope >::set( *md3, "External1", AQMD3_VAL_TRIGGER_SLOPE_POSITIVE ), __FILE__,__LINE__ );
            if ( auto value = attribute< aqmd3::trigger_slope >::value( *md3, rcode, "External1" ) )
                std::cout << "\ttrigger slope: " << value.get() << std::endl;
            else
                md3->clog( rcode, __FILE__, __LINE__ );

            //-------------- dispatch pkd main -------------
            if ( vm.count( "pkd" ) || vm.count( "pkd-amplitude") ) // && ident->Options().find( "PKD" ) != std::string::npos )
                return pkd_main( md3, method, replicates );

            std::cout << "\t---------- DIGITIZER MODE ----------------" << std::endl;

            if ( ident->Options().find( "INT" ) != std::string::npos ) { // Interleave ON
                std::cout << "\t-------------- INTERLEAVE ON ----------------------" << std::endl;
                md3->ConfigureTimeInterleavedChannelList( "Channel1", "Channel2" );
            }

            std::cerr << "Range:              " << method.device_method().front_end_range << '\n';
            std::cerr << "Offset:             " << method.device_method().front_end_offset << '\n';

            md3->clog( AqMD3_ConfigureChannel( md3->session(), "Channel1"
                                               , method.device_method().front_end_range
                                               , method.device_method().front_end_offset
                                               , AQMD3_VAL_VERTICAL_COUPLING_DC
                                               , VI_TRUE ), __FILE__,__LINE__ );


            const std::vector< std::string > ModelSA = { "SA220P", "SAS220E", "SA217P", "SA217E" };

            double max_rate(0);
            if ( std::find( ModelSA.begin(), ModelSA.end(), ident->InstrumentModel() ) != ModelSA.end() ) {
                max_rate = 2.0e9;
            } else {
                // U5303A
                if ( ident->Options().find( "SR1" ) != std::string::npos ) {
                    max_rate = ( ident->Options().find( "INT" ) != std::string::npos ) ? 2.0e9 : 1.0e9;
                } else if ( ident->Options().find( "SR2" ) != std::string::npos ) {
                    max_rate = ( ident->Options().find( "INT" ) != std::string::npos ) ? 3.2e9 : 1.6e9;
                }
            }

            using aqmd3::attribute;
            md3->clog( attribute< aqmd3::sample_rate >::set( *md3, max_rate ), __FILE__,__LINE__ );
            // md3->setSampleRate( max_rate );

            md3->clog( attribute< aqmd3::sample_rate >::get( *md3, method.device_method().samp_rate ), __FILE__,__LINE__ );
            //method.device_method().samp_rate = md3->SampleRate();

            std::cout << "SampleRate: " << method.device_method().samp_rate << std::endl;

            attribute< aqmd3::record_size >::set( *md3, method.device_method().nbr_of_s_to_acquire_ );
            // md3->setAcquisitionRecordSize( method.device_method().digitizer_nbr_of_s_to_acquire );  // 100us @ 3.2GS/s

            attribute< aqmd3::trigger_delay >::set( *md3, method.device_method().delay_to_first_sample_ );
            // md3->setTriggerDelay( method.device_method().digitizer_delay_to_first_sample );

            attribute< aqmd3::num_records_to_acquire >::set( *md3, int64_t( method.device_method().nbr_records ) );
            // md3->setAcquisitionNumRecordsToAcquire( method.device_method().nbr_records );

            attribute< aqmd3::acquisition_mode >::set( *md3, AQMD3_VAL_ACQUISITION_MODE_NORMAL );
            // md3->setAcquisitionMode( AQMD3_VAL_ACQUISITION_MODE_NORMAL ); // Digitizer mode

            attribute< aqmd3::tsr_enabled >::set( *md3, method.device_method().TSR_enabled );
            // md3->setTSREnabled( method.device_method().TSR_enabled );

            md3->SelfCalibrate();

            std::vector< std::shared_ptr< aqmd3controls::waveform > > vec;

            std::cout << "Replicates: " << replicates << std::endl;

            execStatistics::instance().tp_ = std::chrono::system_clock::now();

            bool tsrEnabled;
            if ( md3->clog( attribute< aqmd3::tsr_enabled >::get( *md3, tsrEnabled ), __FILE__,__LINE__ ) && tsrEnabled ) {
                std::cout << "\t------------------ TSR enabled ------------------" << std::endl;
                // if ( md3->TSREnabled() ) {

                md3->AcquisitionInitiate();

                while ( replicates > execStatistics::instance().dataCount_ ) {

                    do {
                        boost::optional< aqmd3::tsr_memory_overflow_occurred::value_type > p;
                        if ((p = attribute< aqmd3::tsr_memory_overflow_occurred >::value( *md3 ) && p.get() )) {
                            std::cout << "***** Memory Overflow" << std::endl;
                            (void)p;
                            break;
                        }
                    } while ( 0 );
                    do {
                        boost::optional< aqmd3::tsr_is_acquisition_complete::value_type > p;
                        while (( p = attribute< aqmd3::tsr_is_acquisition_complete >::value( *md3 ) && !p.get() ))
                            std::this_thread::sleep_for( std::chrono::microseconds( 100 ) ); // assume 1ms trig. interval
                    } while ( 0 );

                    aqmd3::digitizer::readData( *md3, method, vec );
                    md3->TSRContinue();

                    for ( auto& waveform: vec ) {

                        // report if trigger receive interval exceeded
                        double seconds = waveform->xmeta().initialXTimeSeconds;

                        if ( std::abs( execStatistics::instance().last_ ) >= std::numeric_limits<double>::epsilon() ) {

                            double interval = execStatistics::instance().difference_from_last( seconds );
                            if ( interval > execStatistics::instance().rate_ )
                                execStatistics::instance().exceededTimings_.push_back( std::make_pair( seconds, interval ) );

                        }
                        execStatistics::instance().last_ = seconds;
                    }

                    execStatistics::instance().dataCount_ += vec.size();

                    vec.clear();  // throw waveforms away.
                }

            } else {

                double prev_ts(0);

                for ( int i = 0; i < replicates; ++i ) {

                    pp << uint8_t( 0x01 );

                    md3->AcquisitionInitiate();
                    md3->AcquisitionWaitForAcquisitionComplete( 3000 );

                    pp << uint8_t( 0x02 );

                    aqmd3::digitizer::readData( *md3, method, vec );
                    auto ts = vec.at(0)->xmeta().initialXTimeSeconds;

                    int protocolIndex = dgpio.protocol_number(); // <- hard wired protocol id
                    execStatistics::instance().dataCount_ += vec.size();

                    if ( __verbose__ >= 5 ) {
                        auto wform = vec.at(0);
                        std::cout << "aqmd3::digitizer::readData read " << vec.size() << " waveform(s), proto#"
                                  << protocolIndex
                                  << "\t(" << i << "/" << replicates << ")"
                                  << "\t" << (ts - prev_ts)*1e6
                                  << "\t" << execStatistics::instance().dataCount_
                                  << "\tsize=" << wform->size()
                                  << std::endl;
                    }
                    prev_ts = ts;

                    vec.clear();
                }
            }

            std::cout << execStatistics::instance();

        }
    }

    return 0;
}
