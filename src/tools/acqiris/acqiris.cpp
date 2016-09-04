/**************************************************************************
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com or info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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

#include "acqiris.hpp"
#include <boost/format.hpp>
#include <sstream>
#include <iostream>
#include <sys/stat.h>

acqiris::result_code
acqiris::waitForEndOfAcquisition( size_t timeout )
{
    ViStatus st = AcqrsD1_waitForEndOfAcquisition( inst_, ViInt32( timeout ) );
    switch( st ) {
    case VI_SUCCESS: return success;
    case ACQIRIS_ERROR_ACQ_TIMEOUT: return error_timeout;
    case ACQIRIS_ERROR_OVERLOAD: return error_overload; //  if a channel/trigger overload was detected.
    case ACQIRIS_ERROR_IO_READ: return error_io_read;   //  if a link error has been detected (e.g. PCI link lost).
    case ACQIRIS_ERROR_INSTRUMENT_STOPPED: return error_stopped; // if the acquisition was not started beforehand
    }
}
    
bool
acqiris::getInstrumentData()
{
    ViChar buf[256];
    ViStatus st = Acqrs_getInstrumentData(inst_, buf, &serial_number_, &bus_number_, &slot_number_);
    return st == VI_SUCCESS;
}

//static
std::string
acqiris::error_msg( int status, const char * ident )
{
    std::ostringstream o;
    ViChar errorMessage[ 512 ];
    
    if ( Acqrs_errorMessage( VI_NULL, status, errorMessage, sizeof( errorMessage ) ) == VI_SUCCESS ) {
        o << errorMessage << " 0x" << std::hex << status << " at " << ident;
    } else {
        o << "ERROR: code =0x" << std::hex << status << " at " << ident;
    }
    return o.str();
}

//static
bool
acqiris::checkError( ViSession instId, ViStatus st, const char * text, ViInt32 arg )
{
    if ( st == VI_SUCCESS )
        return false;
    std::array< ViChar, 1024 > msg;
    std::fill( msg.begin(), msg.end(), 0 );
    AcqrsD1_errorMessageEx( instId, st, msg.data(), msg.size() );
    std::cerr << boost::format( "%s (0x%x): %s" ) % text % int(st) % msg.data();
    if ( arg )
        std::cerr << " #" << arg;
    std::cerr << std::endl;
    return false;
}

bool
acqiris::initialize()
{
    if ( getenv("AcqirisDxDir") == 0 ) {
        std::cerr << L"AcqirisDxDir environment variable not set." << std::endl;
        return false;
    }

    struct stat st;
    if ( stat( "/dev/acqrsPCI", &st ) != 0 ) {
        std::cerr << L"/dev/acqrsPID does not exists" << std::endl;
        return false;
    }
    return true;
}

bool
acqiris::findDevice()
{
    ViStatus status;
    
    numInstruments_ = 0;
    
    if ( ( status = AcqrsD1_multiInstrAutoDefine( "cal=0", &numInstruments_ ) ) != VI_SUCCESS ) {
        std::cerr << error_msg( status, "Acqiris::findDevice()" ) << std::endl;
    } else {
        std::cout << boost::format( "find %1% acqiris devices." ) % numInstruments_ << std::endl;
    }

    if ( numInstruments_ == 0 ) {
        if ( auto p = getenv( "AcqirisOption" ) ) {
            if ( std::strcmp( p, "simulate" ) == 0 ) {
        
                if ( Acqrs_setSimulationOptions( "M2M" ) == VI_SUCCESS ) {
                    if ( Acqrs_InitWithOptions( const_cast<char*>("PCI::DC271")
                                                , VI_FALSE, VI_FALSE, const_cast<char *>("simulate=TRUE"), &inst_ ) == VI_SUCCESS ) {
                        numInstruments_ = 1;
                    }
                }
            }
        }
    }
    
    if ( numInstruments_ == 0 )
        return false;
    
    for ( int i = 0; i < numInstruments_; ++i ) {
        device_name_ = ( boost::format( "PCI::INSTR%1%" ) % i ).str();
        inst_ = (-1);
        status = Acqrs_init( const_cast< char *>(device_name_.c_str()), VI_FALSE, VI_FALSE, &inst_);
        if ( inst_ != ViSession(-1) && getInstrumentData() ) {
            std::cerr << "\tfound device on: " << device_name_ << std::endl;
            return true;
        } else {
            std::cerr << error_msg( status, "Acqiris::findDevice" ) << std::endl;
        }
    }
    return false;
}

bool
acqiris::averager_setup( int nDelay, int nSamples, int nAverage )
{
    ViStatus status;
    ViStatus * pStatus = &status;

    if ( pStatus == 0 )
        pStatus = &status;
	
    //status = AcqrsD1_configVertical( inst_, 1, 0.5, 0.2, 3, 2 );
    //                                     ch = 1, fs = 5.0V, offset = 0.0v, coupling = DC 50ohm, bw = 700MHz
    status = AcqrsD1_configVertical( inst_, 1, 1.0, 0.0, 3, 2 );
    if ( checkError( inst_, status, "configVertical", __LINE__ ) )
        return false;

    // External trig. input
    status = AcqrsD1_configVertical( inst_, -1, 5.0, 0.0, 3 /*DC 50ohm*/, 2 /* no bw */);
    if ( checkError( inst_, status, "configVertical (2)", __LINE__ ) )
        return false;

    status = AcqrsD1_configMemory( inst_, nSamples, 1 );
    if ( checkError( inst_, status, "configMemory", __LINE__ ) )
        ; //return false;

    status = AcqrsD1_configTrigClass( inst_, 0, 0x80000000, 0, 0, 0, 0 );
    if ( checkError( inst_, status, "AcqrsD1_configTrigClass", __LINE__  ) )
        return false;
	
    // ExtTrigSource(-1), DC coupling(0), positive(0), 1000mV (1400)
    status = AcqrsD1_configTrigSource( inst_, -1, 0, 0, 1000, 0 );
    //status = AcqrsD1_configTrigSource( inst_, -1, 0, 0, -25.0, 0 ); // negative  
    if ( checkError( inst_, status, "AcqrsD1_configTrigSource", __LINE__  ) )
        return false;
	
    status = AcqrsD1_configMode( inst_, 2, 0, 0 ); // 2 := averaging mode, 0 := normal data acq.
    if ( checkError( inst_, status, "AcqrsD1_configMode", __LINE__  ) )
        return false;
	
    status = AcqrsD1_configMultiInput( inst_, 1, 0 );
    if ( checkError( inst_, status, "AcqrsD1_configMultiInput", __LINE__  ) )
        return false;

    status = AcqrsD1_configChannelCombination( inst_, 2, 1 );
    if ( checkError( inst_, status, "AcqrsD1_configChannelCombination", __LINE__  ) )
        return false;

#if 0
    // config "IO A" -- it seems not working for input level
    status = AcqrsD1_configControlIO( inst_, 1, 1, 0, 0 );
    if ( checkError( inst_, status, "AcqrsD1_configControlIO(A)" ) )
         return false;
#endif
    // config "IO B" for Acquisition is active (21)
    status = AcqrsD1_configControlIO( inst_, 2, 21, 0, 0 );
    if ( checkError( inst_, status, "AcqrsD1_configControlIO(B)", __LINE__  ) )
        return false;

    // Configure the front panel trigger out (TR.)
    // The appropriate offset is 1,610 mV.
    status = AcqrsD1_configControlIO( inst_, 9, 1610 / 2, 0, 0 );
    //status = AcqrsD1_configControlIO( inst_, 9, 0, 0, 0 );
    if ( checkError( inst_, status, "AcqrsD1_configControlIO (2)", __LINE__  ) )
        return false;

    ViInt32 int32Arg = nSamples;    
    status = AcqrsD1_configAvgConfig( inst_, 0, "NbrSamples", &int32Arg );
    checkError( inst_, status, "AcqirisD1_configAvgConfig, NbrSamples", __LINE__  );

    int32Arg = nDelay;
    status = AcqrsD1_configAvgConfig( inst_, 0, "StartDelay", &int32Arg );
    checkError( inst_, status, "AcqrsD1_configAvgConfig StartDelay ", __LINE__ );

    // "P2Control" set to average(out) --> disable for debug
    int32Arg = 0;
    status = AcqrsD1_configAvgConfig( inst_, 0, "P1Control", &int32Arg );
    if ( checkError( inst_, status, "AcqrsD1_configAvgConfig (P1Control)", __LINE__  ) )
        return false;

    // "P2Control" to disable
	int32Arg = 0;
    status = AcqrsD1_configAvgConfig( inst_, 0, "P2Control", &int32Arg );
    if ( checkError( inst_, status, "AcqrsD1_configAvgConfig (P2Control)", __LINE__  ) )
        return false;

    int32Arg = 0;
    status = AcqrsD1_configAvgConfig( inst_, 0, "DitherRange", &int32Arg );
    if ( checkError( inst_, status, "AcqrsD1_configAvgConfig (DitherRange)", __LINE__  ) )
        return false;
	
    int32Arg = 1;
    status = AcqrsD1_configAvgConfig( inst_, 0, "NbrSegments", &int32Arg );
    if ( checkError( inst_, status, "AcqrsD1_configAvgConfig (NbrSegments)", __LINE__  ) )
        return false;

    int32Arg = nAverage;
    status = AcqrsD1_configAvgConfig( inst_, 0, "NbrWaveforms", &int32Arg );
    checkError( inst_, status, "AcqrsD1_configAvgConfig NbrWaveforms ", __LINE__ );

    int32Arg = 0;
    *pStatus = AcqrsD1_configAvgConfig( inst_, 0, "StopDelay", &int32Arg );
    if ( checkError( inst_, status, "AcqrsD1_configAvgConfig (StopDelay)", __LINE__  ) )
        return false;

    int32Arg = 0;
    status = AcqrsD1_configAvgConfig( inst_, 0, "TrigAlways", &int32Arg );
    if ( checkError( inst_, status, "AcqrsD1_configAvgConfig (TrigAlways)", __LINE__  ) )
        return false;

    int32Arg = 0;
    status = AcqrsD1_configAvgConfig( inst_, 0, "TrigResync", &int32Arg );
    if ( checkError( inst_, status, "AcqrsD1_configAvgConfig (TrigResync)", __LINE__  ) )
        return false;

    int32Arg = 0;
    status = AcqrsD1_configAvgConfig( inst_, 0, "ThresholdEnable", &int32Arg );
    if ( checkError( inst_, status, "AcqrsD1_configAvgConfig (ThresholdEnable)", __LINE__  ) )
        return false;

    int32Arg = 0;
    status = AcqrsD1_configAvgConfig( inst_, 0, "TriggerTimeout", &int32Arg );
    if ( checkError( inst_, status, "AcqrsD1_configAvgConfig (TriggerTimeut)", __LINE__  ) )
        return false;

    status = AcqrsD1_configHorizontal( inst_, 0.5e-9, 0 );
    if ( checkError( inst_, status, "AcqrsD1_configHorizontal", __LINE__  ) )
        return false;

    //-----------------------
    do {
        int32Arg = 1;
        status = AcqrsD1_configAvgConfig( inst_, 0, "TimestampClock", &int32Arg);
        if ( checkError( inst_, status, "AcqrsD1_configAvgConfig (TimestampClock)", __LINE__  ) )
            return false;
    } while (0);
	
    do {
        int32Arg = 1;  // Always on first trigger
        status = AcqrsD1_configAvgConfig( inst_, 0, "MarkerLatchMode", &int32Arg); 
        if ( checkError( inst_, status, "AcqrsD1_configAvgConfig (MarkerLatchMode)", __LINE__  ) )
            return false;
    } while (0);

    do {
        int32Arg = 1; // invert data for mass spectrum;
        status = AcqrsD1_configAvgConfig( inst_, 0, "InvertData", &int32Arg); 
        if ( checkError( inst_, status, "AcqrsD1_configAvgConfig (InvertData)", __LINE__  ) )
            return false;
    } while (0);
	
    return true;

}

bool
acqiris::digitizer_setup( double delay, double width )
{
    ViStatus status;
    ViStatus * pStatus = &status;

    constexpr ViInt32 nbrSegments = 1;

    nbrSamples_ = width / 0.5e-9 + 1;

    // ViSession instrID(0);
    // status = Acqrs_InitWithOptions("PCI::INSTR0", VI_FALSE, VI_FALSE, "", &instrID);
    // checkError( inst_, status, "Acqrs_InitWithOptions", __LINE__  );

    status = AcqrsD1_configMultiInput( inst_, 1, 0 );
    checkError( inst_, status, "AcqrsD1_configMultiInput", __LINE__  );

    status = AcqrsD1_configChannelCombination( inst_, 2, 1 );
    checkError( inst_, status, "AcqrsD1_configChannelCombination", __LINE__  );
    
    //status = AcqrsD1_configHorizontal( inst_, 0.5e-9, 0 );
    status = AcqrsD1_configHorizontal( inst_, 1.0e-8, 0 );
    checkError( inst_, status, "AcqrsD1_configHorizontal", __LINE__  );

    status = AcqrsD1_configMemory(inst_, nbrSamples_, nbrSegments);
    checkError( inst_, status, "configMemory", __LINE__ );
    
    status = AcqrsD1_configMode( inst_, 0, 0, 0 ); // 2 := averaging mode, 0 := normal data acq.
    checkError( inst_, status, "AcqrsD1_configMode", __LINE__  );

    status = AcqrsD1_configVertical( inst_, 1, 1.0, 0.0, 3, 2 );
    checkError( inst_, status, "configVertical", __LINE__ );

    // External trig. input
    status = AcqrsD1_configVertical( inst_, -1, 5.0, 0.0, 3 /*DC 50ohm*/, 2 /* no bw */);
    checkError( inst_, status, "configVertical (2)", __LINE__ );

    status = AcqrsD1_configTrigClass( inst_, 0, 0x80000000, 0, 0, 0, 0 );
    checkError( inst_, status, "AcqrsD1_configTrigClass", __LINE__  );
	
    status = AcqrsD1_configTrigSource( inst_, -1, 0, 0, 1000, 0 );
    checkError( inst_, status, "AcqrsD1_configTrigSource", __LINE__  );
	
    // config "IO B" for Acquisition is active (21)
    status = AcqrsD1_configControlIO( inst_, 2, 21, 0, 0 );
    checkError( inst_, status, "AcqrsD1_configControlIO(B)", __LINE__  );

    status = AcqrsD1_configControlIO( inst_, 9, 1610 / 2, 0, 0 );
    checkError( inst_, status, "AcqrsD1_configControlIO (2)", __LINE__  );

    // "P2Control" set to average(out) --> disable for debug
    ViInt32 int32Arg = 0;
    status = AcqrsD1_configAvgConfig( inst_, 0, "P1Control", &int32Arg );
    checkError( inst_, status, "AcqrsD1_configAvgConfig (P1Control)", __LINE__  );

    // "P2Control" to disable
	int32Arg = 0;
    status = AcqrsD1_configAvgConfig( inst_, 0, "P2Control", &int32Arg );
    if ( checkError( inst_, status, "AcqrsD1_configAvgConfig (P2Control)", __LINE__  ) )
        return false;

    int32Arg = 0;
    status = AcqrsD1_configAvgConfig( inst_, 0, "DitherRange", &int32Arg );
    if ( checkError( inst_, status, "AcqrsD1_configAvgConfig (DitherRange)", __LINE__  ) )
        return false;
	
    int32Arg = 1;
    status = AcqrsD1_configAvgConfig( inst_, 0, "NbrSegments", &int32Arg );
    if ( checkError( inst_, status, "AcqrsD1_configAvgConfig (NbrSegments)", __LINE__  ) )
        return false;

    int32Arg = 0;
    *pStatus = AcqrsD1_configAvgConfig( inst_, 0, "StopDelay", &int32Arg );
    if ( checkError( inst_, status, "AcqrsD1_configAvgConfig (StopDelay)", __LINE__  ) )
        return false;

    int32Arg = 0;
    status = AcqrsD1_configAvgConfig( inst_, 0, "TrigAlways", &int32Arg );
    if ( checkError( inst_, status, "AcqrsD1_configAvgConfig (TrigAlways)", __LINE__  ) )
        return false;

    int32Arg = 0;
    status = AcqrsD1_configAvgConfig( inst_, 0, "TrigResync", &int32Arg );
    if ( checkError( inst_, status, "AcqrsD1_configAvgConfig (TrigResync)", __LINE__  ) )
        return false;

    int32Arg = 0;
    status = AcqrsD1_configAvgConfig( inst_, 0, "ThresholdEnable", &int32Arg );
    if ( checkError( inst_, status, "AcqrsD1_configAvgConfig (ThresholdEnable)", __LINE__  ) )
        return false;

    int32Arg = 0;
    status = AcqrsD1_configAvgConfig( inst_, 0, "TriggerTimeout", &int32Arg );
    if ( checkError( inst_, status, "AcqrsD1_configAvgConfig (TriggerTimeut)", __LINE__  ) )
        return false;

    //-----------------------
    do {
        int32Arg = 1;
        status = AcqrsD1_configAvgConfig( inst_, 0, "TimestampClock", &int32Arg);
        if ( checkError( inst_, status, "AcqrsD1_configAvgConfig (TimestampClock)", __LINE__  ) )
            return false;
    } while (0);
	
    do {
        int32Arg = 1;  // Always on first trigger
        status = AcqrsD1_configAvgConfig( inst_, 0, "MarkerLatchMode", &int32Arg); 
        if ( checkError( inst_, status, "AcqrsD1_configAvgConfig (MarkerLatchMode)", __LINE__  ) )
            return false;
    } while (0);

    do {
        int32Arg = 1; // invert data for mass spectrum;
        status = AcqrsD1_configAvgConfig( inst_, 0, "InvertData", &int32Arg); 
        if ( checkError( inst_, status, "AcqrsD1_configAvgConfig (InvertData)", __LINE__  ) )
            return false;
    } while (0);
	
    return true;

}

bool
acqiris::acquire()
{
    return AcqrsD1_acquire( inst_ ) == VI_SUCCESS;
}

bool
acqiris::stop()
{
    return AcqrsD1_stopAcquisition( inst_ ) == VI_SUCCESS;
}
