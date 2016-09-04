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

class acqiris {
public:
    acqiris() : numInstruments_( 0 )
              , inst_( -1 )
              , initialized_( false )
              , bSimulated_( false )
              , bus_number_( 0 )
              , slot_number_( 0 )
              , serial_number_( 0 ) {
    }
    
    bool initialize();
    bool findDevice();
    bool averager_setup( int nDelay, int nSamples, int nAverage );
    bool digitizer_setup( double delay, double width );

    bool acquire();
    bool stop();

    enum result_code { success, error_timeout, error_overload, error_io_read, error_stopped };

    result_code waitForEndOfAcquisition( size_t timeout );
    
    bool getInstrumentData();
    
    static std::string error_msg( int status, const char * ident );
    static bool checkError( ViSession instId, ViStatus st, const char * text, ViInt32 arg = 0 );

    template<typename T >
    bool readData( int channel, AqDataDescriptor& dataDesc, AqSegmentDescriptor& segDesc, std::vector<T>& data ) {
        
        std::memset( &dataDesc, sizeof(dataDesc), 0 );
        std::memset( &segDesc, sizeof(segDesc), 0 );        

        AqReadParameters readPar;

        readPar.dataType = 0; //ViInt32( sizeof(T)/8 - 1 ); // ReadInt8 = 0, ReadInt16 = 1, ...
        readPar.readMode = ReadModeStdW; /* Single-segment read mode */
        readPar.firstSegment = 0;
        readPar.nbrSegments = 1;
        readPar.firstSampleInSeg = 0;
        readPar.nbrSamplesInSeg = nbrSamples_;
        readPar.segmentOffset = 0;
        readPar.dataArraySize = (nbrSamples_ + 32) * sizeof(T);
        readPar.segDescArraySize = sizeof(AqSegmentDescriptor);
        data.resize( nbrSamples_ + 32 );
        readPar.flags = 0;
        readPar.reserved = 0;
        readPar.reserved2 = 0;
        readPar.reserved3 = 0;

        //std::cout << "number-of-samples: " << nbrSamples_ << std::endl;
        auto status = AcqrsD1_readData( inst_, 1, &readPar, data.data(), &dataDesc, &segDesc );
        checkError( inst_, status, "AcqirisD1_readData", __LINE__  );

        return status == VI_SUCCESS;
    }

private:
    ViSession inst_;
    ViInt32 numInstruments_;
    std::string device_name_;
    std::string model_name_;
    bool initialized_;
    bool bSimulated_;
    ViInt32 bus_number_;
    ViInt32 slot_number_;
    ViInt32 serial_number_;
    ViInt32 nbrSamples_;
    ViInt32 nStartDelay_;
    ViInt32 nbrWaveforms_;
};

