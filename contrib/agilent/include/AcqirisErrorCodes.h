#ifndef _ACQIRISERRORCODES_H
#define _ACQIRISERRORCODES_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  AcqirisErrorCodes.h:    Agilent Acqiris Error Codes
//
//----------------------------------------------------------------------------------------
//  Copyright (C) Agilent Technologies, Inc. 2000, 2001-2009
//
//////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
//  Acqiris Error Codes Bases and Ranges
//////////////////////////////////////////////////////////////////////////////////////////

#define ACQIRIS_ERROR      ( 0x3FFA4000 - 0x40000000 - 0x40000000 )   // 0xBFFA4000
#define ACQIRIS_WARNING      0x3FFA4000

#define ACQIRIS_MAX_SPECIFIC_ERROR_CODE             (ACQIRIS_ERROR + 0xfff)
#define ACQIRIS_MAX_SPECIFIC_WARN_CODE              (ACQIRIS_WARNING + 0xfff)


//////////////////////////////////////////////////////////////////////////////////////////
//  Instrument Specific Error Codes
//////////////////////////////////////////////////////////////////////////////////////////

#define ACQIRIS_ERROR_FILE_NOT_FOUND                (ACQIRIS_ERROR + 0x800)
#define ACQIRIS_ERROR_PATH_NOT_FOUND                (ACQIRIS_ERROR + 0x801)
#define ACQIRIS_ERROR_INVALID_CHARS_IN_PATH         (ACQIRIS_ERROR + 0x802)
#define ACQIRIS_ERROR_INVALID_HANDLE                (ACQIRIS_ERROR + 0x803)
#define ACQIRIS_ERROR_NOT_SUPPORTED                 (ACQIRIS_ERROR + 0x805)
#define ACQIRIS_ERROR_INVALID_WINDOWS_PARAM         (ACQIRIS_ERROR + 0x806)
#define ACQIRIS_ERROR_NO_DATA                       (ACQIRIS_ERROR + 0x807)
#define ACQIRIS_ERROR_NO_ACCESS                     (ACQIRIS_ERROR + 0x808)
#define ACQIRIS_ERROR_BUFFER_OVERFLOW               (ACQIRIS_ERROR + 0x809)
#define ACQIRIS_ERROR_BUFFER_NOT_64BITS_ALIGNED     (ACQIRIS_ERROR + 0x80a)
#define ACQIRIS_ERROR_BUFFER_NOT_32BITS_ALIGNED     (ACQIRIS_ERROR + 0x80b)
#define ACQIRIS_ERROR_CAL_FILE_CORRUPTED            (ACQIRIS_ERROR + 0x80c)
#define ACQIRIS_ERROR_CAL_FILE_VERSION              (ACQIRIS_ERROR + 0x80d)
#define ACQIRIS_ERROR_CAL_FILE_SERIAL               (ACQIRIS_ERROR + 0x80e)
#define ACQIRIS_ERROR_CAL_BUFFER_OUT_OF_BOUNDS      (ACQIRIS_ERROR + 0x80f)

#define ACQIRIS_ERROR_ALREADY_OPEN                  (ACQIRIS_ERROR + 0x840)
#define ACQIRIS_ERROR_SETUP_NOT_AVAILABLE           (ACQIRIS_ERROR + 0x880)

#define ACQIRIS_ERROR_IO_WRITE                      (ACQIRIS_ERROR + 0x8a0)
#define ACQIRIS_ERROR_IO_READ                       (ACQIRIS_ERROR + 0x8a1)
#define ACQIRIS_ERROR_IO_DEVICE_OFF                 (ACQIRIS_ERROR + 0x8a2)
#define ACQIRIS_ERROR_IO_VME_CONFIG                 (ACQIRIS_ERROR + 0x8a3)
#define ACQIRIS_ERROR_IO_VME_ACCESS                 (ACQIRIS_ERROR + 0x8a4)

#define ACQIRIS_ERROR_INTERNAL_DEVICENO_INVALID     (ACQIRIS_ERROR + 0x8c0)
#define ACQIRIS_ERROR_TOO_MANY_DEVICES              (ACQIRIS_ERROR + 0x8c1)
#define ACQIRIS_ERROR_EEPROM_DATA_INVALID           (ACQIRIS_ERROR + 0x8c2)
#define ACQIRIS_ERROR_INIT_STRING_INVALID           (ACQIRIS_ERROR + 0x8c3)
#define ACQIRIS_ERROR_INSTRUMENT_NOT_FOUND          (ACQIRIS_ERROR + 0x8c4)
#define ACQIRIS_ERROR_INSTRUMENT_RUNNING            (ACQIRIS_ERROR + 0x8c5)
#define ACQIRIS_ERROR_INSTRUMENT_STOPPED            (ACQIRIS_ERROR + 0x8c6)
#define ACQIRIS_ERROR_MODULES_NOT_ON_SAME_BUS       (ACQIRIS_ERROR + 0x8c7)
#define ACQIRIS_ERROR_NOT_ENOUGH_DEVICES            (ACQIRIS_ERROR + 0x8c8)
#define ACQIRIS_ERROR_NO_MASTER_DEVICE              (ACQIRIS_ERROR + 0x8c9)
#define ACQIRIS_ERROR_PARAM_STRING_INVALID          (ACQIRIS_ERROR + 0x8ca)
#define ACQIRIS_ERROR_COULD_NOT_CALIBRATE           (ACQIRIS_ERROR + 0x8cb)
#define ACQIRIS_ERROR_CANNOT_READ_THIS_CHANNEL      (ACQIRIS_ERROR + 0x8cc)
#define ACQIRIS_ERROR_PRETRIGGER_STILL_RUNNING      (ACQIRIS_ERROR + 0x8cd)
#define ACQIRIS_ERROR_CALIBRATION_FAILED            (ACQIRIS_ERROR + 0x8ce)
#define ACQIRIS_ERROR_MODULES_NOT_CONTIGUOUS        (ACQIRIS_ERROR + 0x8cf)
#define ACQIRIS_ERROR_INSTRUMENT_ACQ_LOCKED         (ACQIRIS_ERROR + 0x8d0)
#define ACQIRIS_ERROR_INSTRUMENT_ACQ_NOT_LOCKED     (ACQIRIS_ERROR + 0x8d1)
#define ACQIRIS_ERROR_EEPROM2_DATA_INVALID          (ACQIRIS_ERROR + 0x8d2)
#define ACQIRIS_ERROR_INSTRUMENT_IN_USE             (ACQIRIS_ERROR + 0x8d3)
#define ACQIRIS_ERROR_MEZZIO_IN_USE                 (ACQIRIS_ERROR + 0x8d4)
#define ACQIRIS_ERROR_MEZZIO_ACQ_TIMEOUT            (ACQIRIS_ERROR + 0x8d5)
#define ACQIRIS_ERROR_DEVICE_ALREADY_OPEN           (ACQIRIS_ERROR + 0x8d6)
#define ACQIRIS_ERROR_EEPROM_CRC_FAILED             (ACQIRIS_ERROR + 0x8d7)
#define ACQIRIS_ERROR_INSTRUMENT_SUSPENDED          (ACQIRIS_ERROR + 0x8d8)
#define ACQIRIS_ERROR_INCOMPATIBLE_MODE             (ACQIRIS_ERROR + 0x8d9)

#define ACQIRIS_ERROR_INVALID_GEOMAP_FILE           (ACQIRIS_ERROR + 0x8e0)

#define ACQIRIS_ERROR_ACQ_TIMEOUT                   (ACQIRIS_ERROR + 0x900)
#define ACQIRIS_ERROR_TIMEOUT                       ACQIRIS_ERROR_ACQ_TIMEOUT    // For backwards compatibility
#define ACQIRIS_ERROR_OVERLOAD                      (ACQIRIS_ERROR + 0x901)
#define ACQIRIS_ERROR_PROC_TIMEOUT                  (ACQIRIS_ERROR + 0x902)
#define ACQIRIS_ERROR_LOAD_TIMEOUT                  (ACQIRIS_ERROR + 0x903)
#define ACQIRIS_ERROR_READ_TIMEOUT                  (ACQIRIS_ERROR + 0x904)
#define ACQIRIS_ERROR_INTERRUPTED                   (ACQIRIS_ERROR + 0x905)
#define ACQIRIS_ERROR_WAIT_TIMEOUT                  (ACQIRIS_ERROR + 0x906)
#define ACQIRIS_ERROR_CLOCK_SOURCE                  (ACQIRIS_ERROR + 0x907)
#define ACQIRIS_ERROR_OPERATION_CANCELLED           (ACQIRIS_ERROR + 0x908)

#define ACQIRIS_ERROR_FIRMWARE_NOT_AUTHORIZED       (ACQIRIS_ERROR + 0xa00)
#define ACQIRIS_ERROR_FPGA_1_LOAD                   (ACQIRIS_ERROR + 0xa01)
#define ACQIRIS_ERROR_FPGA_2_LOAD                   (ACQIRIS_ERROR + 0xa02)
#define ACQIRIS_ERROR_FPGA_3_LOAD                   (ACQIRIS_ERROR + 0xa03)
#define ACQIRIS_ERROR_FPGA_4_LOAD                   (ACQIRIS_ERROR + 0xa04)
#define ACQIRIS_ERROR_FPGA_5_LOAD                   (ACQIRIS_ERROR + 0xa05)
#define ACQIRIS_ERROR_FPGA_6_LOAD                   (ACQIRIS_ERROR + 0xa06)
#define ACQIRIS_ERROR_FPGA_7_LOAD                   (ACQIRIS_ERROR + 0xa07)
#define ACQIRIS_ERROR_FPGA_8_LOAD                   (ACQIRIS_ERROR + 0xa08)
#define ACQIRIS_ERROR_FIRMWARE_NOT_SUPPORTED        (ACQIRIS_ERROR + 0xa09)

#define ACQIRIS_ERROR_FPGA_1_FLASHLOAD_NO_INIT      (ACQIRIS_ERROR + 0xa10)
#define ACQIRIS_ERROR_FPGA_1_FLASHLOAD_NO_DONE      (ACQIRIS_ERROR + 0xa11)
#define ACQIRIS_ERROR_FPGA_2_FLASHLOAD_NO_INIT      (ACQIRIS_ERROR + 0xa12)
#define ACQIRIS_ERROR_FPGA_2_FLASHLOAD_NO_DONE      (ACQIRIS_ERROR + 0xa13)

#define ACQIRIS_ERROR_SELFCHECK_MEMORY              (ACQIRIS_ERROR + 0xa20)
#define ACQIRIS_ERROR_SELFCHECK_DAC                 (ACQIRIS_ERROR + 0xa21)
#define ACQIRIS_ERROR_SELFCHECK_RAMP                (ACQIRIS_ERROR + 0xa22)
#define ACQIRIS_ERROR_SELFCHECK_PCIE_LINK           (ACQIRIS_ERROR + 0xa23)
#define ACQIRIS_ERROR_SELFCHECK_PCIE_DEVICE         (ACQIRIS_ERROR + 0xa24)

#define ACQIRIS_ERROR_FLASH_ACCESS_TIMEOUT          (ACQIRIS_ERROR + 0xa30)
#define ACQIRIS_ERROR_FLASH_FAILURE                 (ACQIRIS_ERROR + 0xa31)
#define ACQIRIS_ERROR_FLASH_READ                    (ACQIRIS_ERROR + 0xa32)
#define ACQIRIS_ERROR_FLASH_WRITE                   (ACQIRIS_ERROR + 0xa33)
#define ACQIRIS_ERROR_FLASH_EMPTY                   (ACQIRIS_ERROR + 0xa34)

#define ACQIRIS_ERROR_ATTR_NOT_FOUND                (ACQIRIS_ERROR + 0xb00)
#define ACQIRIS_ERROR_ATTR_WRONG_TYPE               (ACQIRIS_ERROR + 0xb01)
#define ACQIRIS_ERROR_ATTR_IS_READ_ONLY             (ACQIRIS_ERROR + 0xb02)
#define ACQIRIS_ERROR_ATTR_IS_WRITE_ONLY            (ACQIRIS_ERROR + 0xb03)
#define ACQIRIS_ERROR_ATTR_ALREADY_DEFINED          (ACQIRIS_ERROR + 0xb04)
#define ACQIRIS_ERROR_ATTR_IS_LOCKED                (ACQIRIS_ERROR + 0xb05)
#define ACQIRIS_ERROR_ATTR_INVALID_VALUE            (ACQIRIS_ERROR + 0xb06)
#define ACQIRIS_ERROR_ATTR_CALLBACK_STATUS          (ACQIRIS_ERROR + 0xb07)
#define ACQIRIS_ERROR_ATTR_CALLBACK_EXCEPTION       (ACQIRIS_ERROR + 0xb08)

#define ACQIRIS_ERROR_KERNEL_VERSION                (ACQIRIS_ERROR + 0xc00)
#define ACQIRIS_ERROR_UNKNOWN_ERROR                 (ACQIRIS_ERROR + 0xc01)
#define ACQIRIS_ERROR_OTHER_WINDOWS_ERROR           (ACQIRIS_ERROR + 0xc02)
#define ACQIRIS_ERROR_VISA_DLL_NOT_FOUND            (ACQIRIS_ERROR + 0xc03)
#define ACQIRIS_ERROR_OUT_OF_MEMORY                 (ACQIRIS_ERROR + 0xc04)
#define ACQIRIS_ERROR_UNSUPPORTED_DEVICE            (ACQIRIS_ERROR + 0xc05)

#define ACQIRIS_ERROR_VME_SBS_DLL_LOAD              (ACQIRIS_ERROR + 0xc10)

#define ACQIRIS_ERROR_PARAMETER9                    (ACQIRIS_ERROR + 0xd09)
#define ACQIRIS_ERROR_PARAMETER10                   (ACQIRIS_ERROR + 0xd0a)
#define ACQIRIS_ERROR_PARAMETER11                   (ACQIRIS_ERROR + 0xd0b)
#define ACQIRIS_ERROR_PARAMETER12                   (ACQIRIS_ERROR + 0xd0c)
#define ACQIRIS_ERROR_PARAMETER13                   (ACQIRIS_ERROR + 0xd0d)
#define ACQIRIS_ERROR_PARAMETER14                   (ACQIRIS_ERROR + 0xd0e)
#define ACQIRIS_ERROR_PARAMETER15                   (ACQIRIS_ERROR + 0xd0f)

#define ACQIRIS_ERROR_NBR_SEG                       (ACQIRIS_ERROR + 0xd10)
#define ACQIRIS_ERROR_NBR_SAMPLE                    (ACQIRIS_ERROR + 0xd11)
#define ACQIRIS_ERROR_DATA_ARRAY                    (ACQIRIS_ERROR + 0xd12)
#define ACQIRIS_ERROR_SEG_DESC_ARRAY                (ACQIRIS_ERROR + 0xd13)
#define ACQIRIS_ERROR_FIRST_SEG                     (ACQIRIS_ERROR + 0xd14)
#define ACQIRIS_ERROR_SEG_OFF                       (ACQIRIS_ERROR + 0xd15)
#define ACQIRIS_ERROR_FIRST_SAMPLE                  (ACQIRIS_ERROR + 0xd16)
#define ACQIRIS_ERROR_DATATYPE                      (ACQIRIS_ERROR + 0xd17)
#define ACQIRIS_ERROR_READMODE                      (ACQIRIS_ERROR + 0xd18)

#define ACQIRIS_ERROR_VM_FILE_EXTENSION             (ACQIRIS_ERROR + 0xd50)
#define ACQIRIS_ERROR_VM_FILE_VERSION               (ACQIRIS_ERROR + 0xd51)
#define ACQIRIS_ERROR_VM_FILE_READ                  (ACQIRIS_ERROR + 0xd52)
#define ACQIRIS_ERROR_VM_FILE_INVALID               (ACQIRIS_ERROR + 0xd53)
#define ACQIRIS_ERROR_VM_VERIFICATION               (ACQIRIS_ERROR + 0xd54)
#define ACQIRIS_ERROR_VM_CRC                        (ACQIRIS_ERROR + 0xd55)

#define ACQIRIS_ERROR_HW_FAILURE                    (ACQIRIS_ERROR + 0xd80)
#define ACQIRIS_ERROR_HW_FAILURE_CH1                (ACQIRIS_ERROR + 0xd81)
#define ACQIRIS_ERROR_HW_FAILURE_CH2                (ACQIRIS_ERROR + 0xd82)
#define ACQIRIS_ERROR_HW_FAILURE_CH3                (ACQIRIS_ERROR + 0xd83)
#define ACQIRIS_ERROR_HW_FAILURE_CH4                (ACQIRIS_ERROR + 0xd84)
#define ACQIRIS_ERROR_HW_FAILURE_CH5                (ACQIRIS_ERROR + 0xd85)
#define ACQIRIS_ERROR_HW_FAILURE_CH6                (ACQIRIS_ERROR + 0xd86)
#define ACQIRIS_ERROR_HW_FAILURE_CH7                (ACQIRIS_ERROR + 0xd87)
#define ACQIRIS_ERROR_HW_FAILURE_CH8                (ACQIRIS_ERROR + 0xd88)
#define ACQIRIS_ERROR_HW_FAILURE_EXT1               (ACQIRIS_ERROR + 0xda0)

#define ACQIRIS_ERROR_MAC_T0_ADJUSTMENT             (ACQIRIS_ERROR + 0xdc0)
#define ACQIRIS_ERROR_MAC_ADC_ADJUSTMENT            (ACQIRIS_ERROR + 0xdc1)
#define ACQIRIS_ERROR_MAC_RESYNC_ADJUSTMENT         (ACQIRIS_ERROR + 0xdc2)


#define ACQIRIS_WARN_SETUP_ADAPTED                  (ACQIRIS_WARNING + 0xe00)
#define ACQIRIS_WARN_READPARA_NBRSEG_ADAPTED        (ACQIRIS_WARNING + 0xe10)
#define ACQIRIS_WARN_READPARA_NBRSAMP_ADAPTED       (ACQIRIS_WARNING + 0xe11)
#define ACQIRIS_WARN_NOT_CALIBRATED                 (ACQIRIS_WARNING + 0xe12)
#define ACQIRIS_WARN_ACTUAL_DATASIZE_ADAPTED        (ACQIRIS_WARNING + 0xe13)
#define ACQIRIS_WARN_UNEXPECTED_TRIGGER             (ACQIRIS_WARNING + 0xe14)
#define ACQIRIS_WARN_READPARA_FLAGS_ADAPTED         (ACQIRIS_WARNING + 0xe15)
#define ACQIRIS_WARN_SIMOPTION_STRING_UNKNOWN       (ACQIRIS_WARNING + 0xe16)
#define ACQIRIS_WARN_INSTRUMENT_IN_USE              (ACQIRIS_WARNING + 0xe17)

#define ACQIRIS_WARN_HARDWARE_TIMEOUT               (ACQIRIS_WARNING + 0xe60)
#define ACQIRIS_WARN_RESET_IGNORED                  (ACQIRIS_WARNING + 0xe61)

#define ACQIRIS_WARN_SELFCHECK_MEMORY               (ACQIRIS_WARNING + 0xf00)
#define ACQIRIS_WARN_CLOCK_SOURCE                   (ACQIRIS_WARNING + 0xf01)

#define ACQIRIS_WARN_NUMERIC_OVERFLOW               (ACQIRIS_WARNING + 0xf20)

#endif // sentry

