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

#include <AcqirisImport.h> 
#include <AcqirisD1Import.h>
#include <cstring>
#include <string>
#include <vector>
#include <memory>

namespace acqrscontrols {
    namespace aqdrv4 { class acqiris_method; }
}

template< typename T = int8_t > struct readMode {  inline static int value(); };
template<> inline int readMode<int8_t>::value() { return 0; };
template<> inline int readMode<int16_t>::value() { return 1; };
template<> inline int readMode<int32_t>::value() { return 2; };

class digitizer {
public:
    digitizer() : numInstruments_( 0 )
                , inst_( -1 )
                , initialized_( false )
                , bus_number_( 0 )
                , slot_number_( 0 )
                , serial_number_( 0 )
                , nbrADCBits_( 8 ) {
    }
    
    bool initialize();
    bool findDevice();
    
    std::shared_ptr< acqrscontrols::aqdrv4::acqiris_method > digitizer_setup( std::shared_ptr< const acqrscontrols::aqdrv4::acqiris_method > );
    bool acquire();
    bool stop();

    enum result_code { success, error_timeout, error_overload, error_io_read, error_stopped };

    result_code waitForEndOfAcquisition( size_t timeout );
    
    bool getInstrumentData();
    
    static std::string error_msg( int status, const char * ident );
    static bool checkError( ViSession instId, ViStatus st, const char * text, ViInt32 arg = 0 );

    template<typename T>
    bool readData( int channel, AqDataDescriptor& dataDesc, AqSegmentDescriptor& segDesc, std::vector<int32_t>& data ) {
        
        std::memset( &dataDesc, sizeof(dataDesc), 0 );
        std::memset( &segDesc, sizeof(segDesc), 0 );        

        AqReadParameters readPar;

        readPar.dataType = readMode<T>::value();
        readPar.readMode = ReadModeStdW; /* Single-segment read mode */
        readPar.firstSegment = 0;
        readPar.nbrSegments = 1;
        readPar.firstSampleInSeg = 0;
        readPar.nbrSamplesInSeg = nbrSamples_;
        readPar.segmentOffset = 0;
        readPar.dataArraySize = (nbrSamples_ + 32) * sizeof(T);
        readPar.segDescArraySize = sizeof(AqSegmentDescriptor);
        readPar.flags = 0;
        readPar.reserved = 0;
        readPar.reserved2 = 0;
        readPar.reserved3 = 0;

        data.resize( ( ( ( nbrSamples_ + 32 ) * sizeof(T) ) + ( sizeof(int32_t)-1) ) / sizeof( int32_t ) );

        auto status = AcqrsD1_readData( inst_, 1, &readPar, data.data(), &dataDesc, &segDesc );
        checkError( inst_, status, "AcqirisD1_readData", __LINE__  );

        return status == VI_SUCCESS;
    }

    double delayTime() const;
    int nbrADCBits() const;
    int temperature() const;
    int readTemperature();
    bool isSimulated() const;

private:
    ViSession inst_;
    ViInt32 numInstruments_;
    std::string device_name_;
    bool initialized_;
    ViInt32 bus_number_;
    ViInt32 slot_number_;
    ViInt32 serial_number_;
    ViInt32 nbrSamples_;
    ViInt32 nbrWaveforms_;
    double delayTime_;
    std::shared_ptr< const acqrscontrols::aqdrv4::acqiris_method > method_;
    ViInt32 nbrADCBits_;
    ViInt32 temperature_;
};

