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

#include "digitizer.hpp"
#include <acqrscontrols/acqiris_method.hpp>
#include <adportable/debug.hpp>
#include <boost/format.hpp>
#include <sys/stat.h>
#include <array>
#include <chrono>
#include <iostream>
#include <ratio>
#include <sstream>
#include <thread>

static bool __isSimulated__ = false;

digitizer::result_code
digitizer::waitForEndOfAcquisition( size_t timeout )
{
    ViStatus st = AcqrsD1_waitForEndOfAcquisition( inst_, ViInt32( timeout ) );

    if ( __isSimulated__ ) {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for( 500ms );
    }

    switch( st ) {
    case VI_SUCCESS: return success;
    case ACQIRIS_ERROR_ACQ_TIMEOUT: return error_timeout;
    case ACQIRIS_ERROR_OVERLOAD: return error_overload; //  if a channel/trigger overload was detected.
    case ACQIRIS_ERROR_IO_READ: return error_io_read;   //  if a link error has been detected (e.g. PCI link lost).
    case ACQIRIS_ERROR_INSTRUMENT_STOPPED: return error_stopped; // if the acquisition was not started beforehand
    }
    return success;
}
    
bool
digitizer::getInstrumentData()
{
    ViChar buf[256];
    ViStatus st = Acqrs_getInstrumentData(inst_, buf, &serial_number_, &bus_number_, &slot_number_);
    return st == VI_SUCCESS;
}

//static
std::string
digitizer::error_msg( int status, const char * ident )
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
digitizer::checkError( ViSession instId, ViStatus st, const char * text, ViInt32 arg )
{
    if ( st == VI_SUCCESS )
        return false;
    std::array< ViChar, 1024 > msg;
    std::fill( msg.begin(), msg.end(), 0 );
    AcqrsD1_errorMessageEx( instId, st, msg.data(), msg.size() );
    std::ostringstream o;
    o << boost::format( "%s (0x%x): %s" ) % text % int(st) % msg.data();
    if ( arg )
        o << " #" << arg;
    ADDEBUG() << o.str();
    return false;
}

bool
digitizer::initialize()
{
    bool simulate( false );
    if ( const auto p = getenv( "AcqirisOption" ) )
        simulate = std::strcmp( p, "simulate" ) == 0;
    
    if ( getenv("AcqirisDxDir") == 0 ) {
        ADDEBUG() << L"AcqirisDxDir environment variable not set.";
        if ( !simulate )
            return false;
    }

    struct stat st;
    if ( stat( "/dev/acqrsPCI", &st ) != 0 ) {
        ADDEBUG() << L"/dev/acqrsPID does not exists";
        if ( !simulate )
            return false;
    }
    return true;
}

bool
digitizer::findDevice()
{
    ViStatus status;
    
    numInstruments_ = 0;
    
    if ( ( status = AcqrsD1_multiInstrAutoDefine( "cal=0", &numInstruments_ ) ) != VI_SUCCESS ) {
        ADDEBUG() << error_msg( status, "digitizer::findDevice()" );
    } else {
        ADDEBUG() << boost::format( "find %1% acqiris devices." ) % numInstruments_;
    }

    if ( numInstruments_ == 0 ) {
        if ( auto p = getenv( "AcqirisOption" ) ) {
            if ( std::strcmp( p, "simulate" ) == 0 ) {
                
                if ( Acqrs_setSimulationOptions( "M2M" ) == VI_SUCCESS ) {
                    //if ( Acqrs_InitWithOptions( const_cast<char*>("PCI::DC110")
                    if ( Acqrs_InitWithOptions( const_cast<char*>("PCI::DC271")
                                                , VI_FALSE, VI_FALSE, const_cast<char *>("simulate=TRUE"), &inst_ ) == VI_SUCCESS ) {
                        numInstruments_ = 1;
                        __isSimulated__ = true;
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

            ViInt32 value;
            if ( Acqrs_getInstrumentInfo( inst_, "NbrADCBits", &value ) == VI_SUCCESS )
                nbrADCBits_ = value;

            
            ADDEBUG() << "\tfound device on: " << device_name_;
            return true;
        } else {
            std::cout << error_msg( status, "digitizer::findDevice" ) << std::endl;
        }
    }
    return false;
}

std::shared_ptr< acqrscontrols::aqdrv4::acqiris_method >
digitizer::digitizer_setup( std::shared_ptr< const acqrscontrols::aqdrv4::acqiris_method > m )
{    
    ViStatus status;
    ViStatus * pStatus = &status;

    constexpr ViInt32 nbrSegments = 1;

    status = AcqrsD1_configMultiInput( inst_, 1, 0 );
    checkError( inst_, status, "AcqrsD1_configMultiInput", __LINE__  );

    auto adapted = std::make_shared< acqrscontrols::aqdrv4::acqiris_method >( *m );

    // if ap240
    //status = AcqrsD1_configChannelCombination( inst_, 2, 1 );
    //checkError( inst_, status, "AcqrsD1_configChannelCombination", __LINE__  );

    // vertical setup
    // configVertical( channel = -1 must be done before configTrigSource, see Programmer's Guide p23 )
    if ( auto ext = m->ext() ) {
        if ( ext->enable ) {
            status = AcqrsD1_configVertical( inst_
                                             , -1
                                             , ext->fullScale  
                                             , ext->offset     
                                             , ext->coupling   
                                             , ext->bandwidth );
            
            if ( status == ACQIRIS_WARN_SETUP_ADAPTED ) {
                ViReal64 fullScale, offset; ViInt32 coupling, bandwidth;
                if ( AcqrsD1_getVertical( inst_, -1, &fullScale, &offset, &coupling, &bandwidth ) == VI_SUCCESS ) {
                    auto ax = adapted->mutable_ext();
                    ax->fullScale = fullScale;
                    ax->offset = offset;
                    ax->coupling = coupling;
                    ax->bandwidth = bandwidth;
                }
            } else if ( status != VI_SUCCESS ) {
                checkError( inst_, status, ( boost::format( "AcqrsD1_configVertical (%d)" ) % (-1) ).str().c_str(), __LINE__ );
            }
        }
    }
    

    ViInt32 trigChannel( -1 );

    if ( auto trig = m->trig() ) {

        ViInt32 trigClass( trig->trigClass ), trigPattern( trig->trigPattern ), a(0), b(0);
        ViReal64 c(0), d(0);

        if ( ( status =
               AcqrsD1_configTrigClass( inst_, trig->trigClass, trig->trigPattern, 0, 0, 0, 0 ) ) == ACQIRIS_WARN_SETUP_ADAPTED ) {
            if ( AcqrsD1_getTrigClass( inst_, &trigClass, &trigPattern, &a, &b, &c, &d ) == VI_SUCCESS ) {
                std::cout << boost::format( "\ttrigClass: %x <- %x, trigPattern: %x <- %x")
                    % trigClass % trig->trigClass % trigPattern % trig->trigPattern << std::endl;
                auto ax = adapted->mutable_trig();
                ax->trigClass = trigClass;
                ax->trigPattern = trigPattern;
            }
        }
        checkError( inst_, status, ( boost::format( "AcqrsD1_configTrigClass(trig = %d)") % trigChannel ).str().c_str() , __LINE__  );

        trigChannel = trigPattern & 0x80000000 ? (-1) : trigPattern & 0x03;
        
        status = AcqrsD1_configTrigSource( inst_
                                           , trigChannel
                                           , trig->trigCoupling
                                           , trig->trigSlope
                                           , trig->trigLevel1
                                           , trig->trigLevel2 );
        checkError( inst_, status, ( boost::format( "AcqrsD1_configTrigSource(trig = %d)") % trigChannel ).str().c_str() , __LINE__  );
        ViInt32 xTrigCoupling, xTrigSlope;
        ViReal64 xTrigLevel1, xTrigLevel2;
        if ( status == ACQIRIS_WARN_SETUP_ADAPTED ) {
            AcqrsD1_getTrigSource( inst_, trigChannel, &xTrigCoupling, &xTrigSlope, &xTrigLevel1, &xTrigLevel2 );
            ADDEBUG() << xTrigCoupling << ", " << xTrigSlope << ", " << xTrigLevel1 << ", " << xTrigLevel2;
        }
    }

    const int chlist [] = { 1, 2 };
    int idx = 0;
    for ( auto& ver: { m->ch1(), m->ch2() } ) {
        int channel = chlist[ idx++ ];
        if ( ver && ver->enable ) {
            
            status = AcqrsD1_configVertical( inst_
                                             , channel
                                             , ver->fullScale  
                                             , ver->offset     
                                             , ver->coupling   
                                             , ver->bandwidth );
            
            if ( status == ACQIRIS_WARN_SETUP_ADAPTED ) {
                ViReal64 fullScale, offset; ViInt32 coupling, bandwidth;
                if ( AcqrsD1_getVertical( inst_, channel, &fullScale, &offset, &coupling, &bandwidth ) == VI_SUCCESS ) {
                    auto ax = ( idx == 1 ) ? adapted->mutable_ch1() : adapted->mutable_ch2();
                    ax->fullScale = fullScale;
                    ax->offset = offset;
                    ax->coupling = coupling;
                    ax->bandwidth = bandwidth;
                }
            } else if ( status != VI_SUCCESS ) {
                ADDEBUG() << "AcqrsD1_configVertical(" << channel << ", "
                          << ver->fullScale << ", " << ver->offset << ", " << ver->coupling << ", " << ver->bandwidth << ")";
                checkError( inst_, status, ( boost::format( "AcqrsD1_configVertical (%d)" ) % channel ).str().c_str(), __LINE__ );
            }
        }
    }
    
    // horizontal setup
    if ( auto hor = m->hor() ) {
        nbrSamples_ = hor->nbrSamples;
        nbrWaveforms_ = 1;
        delayTime_ = hor->delayTime;
        ViReal64 sampInterval( hor->sampInterval );
        
        if ( ( status = AcqrsD1_configHorizontal( inst_, hor->sampInterval, hor->delayTime ) ) != VI_SUCCESS ) {
            if ( status == ACQIRIS_WARN_SETUP_ADAPTED ) {
                if ( AcqrsD1_getHorizontal( inst_, &sampInterval, &delayTime_ ) == VI_SUCCESS ) {
                    checkError( inst_, status, "AcqrsD1_configHorizontal", __LINE__  );
                    ADDEBUG() << boost::format( "\tsampInterval: %gns <- %gns, delay: %e <- %e\n" )
                        % ( sampInterval * std::nano::den )
                        % ( hor->sampInterval * std::nano::den )
                        % delayTime_ % hor->delayTime;
                    auto ax = adapted->mutable_hor();
                    ax->sampInterval = sampInterval;
                    ax->delayTime = delayTime_;
                }
            } else
                checkError( inst_, status, "AcqrsD1_configHorizontal", __LINE__  );
        }

        if ( (status = AcqrsD1_configMemory( inst_, hor->nbrSamples, nbrSegments )) != VI_SUCCESS ) {
            if ( status == ACQIRIS_WARN_SETUP_ADAPTED ) {
                ViInt32 nSegments;
                if ( AcqrsD1_getMemory( inst_, &nbrSamples_, &nSegments ) == VI_SUCCESS )
                    
                    if ( hor->nbrSamples != nbrSamples_ ) {
                        ADDEBUG() << "\tnbrSamples adapted from " << hor->nbrSamples << " to " << nbrSamples_;
                        checkError( inst_, status, "AcqrsD1_configMemory", __LINE__ );
                        auto ax = adapted->mutable_hor();
                        ax->nbrSamples = nbrSamples_;
                    }
            } else
                checkError( inst_, status, "AcqrsD1_configMemory", __LINE__ );
        }
    
        status = AcqrsD1_configMode( inst_, 0, 0, 0 ); // 2 := averaging mode, 0 := normal data acq.
        checkError( inst_, status, "AcqrsD1_configMode", __LINE__  );
    }

    return adapted;
}

bool
digitizer::acquire()
{
    auto status = AcqrsD1_acquire( inst_ );
    checkError( inst_, status, "AcqrsD1_acquire", __LINE__ );
    return status == VI_SUCCESS;
}

bool
digitizer::stop()
{
    return AcqrsD1_stopAcquisition( inst_ ) == VI_SUCCESS;
}

double
digitizer::delayTime() const
{
    return delayTime_;
}

int
digitizer::nbrADCBits() const
{
    return nbrADCBits_;
}

int
digitizer::readTemperature()
{
    AcqrsD1_getInstrumentInfo( inst_, "Temperature", &temperature_ );
    return temperature_;
}

int
digitizer::temperature() const
{
    return temperature_;
}

bool
digitizer::isSimulated() const
{
    return __isSimulated__;
}
