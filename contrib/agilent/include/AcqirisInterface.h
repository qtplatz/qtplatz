///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  AcqirisInterface.h:    Common Driver Function Declarations
//
//----------------------------------------------------------------------------------------
//  Copyright (C) Agilent_Technologies, Inc. 2006-2009
//
//  Purpose:    Declaration of Acqrs device driver API
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "vpptype.h"
#include "AcqirisDataTypes.h"

// The following prefixes should be set outside of this file, typically by the
// AcqirisImport.h file, to specify the calling convention used for the function
// and whether it is exported or imported.
#if !defined(ACQ_DLL) || !defined(ACQ_CC)
#error AcqirisInterface.h should not be included directly, please use AcqirisImport.h instead.
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////      
// General comments about the function prototypes:
//
// - All function calls require the argument 'instrumentID' in order to identify
//   the Agilent_Technologies card to which the call is directed.
//   The only exceptions are the initialization functions 'Acqrs_getNbrInstruments',
//     'Acqrs_setSimulationOptions', 'Acqrs_init' and 'Acqrs_InitWithOptions'.
//   The last two functions actually return instrument identifiers at initialization time,
//   for subsequent use in the other function calls.
//
// - All function calls return a status value of type 'ViStatus' with information about
//   the success or failure of the call.
//   The Agilent_Technologies specific values are defined in the AcqirisErrorCodes.h file.
//   The generic VISA ones are listed in the header file 'vpptype.h'.
//
// - If important parameters supplied by the user (e.g. an invalid instrumentID) are found
//   to be invalid, most functions do not execute and return an error code of the type
//   VI_ERROR_PARAMETERi, where i = 1, 2, ... corresponds to the argument number.
//
// - If the user attempts (with a function Acqrs_configXXXX) to set a instrument
//   parameter to a value which is outside of its acceptable range, the function
//   typically adapts the parameter to the closest available value and returns
//   ACQIRIS_WARN_SETUP_ADAPTED. The instrument parameters actually set can be retrieved
//   with the 'query' functions Acqrs_getXXXX.
//
// - All calls to an instrument which was previously suspended using 'Acqrs_suspendControl' will return
//   ACQIRIS_ERROR_INVALID_HANDLE, until 'Acqrs_resumeControl' is called.
//
// - Data are always returned through pointers to user-allocated variables or arrays.
// 
///////////////////////////////////////////////////////////////////////////////////////////////////



//! Performs an auto-calibration of the instrument.
/*! Equivalent to Acqrs_calibrateEx with 'calType' = 0 */
ACQ_DLL ViStatus ACQ_CC Acqrs_calibrate(ViSession instrumentID); 



//! Interrupts the calibration and return.
/*! If a calibration is run by another thread, this other thread will be interrupted immediately and it
    will get the error 'ACQIRIS_ERROR_OPERATION_CANCELLED'.

Returns one of the following ViStatus values:
    VI_SUCCESS                           Always. */
ACQ_DLL ViStatus ACQ_CC Acqrs_calibrateCancel(ViSession instrumentID);



//! Performs a (partial) auto-calibration of the instrument.
/*! 'calType'=  0    calibrate the entire instrument (equivalent to 'Acqrs_calibrate')
                1    calibrate only the current channel configuration, 
                     as set by 'Acqrs_configChannelCombination'
                2    calibrate external clock timing. Requires operation in External Clock 
                     (Continuous), i.e. 'clockType' = 1, as set with 'Acqrs_configExtClock'
                3    calibrate only at the current frequency (only instruments with fine resolution 
                     time bases)
                4    calibrate fast, only at the current settings (if supported). 
                     Note: In this mode, any change on 'fullscale', 'bandwidth limit', 'channel 
                     combine', 'impedance' or 'coupling' will require a new calibration.
                      
    'modifier'    
             ['calType' = 0]    currently unused, set to zero
             ['calType' = 1]    currently unused, set to zero
             ['calType' = 2]    currently unused, set to zero
             ['calType' = 3]  0    = All channels
                             >0    = Only specified channel (for i.e. 1 = calibrate only channel 1)
             ['calType' = 4]  0    = All channels
                             >0    = Only specified channel (for i.e. 1 = calibrate only channel 1)

    'flags'            currently unused, set to zero 

    Returns one of the following ViStatus values:
    ACQIRIS_ERROR_INSTRUMENT_RUNNING   if the instrument is currently running.
    ACQIRIS_ERROR_CALIBRATION_FAILED   if the requested calibration failed.
    ACQIRIS_ERROR_NOT_SUPPORTED        if the requested calibration is not supported by the 
                                       instrument.
    ACQIRIS_ERROR_CANNOT_READ_THIS_CHANNEL if the requested channel is not available.
    ACQIRIS_ERROR_COULD_NOT_CALIBRATE  if the requested frequency is invalid ('calType' = 2 only).
    ACQIRIS_ERROR_ACQ_TIMEOUT          if an acquisition timed out during the calibration 
                                       (e.g. no clocks provided).
    ACQIRIS_ERROR_CANCELLED            if the calibration has been cancelled.
    VI_SUCCESS                         otherwise. */
ACQ_DLL ViStatus ACQ_CC Acqrs_calibrateEx(ViSession instrumentID, ViInt32 calType,
    ViInt32 modifier, ViInt32 flags); 



//! Load from 'filePathName' binary file all calibration values and info.
/*! 'filePathName'     name of the file,path or 'NULL' (see 'flags').
    'flags'            = 0, default filename. Calibration values will be loaded from the snXXXXX_calVal.bin
                            file in the working directory. 'filePathName' MUST be NULL or (empty String).
                       = 1, specify path only. Calibration values will be loaded from the snXXXXX_calVal.bin
                            file in the speficied directory. 'filePathName' MUST be non-NULL.
                       = 2, specify filename. 'filePathName' represents the filename (with or without path),
                            and MUST be non-NULL and non-empty.
 
    Returns one of the following ViStatus values:
    ACQIRIS_ERROR_CAL_FILE_CORRUPTED         if the file is corrupted.
    ACQIRIS_ERROR_CAL_FILE_VERSION           if the file has been generated with a different driver version (major and minor).
    ACQIRIS_ERROR_CAL_FILE_SERIAL            if the file does not correspond to this (multi)instrument.
    ACQIRIS_ERROR_FILE_NOT_FOUND             if the file is not found.
    ACQIRIS_ERROR_NOT_SUPPORTED              if the instrument does not support this feature. 
    VI_SUCCESS                               otherwise. */
ACQ_DLL ViStatus ACQ_CC Acqrs_calLoad(ViSession instrumentID, ViConstString filePathName, ViInt32 flags);



//! Query about the desirability of a calibration for the current requested configuration.
/*! 'chan '      = 0, the query will be done on all channels i.e. isRequiredP = VI_TRUE if at least 1 channel needs to be calibrated.
                 = n, the query will be done on channel n

    Returns 'isRequiredP':
                 VI_TRUE if the channel number 'chan' of the instrument needs to be calibrated.
                         This is the case if it has been calibrated more than two hours ago,
                         or if the temperature deviation since the last calibration is more than 5 degrees.
                 VI_FALSE otherwise.

    Returns one of the following ViStatus values:
    ACQIRIS_ERROR_NOT_SUPPORTED             if the instrument does not support this feature. 
    VI_SUCCESS                              otherwise. */
ACQ_DLL ViStatus ACQ_CC Acqrs_calRequired(ViSession instrumentID, ViInt32 channel, ViBoolean* isRequiredP);



//! Save in 'filePathName' binary file all calibration values and info.
/*! 'filePathName'     name of the file,path or 'NULL' (see 'flags').
    'flags'            = 0, default filename. Calibration values will be saved in the snXXXXX_calVal.bin
                            file in the working directory. 'filePathName' MUST be NULL or (empty String).
                       = 1, specify path only. Calibration values will be saved in the snXXXXX_calVal.bin
                            file in the speficied directory. 'filePathName' MUST be non-NULL.
                       = 2, specify filename. 'filePathName' represents the filename (with or without path),
                            and MUST be non-NULL and non-empty.

    Returns one of the following ViStatus values:
    ACQIRIS_ERROR_NOT_SUPPORTED             if the instrument does not support this feature.
    ACQIRIS_ERROR_NO_ACCESS                 if the access to the file is denied.
    VI_SUCCESS                              otherwise. */
ACQ_DLL ViStatus ACQ_CC Acqrs_calSave(ViSession instrumentID, ViConstString filePathName, ViInt32 flags);



//! Close specified instrument. 
/*! Once closed, this instrument is not available anymore and
    need to be reenabled using 'InitWithOptions' or 'init'.
    Note: For freeing properly all resources, 'closeAll' must still be called when
    the application close, even if 'close' was called for each instrument.

    Returns one of the following ViStatus values:
    VI_SUCCESS always. */
ACQ_DLL ViStatus ACQ_CC Acqrs_close(ViSession instrumentID); 



//! Closes all instruments and prepares for closing of application.
/*! 
    Returns one of the following ViStatus values:
    VI_SUCCESS always. */
ACQ_DLL ViStatus ACQ_CC Acqrs_closeAll(void); 



//! Load/Clear/SetFilePath of the on-board logic devices. 
/*! ONLY useful for a module with on-board programmable logic devices 
   (SCxxx, ACxxx, APxxx, and 12-bit Digitizers).

   'flags'= operation
            = 0 program device, using strictly the specified path/file name
            = 1 clear device
            = 2 clear and program devices using appropriate files from specified path.
            = 3 program device, but also look for file path in the 'AqDrv4.ini' file, if the file 
                was not found. It is sufficient to specify the file name (without explicit path), 
                if the file is either in the current working directory or in the directory pointed 
                to by 'FPGAPATH' in 'AqDrv4.ini'.

   'deviceName'   Identifies which device to program. 
      ['flags' = 0 or 3] = device to program.
          This string must be "Block1Devx", with 'x' = is the FPGA number to be programmed. 
          For instance, in the SC240, it must be "Block1Dev1". 
      ['flags' = 1] = device to clear, must be "Block1DevAll". 
      ['flags' = 2] = unused.

   'filePathName'  
      ['flags' = 0 or 3] = file path and file name. 
      ['flags' = 1] = unused. 
      ['flags' = 2] = path (no file name !) where all .bit files can be found.

   Note: Most users do not need to call this function. Check the manual for further details.

   Returns one of the following ViStatus values:
   ACQIRIS_ERROR_PARAM_STRING_INVALID    if 'deviceName' is invalid.
   ACQIRIS_ERROR_FILE_NOT_FOUND          if an FPGA file could not be found.
   ACQIRIS_ERROR_NO_DATA                 if an FPGA file did not contain expected 
                                         data.
   ACQIRIS_ERROR_FIRMWARE_NOT_AUTHORIZED if the instrument is not authorized to use the 
                                         specified 'filePathName' file.
   ACQIRIS_ERROR_EEPROM_DATA_INVALID     if the CPLD 'filePathName' is invalid.
   ACQIRIS_ERROR_FPGA_y_LOAD             if an FPGA could not be loaded, 
                                         where 'y' = FPGA nbr. 
   ACQIRIS_WARN_SETUP_ADAPTED            if one of the parameters has been adapted.
   VI_SUCCESS                            otherwise. */
ACQ_DLL ViStatus ACQ_CC Acqrs_configLogicDevice(ViSession instrumentID, 
    ViChar deviceNameP[], ViChar filePathNameP[], ViInt32 flags);



//! Translates an error code into a human readable form. 
/*! For file errors, the returned message will also contain the file name and the 
    original 'ansi' error string.

   'errorCode'          = Error code (returned by a function) to be translated
   'errorMessage'       = Pointer to user-allocated character string (suggested size 512 bytes),
                          into which the error message text will be copied.
   'errorMessageSize'   = size of 'errorMessage', in bytes.

   NOTE: 'instrumentID' can be VI_NULL.

   Returns one of the following ViStatus values:
   ACQIRIS_ERROR_BUFFER_OVERFLOW    if 'errorMessageSize' is too small.
   VI_SUCCESS                       otherwise. */
ACQ_DLL ViStatus ACQ_CC Acqrs_errorMessage(ViSession instrumentID, ViStatus errorCode, 
    ViChar errorMessage[], ViInt32 errorMessageSize);



//! Returns the API interface type appropriate to this 'instrumentID' and 'channel'.
/*! 'devTypeP'          = The device type. See 'AqDevType'.

    Returns one of the following ViStatus values:
    VI_SUCCESS                         always. */
ACQ_DLL ViStatus ACQ_CC Acqrs_getChanDevType(ViSession instrumentID, ViInt32 channel, ViInt32* devTypeP);



//! Returns the device type of the specified 'instrumentID'.
/*! 'devTypeP'          = The device type. See 'AqDevType'.

    Returns one of the following ViStatus values:
    VI_SUCCESS                         always. */
ACQ_DLL ViStatus ACQ_CC Acqrs_getDevType(ViSession instrumentID, ViInt32* devTypeP);



//! Returns the device type of the instrument specified by 'deviceIndex'.
/*! 'devTypeP'          = The device type. See 'AqDevType'.

    Returns one of the following ViStatus values:
    VI_SUCCESS                         always. */
ACQ_DLL ViStatus ACQ_CC Acqrs_getDevTypeByIndex(ViInt32 deviceIndex, ViInt32* devTypeP);



//! Returns some basic data about a specified instrument.
/*! Values returned by the function:

    'name'            = pointer to user-allocated string, into which the
                        model name is returned (length < 32 characters).
    'serialNbr'       = serial number of the instrument.
    'busNbr'          = bus number where the instrument is located.
    'slotNbr'         = slot number where the instrument is located.

    Returns one of the following ViStatus values:
    VI_SUCCESS                         always. */
ACQ_DLL ViStatus ACQ_CC Acqrs_getInstrumentData(ViSession instrumentID,
    ViChar name[], ViInt32* serialNbr, ViInt32* busNbrP, ViInt32* slotNbrP);



//! Returns some basic data about a specified instrument.
/*! Values returned by the function:

    'modelNumberP'      = pointer to user-allocated string, into which the
                          model number is returned (length < 32 characters)
    'modelNumberSize'   = size of the 'modelNumber' buffer.
    'serialP'           = pointer to user-allocated string, into which the 
                          serial number of the instrument is returned (length < 32 characters).
    'serialSize'        = size of the 'serial' buffer.
    'busNbrP'           = bus number where the instrument is located
    'slotNbrP'          = slot number where the instrument is located
    'crateNbrP'         = crate where the instrument is located.
    'posInCrateP'       = position in the crate where the instrument is located.

    Returns one of the following ViStatus values:
    VI_SUCCESS                         always. */
ACQ_DLL ViStatus ACQ_CC Acqrs_getInstrumentDataEx(ViSession instrumentID,
    ViChar modelNumberP[], ViUInt32 modelNumberSize, ViChar serialP[], ViUInt32 serialSize, 
    ViUInt32* busNbrP, ViUInt32* slotNbrP, ViUInt32* crateNbrP, ViUInt32* posInCrateP);



//! Returns general information about a specified instrument.
/*! The following value must be supplied to the function:

    'parameterString'  = character string specifying the requested parameter
                         Please refer to the manual for the accepted parameter strings

    Value returned by the function:

    'infoValue'        = value of requested parameter
                         The type of the returned value depends on the parameter string
                         Please refer to the manual for the required  data type as a
                         function of the accepted parameter strings
                         NOTE to C/C++ programmers: 'ViAddr' resolves to 'void*'

    Returns one of the following ViStatus values:
    ACQIRIS_ERROR_PARAM_STRING_INVALID if 'parameterString' is invalid.
    ACQIRIS_ERROR_NOT_SUPPORTED        if the requested value is not supported by the 
                                       instrument.
    VI_SUCCESS                         otherwise. */
ACQ_DLL ViStatus ACQ_CC Acqrs_getInstrumentInfo(ViSession instrumentID, 
    ViConstString parameterString, ViAddr infoValue);



//! Returns the number of channels on the specified instrument.
/*! Returns one of the following ViStatus values:
    VI_SUCCESS                         always. */
ACQ_DLL ViStatus ACQ_CC Acqrs_getNbrChannels(ViSession instrumentID, ViInt32* nbrChannels);



//! Returns the number of supported physical Agilent_Technologies devices found on the computer.
/*! Returns one of the following ViStatus values:
    VI_SUCCESS                         always. */
ACQ_DLL ViStatus ACQ_CC Acqrs_getNbrInstruments(ViInt32* nbrInstruments);



//! Returns version numbers associated with a specified instrument / current device driver.
/*! The following value must be supplied to the function:

    'versionItem'    =   1        for version of Kernel-Mode Driver 
                         2        for version of EEPROM Common Section 
                         3        for version of EEPROM Digitizer Section
                         4        for version of CPLD firmware
 
    Value returned by the function:

    'version'        = version number.

    For drivers, the version number is composed of 2 parts. The upper 2 bytes represent
    the major version number, and the lower 2 bytes represent the minor version number. 

    Returns one of the following ViStatus values:
    VI_SUCCESS                         always. */
ACQ_DLL ViStatus ACQ_CC Acqrs_getVersion(ViSession instrumentID,
    ViInt32 versionItem, ViInt32* version);



//! Initializes an instrument.
/*! See remarks under 'Acqrs_InitWithOptions' */
ACQ_DLL ViStatus ACQ_CC Acqrs_init(ViRsrc resourceName, ViBoolean IDQuery, 
    ViBoolean resetDevice, ViSession* instrumentID);
 


//! Initializes an instrument with options.
/*! The following values must be supplied to the function:

    'resourceName'   = an ASCII string which identifies the instrument to be initialized
                       See manual for a detailed description of this string.
    'IDQuery'        = currently ignored
    'resetDevice'    = if set to 'TRUE', resets the instrument after initialization
    'optionsString'  = an ASCII string which specifies options. Currently, we support
                       "CAL=False" to suppress calibration during the initialization
                       "DMA=False" to inhibit (for diagnostics) the use of scatter-gather DMA for 
                       data transfers
                       "Simulate=True" for the use of simulated instruments during application 
                       development. 
    Values returned by the function:

    'instrumentID'   = identifier for subsequent use in the other function calls.

    Returns one of the following ViStatus values:
    ACQIRIS_ERROR_INIT_STRING_INVALID        if the 'resourceName' is invalid.
    ACQIRIS_ERROR_INTERNAL_DEVICENO_INVALID  if the 'resourceName' contains values that are not 
                                             within the expected ranges (e.g. wrong serial number).
    ACQIRIS_ERROR_TOO_MANY_DEVICES           if there are too many devices installed.
    ACQIRIS_ERROR_KERNEL_VERSION             if the instrument require a newer kernel driver.
    ACQIRIS_ERROR_HW_FAILURE                 if the instrument doesn't answer properly to 
                                             defined requests.
    ACQIRIS_ERROR_HW_FAILURE_CHx             if one of the channels doesn't answer properly to 
                                             defined requests, where 'x' = channel number.
    ACQIRIS_ERROR_FILE_NOT_FOUND             if a required FPGA file could not be found.
    ACQIRIS_ERROR_NO_DATA                    if a required FPGA file did not contain expected data.
    VI_SUCCESS                               otherwise. */
ACQ_DLL ViStatus ACQ_CC Acqrs_InitWithOptions(ViRsrc resourceName, ViBoolean IDQuery, 
    ViBoolean resetDevice, ViConstString optionsString, ViSession* instrumentID);



//! Reads/writes data values from programmable logic devices.
/*! Reads/writes a number of 32-bit data values from/to a user-defined register in the
    logic device identified by 'deviceName[]'. 
    ONLY useful for a instrument with on-board programmable logic devices.
    Currently ONLY for SCxxx and ACxxx!

    The following values must be supplied to the function:

    'deviceName'       = an ASCII string which identifies the device
                         Must be of the form "BlockMDevN", where M = 1..4 and N = 1..number
                         of logical devices in the device block M.
                         In the AC/SC Analyzers & the RC200, this string must be "Block1Dev1"
                         See manual for a detailed description of this string.
    'registerID'       = 0..to (nbrRegisters-1)
    'nbrValues'        = number of 32-bit values to be transferred
    'dataArray[]'      = user-supplied buffer for data values
    'readWrite'        = 0 for reading (from logic device to application)
                         1 for writing (from application to logic device)
    'flags'            : bit31=1 forces not to use DMA transfer to FPGA

    Returns one of the following ViStatus values:
    ACQIRIS_ERROR_PARAM_STRING_INVALID   if the 'deviceName' is not valid.
    ACQIRIS_ERROR_NO_ACCESS              if the operation is not authorized.
    ACQIRIS_ERROR_NOT_SUPPORTED          if the operation is not supported by the instrument,  
                                             or if 'registerID' is outside the expected values.
    ACQIRIS_ERROR_INSTRUMENT_RUNNING     if the instrument was not stopped beforehand.
    ACQIRIS_ERROR_DATA_ARRAY             if 'dataArray' is NULL.
    ACQIRIS_ERROR_IO_WRITE               if a 'write' verify failed.
    VI_SUCCESS                           otherwise. */
ACQ_DLL ViStatus ACQ_CC Acqrs_logicDeviceIO(ViSession instrumentID,
    ViChar deviceName[],        ViInt32 registerID,           ViInt32 nbrValues,
    ViInt32 dataArray[],        ViInt32 readWrite,            ViInt32 flags);



//! Forces all instruments to prepare entry into or return from the system power down state.
/*! Typically, this function is called by a 'Power Aware' application, 
    when it catches a 'system power down' event, such as 'hibernate'. 

    If 'state == 0' (AqPowerOff), it will suspend all other calling threads. If a thread
    is performing a long operation which cannot be completed within milliseconds, 
    such as 'calibrate', it will be interrupted immediately and will get the status 
    'ACQIRIS_ERROR_OPERATION_CANCELLED'. Note that if an acquisition is still running
    when 'powerSystem(0, 0)' is called, it might be incomplete or corrupted.
   
    If 'state == 1' (AqPowerOn), it will re-enable the instruments in the same state as they
    were before 'powerSystem(0, 0)'. Threads which were suspended will be resumed.
    However, interrupted operations which returned an error 
    'ACQIRIS_ERROR_OPERATION_CANCELLED' have to be redone.

    The following values must be supplied to the function:

   'state'          = 0 (=AqPowerOff) : prepare for power down.
                    = 1 (=AqPowerOn)  : re-enable instruments after power down.

   'flags'          = Unused, must be 0.

   Returns one of the following ViStatus values:
   VI_SUCCESS                           always. */
ACQ_DLL ViStatus ACQ_CC Acqrs_powerSystem(ViInt32 state, ViInt32 flags);



//! Resets an instrument.
/*! Returns one of the following ViStatus values:
   VI_SUCCESS                           always. */
ACQ_DLL ViStatus ACQ_CC Acqrs_reset(ViSession instrumentID);


//! Resets the device memory to a known default state. 
/*! Returns one of the following ViStatus values:
    ACQIRIS_ERROR_NOT_SUPPORTED         if the instrument does not support this feature. 
    ACQIRIS_ERROR_INSTRUMENT_RUNNING    if the instrument is already running. 
    VI_SUCCESS                          otherwise. */
ACQ_DLL ViStatus ACQ_CC Acqrs_resetMemory(ViSession instrumentID);



//! Resume control of an instrument.
/*! This function reacquires the driver lock of the instrument and allows calls to it from 
    the current process (Windows only). After successfully calling 'Acqrs_resumeControl', the module will be
    set to a default hardware state. It will have no valid data and the timestamp will be set
    to 0. When the next acquisition is started, the module will be configured with all of the 
    unmodified settings from before the 'Acqrs_suspendControl' was invoked.
   
    Returns one of the following ViStatus values:
    ACQIRIS_ERROR_DEVICE_ALREADY_OPEN       if the instrument is already used by another process
    VI_SUCCESS                              otherwise. */
ACQ_DLL ViStatus ACQ_CC Acqrs_resumeControl(ViSession instrumentID);



//! Set through 'value' the value of the attribute named 'name'.
/*! 'channel'            = 1...Nchan (as returned by 'Acqrs_getNbrChannels' ) or
                           0 if it is an attribute related to the instrument 
                           itself (i.e. not to a channel).
    'name'               = specify the name of the attribute to change.
                           Please refer to the manual for the accepted names.
    'value'              = specify the value in which the attribute will be set.
                           Please refer to the manual for the accepted values.

    Returns one of the following ViStatus values:
    ACQIRIS_ERROR_ATTR_NOT_FOUND         if not found or if a wrong 'channel' is specified. 
    ACQIRIS_ERROR_ATTR_WRONG_TYPE        if found but not of the expected type.
    ACQIRIS_ERROR_ATTR_INVALID_VALUE     if 'value' is not valid.
    ACQIRIS_ERROR_ATTR_IS_READ_ONLY      if found but not writable.
    VI_SUCCESS                           otherwise. */
ACQ_DLL ViStatus ACQ_CC Acqrs_setAttributeString(ViSession instrumentID, ViInt32 channel, 
    ViConstString name, ViConstString value); 



//! Sets the front-panel LED to the desired color.
/*! 'color' = 0        OFF (returns to normal 'acquisition status' indicator)
              1        Green
              2        Red
              3        Yellow

    Returns one of the following ViStatus values:
   VI_SUCCESS                           always. */
ACQ_DLL ViStatus ACQ_CC Acqrs_setLEDColor(ViSession instrumentID, ViInt32 color);



//! Set simulation options.
/*! Sets one or several options which will be used by the function 'Acqrs_InitWithOptions',
    provided that the 'optionString' supplied to 'Acqrs_InitWithOptions' contains the
    string 'simulate=TRUE' (or similar).
    Refer to the manual for the accepted form of 'simOptionString'.
    The simulation options are reset to none by setting 'simOptionString' to an empty string "".

   Returns one of the following ViStatus values:
   VI_SUCCESS                           always. */
ACQ_DLL ViStatus ACQ_CC Acqrs_setSimulationOptions(ViConstString simOptionString);



//! Supend Control of an instrument.
/*! This function releases the driver lock of the instrument and prevents all further calls
    from the current process (Windows only). Use Acqrs_resumeControl to reacquire the 
    control of the instrument. Once suspended, this instrument can be used from another process.
    However, if this is the first time this other process is used, all desired acquisition settings
    must be defined and a calibration will be needed.

    Returns one of the following ViStatus values:
   VI_SUCCESS                           always. */
ACQ_DLL ViStatus ACQ_CC Acqrs_suspendControl(ViSession instrumentID);


