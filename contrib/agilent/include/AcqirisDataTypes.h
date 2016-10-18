#ifndef _ACQIRISDATATYPES_H
#define _ACQIRISDATATYPES_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  AcqirisDataTypes.h:    Agilent Acqiris Readout Data Types
//
//----------------------------------------------------------------------------------------
//  Copyright Agilent Technologies, Inc. 2000, 2001-2009
//
//////////////////////////////////////////////////////////////////////////////////////////

#include "vpptype.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Data Structures for '...readData' and '...writeData' functions

typedef struct 
{
    ViInt32 dataType;               //!< 0 = 8-bit (char)  1 = 16-bit (short) 2 = 32-bit (integer)  3 = 64-bit (real)
                                    //!< or use 'enum AqReadType' defined below
    ViInt32 readMode;               //!< Use 'enum AqReadDataMode' defined below
    ViInt32 firstSegment;
    ViInt32 nbrSegments;
    ViInt32 firstSampleInSeg;
    ViInt32 nbrSamplesInSeg;
    ViInt32 segmentOffset;
    ViInt32 dataArraySize;          //!< In bytes            
    ViInt32 segDescArraySize;       //!< In bytes            
    ViInt32 flags;                  //!< Use 'enum AqReadDataFlags' defined below
    ViInt32 reserved;
    ViReal64 reserved2;
    ViReal64 reserved3;
} AqReadParameters;

typedef struct 
{
    ViInt32 dataType;               //!< 0 = 8-bit (char)  1 = 16-bit (short) 2 = 32-bit (integer)  3 = 64-bit (real)
                                    //!< or use 'enum ReadType' defined below
    ViInt32 writeMode;              //!< Use 'enum AqWriteDataMode' defined below
    ViInt32 firstSegment;
    ViInt32 nbrSegments;
    ViInt32 firstSampleInSeg;
    ViInt32 nbrSamplesInSeg;
    ViInt32 segmentOffset;
    ViInt32 dataArraySize;          //!< In bytes            
    ViInt32 segDescArraySize;       //!< In bytes            
    ViInt32 flags;                  //!< Use 'enum AqReadDataFlags' defined below
    ViInt32 reserved;
    ViReal64 reserved2;
    ViReal64 reserved3;
} AqWriteParameters;

typedef struct
{
    ViReal64 horPos;
    ViUInt32 timeStampLo;           //!< Timestamp 
    ViUInt32 timeStampHi;
} AqSegmentDescriptor;

//! For 'readMode = ReadModeSeqRawW' only.
typedef struct
{
    ViReal64 horPos;
    ViUInt32 timeStampLo;           //!< Timestamp 
    ViUInt32 timeStampHi;
    ViUInt32 indexFirstPoint;       //!< Pointer to the first sample of this segment, 
                                    //!< into the array passed to 'readData'.
    ViUInt32 actualSegmentSize;     //!< In samples, for the circular buffer wrapping.
    ViInt32  reserved;
} AqSegmentDescriptorSeqRaw;

typedef struct
{
    ViReal64 horPos;
    ViUInt32 timeStampLo;           //!< Timestamp 
    ViUInt32 timeStampHi;
    ViUInt32 actualTriggersInSeg;   //!< Number of actual triggers acquired in this segment                
    ViInt32  avgOvfl;               //!< Acquisition overflow (avg)
    ViInt32  avgStatus;             //!< Average status (avg)
    ViInt32  avgMax;                //!< Max value in the sequence (avg)
    ViUInt32 flags;                 //!< Bits 0-3: markers (avg)
    ViInt32  reserved;
} AqSegmentDescriptorAvg;

//! For backward compatibility.
#define addrFirstPoint indexFirstPoint

typedef struct
{
    ViInt32     returnedSamplesPerSeg;
    ViInt32     indexFirstPoint;    //!< 'data[desc.indexFirstPoint]' is the first valid point. 
                                    //!< Note: Not valid for 'readMode = ReadModeSeqRawW'.
    ViReal64    sampTime;
    ViReal64    vGain;
    ViReal64    vOffset;
    ViInt32     returnedSegments;   //!< When reading multiple segments in one waveform
    ViInt32     nbrAvgWforms;        
    ViUInt32    actualTriggersInAcqLo;
    ViUInt32    actualTriggersInAcqHi;
    ViUInt32    actualDataSize;
    ViInt32     reserved2;    
    ViReal64    reserved3;
} AqDataDescriptor;

typedef struct     
{
    ViInt32 GatePos;
    ViInt32 GateLength;
} AqGateParameters;

enum AqReadType 
{ 
    ReadInt8 = 0, 
    ReadInt16, 
    ReadInt32, 
    ReadReal64, 
    ReadRawData,
    nbrAqReadType,
};    

enum AqReadDataMode 
{
    ReadModeStdW = 0,   //!< Standard waveform
    ReadModeSeqW,       //!< Sequenced waveform
    ReadModeAvgW,       //!< Averaged waveform
    ReadModeGateW,      //!< Gated waveform
    ReadModePeak,       //!< Peaks of a waveform 
    ReadModeShAvgW,     //!< Standard short averaged
    ReadModeSShAvgW,    //!< Short shifted averaged waveform 
    ReadModeSSRW,       //!< Sustained sequential recording
    ReadModeZsW,        //!< Zero suppressed waveform
    ReadModeHistogram,  //!< Histogram
    ReadModePeakPic,    //!< Peak picture
    ReadModeSeqRawW,    //!< Raw Sequenced waveform (no unwrap)
    nbrAqReadDataMode 
};

enum AqReadDataFlags 
{
    AqIgnoreTDC          = 0x0001, //!< If set, the TDC value (if any) will be ignored.
    AqIgnoreLookUp       = 0x0002, //!< If set, the lookup table (if any) will not be applied on data.
    AqSkipClearHistogram = 0x0004, //!< If set, the histogram will be not be zero-ed during read
    AqSkipCircular2Linear= 0x0008, //!< If set, the memory image will be transferred in ReadModeSeqW
    AqDmaToCompanion     = 0x0010, //!< If set, a VX407c device will transfer data to its companion device.
};


//////////////////////////////////////////////////////////////////////////////////////////
// Constants for D1 configMode

enum AqAcqMode
{
    AqAcqModeDigitizer      = 0,    //!< Normal Digitizer mode
    AqAcqModeRepeat         = 1,    //!< Continous acquisition and streaming to DPU (for ACs / SCs)
    AqAcqModeAverager       = 2,    //!< Averaging mode (for real-time averagers only)
    AqAcqModePingPong       = 3,    //!< Buffered acquisition (AP201 / AP101 only)
    AqAcqModePeakTDC        = 5,    //!< Peak detection
    AqAcqModeFreqCounter    = 6,    //!< Frequency counter
    AqAcqModeSSR            = 7,    //!< AP235 / AP240 SSR mode
};

//////////////////////////////////////////////////////////////////////////////////////////
// Constants for power

enum AqPowerState
{
    AqPowerOff = 0,
    AqPowerOn = 1,
    nbrAqPowerState,
};

//////////////////////////////////////////////////////////////////////////////////////////
//  AcqrsT3Interface structure definitions

typedef struct
{
    ViAddr dataArray;           //!< Pointer to user allocated memory
    ViUInt32 dataSizeInBytes;   //!< Size of user allocated memory in bytes
    ViInt32 nbrSamples;         //!< Number of samples requested
    ViInt32 dataType;           //!< Use 'enum AqReadType' defined above
    ViInt32 readMode;           //!< Use 'enum AqT3ReadModes' defined below
    ViInt32 reserved3;          //!< Reserved, must be 0
    ViInt32 reserved2;          //!< Reserved, must be 0
    ViInt32 reserved1;          //!< Reserved, must be 0
} AqT3ReadParameters;

typedef struct
{
    ViAddr dataPtr;             //!< Pointer to returned data (same as user buffer if aligned)
    ViInt32 nbrSamples;         //!< Number of samples returned
    ViInt32 sampleSize;         //!< Size in bytes of one data sample
    ViInt32 sampleType;         //!< Type of the returned samples, see AqT3SampleType below
    ViInt32 flags;              //!< Readout flags
    ViInt32 reserved3;          //!< Reserved, will be 0
    ViInt32 reserved2;          //!< Reserved, will be 0
    ViInt32 reserved1;          //!< Reserved, will be 0
} AqT3DataDescriptor;

enum AqT3ReadModes
{
    AqT3ReadStandard,           //!< Standard read mode
    AqT3ReadContinuous,         //!< Continuous read mode
    nbrAqT3ReadModes,
};

enum AqT3SampleType
{
    AqT3SecondReal64,           //!< Double value in seconds
    AqT3Count50psInt32,         //!< Count of 50 ps
    AqT3Count5psInt32,          //!< Count of 5 ps
    AqT3Struct50ps6ch,          //!< Struct on 32 bits with (LSB to MSB)
                                //!<  27 bits count of 50 ps, 3 bits channel number, 1 bit overflow
    nbrAqT3SampleType,
};

//////////////////////////////////////////////////////////////////////////////////////////
// Device type. It determines the API interface that will be used.
enum AqDevType
{
    AqDevTypeInvalid = 0,   //!< Invalid, or 'ALL', depending on the context.
    AqD1 = 1,               //!< Digitizer.
    AqG2 = 2,               //!< Generator first generation (RC2xx).
    AqD1G2 = 3,             //!< Digitizer+Generator.
    AqT3 = 4,               //!< Digital timer/tester.
    AqG4 = 5,               //!< Generator second generation (GVMxxx).
    AqD1G4 = 6,             //!< Digitizer+GeneratorNG (RVMxxxx).
    AqP5 = 7,               //!< Processing board.
    nbrAqDevType,
};

#endif // sentry

