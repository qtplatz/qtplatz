#ifndef _DDRIORULES_H
#define _DDRIORULES_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  DDrIORules.h        Control Codes for Kernel-Mode TR Device Driver
//
//----------------------------------------------------------------------------------------
//  Copyright (C) 1998, 1999-2010 Agilent Technologies, Inc.
//
//  Started:    10 Dec 1997
//  By:         V. Hungerbuhler
//
//  Purpose:    Declaration of Structure for IO Resource Allocation
//              Declaration of IO Control Codes for 'DeviceIoControl' interface
//
//  Comments:   This file should only contain declarations which are needed by the
//              user-mode device driver(s) AND the kernel-mode drivers.
//              Any declarations exclusively used in the kernel-mode drivers should
//              be made in their own include files.
//              NOTE: This file is the ONLY declaration file which may be included both
//              by user-mode and kernel-mode files. It is the only 'bridge' between them.
//
//  Warning:    As explained above, this file is used to compile both user-mode code
//              and kernel-mode code. Thus it must not include any C header.
//
//////////////////////////////////////////////////////////////////////////////////////////

#if defined(_MSC_VER) && _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////
// Windows defines
#ifdef WIN32
typedef unsigned __int64 u64;
typedef unsigned long u32;
typedef signed long s32;
#endif //WIN32


/////////////////////////////////////
// Linux defines
#ifdef __linux
#include <linux/types.h>
//typedef unsigned long long u64;
//#if __i386 || __LONG_MAX__ == 2147483647L || __SIZEOF_LONG__ == 4 || __GNUC__ == 2
//typedef unsigned long u32;
//typedef signed long s32;
//#elif __amd64 || __LONG_MAX__ == 9223372036854775807L || __SIZEOF_LONG__ == 8
//typedef unsigned int u32;
//typedef signed int s32;
//#else
//#error "Unknown size of long for gcc. Cannot define 32 bit integer type."
//#endif
#endif // __linux


/////////////////////////////////////
// VxWorks defines
#ifdef __vxworks
#include <vxWorks.h>
#define HANDLE HANDLEVXW

typedef unsigned long long u64;
typedef unsigned long u32;
typedef signed long s32;
typedef u32 HANDLE;

// Note: VxW6 compilator has already defined __int64
#endif    // __vxworks



/////////////////////////////////////
// Common defines
#define DDR_ACQIRIS_VENDORID            0x14ec
#define DDR_ACQIRIS_VENDORID_STRING     "VEN_14EC"

#define DDR_ADLINK_VENDORID             0x144A
#define DDR_ADLINK_VENDORID_STRING      "VEN_144A"

#define DDR_PLX_VENDORID                0x10B5
#define DDR_MAX_BUSSES                  64              //<! max. # of busses on which to search

// The number of required pages MUST be < 256, because the first or last pages might
// be partially filled   (transfers with IOCTL_READ_DMA or IOCTL_WRITE_DMA)
#define DDR_MAX_DMABLOCK                255*1024        //<! max. # longs transferred in a single DMA

#define DDR_TIMEOUT_MARKER              0x8000          //<! bit 15 in returned interrupt status register
#define DDR_ACQEND_INTERRUPT            0x02000000      //<! interrupt event pattern for 'end-of-acquisition'
#define DDR_PROCEND_INTERRUPT           0x04000000      //<! interrupt event pattern for 'end-of-processing'


//////////////////////////////////////////////////////////////////////////////////////////
// Resource descriptor
//////////////////////////////////////////////////////////////////////////////////////////
#define DDR_NAME_SIZE            50            //<! buffer size for ASCII names in DDrResources


// NOTE: we need to declare structures with 'typedef' because it is also used by
//       pure C functions!!!


//! Resource descriptor for Acqiris devices inside the kernel driver.
/*! This descriptor is used all along the kernel driver. For the user driver, this descriptor is
    used conditionnaly, depending if we are in a 32 bits compilation or 64 bits.

    Note: For backwards compatibility with 32 bits dll, this structure might be assigned to a
    'DDrResources_x32' in 'IOCTL_GET_RESOURCES'.

    IMPORTANT:  We should avoid to EVER change the size of 'DDrResources_xx'. Doing so introduces
                a potential incompatibility between old/new kernel-mode drivers and old/new
                user-mode drivers. */
typedef struct
{
    u64 controlAddr;          //<! Physical address of memory-mapped iface control regs
    u64 controlBase;          //<! Base address of memory-mapped interface control registers
    u64 controlSize;          //<! Address space (in bytes) of the control registers
    u64 directAddr;           //<! Physical address of memory-mapped direct module space
    u64 directBase;           //<! Base address of memory-mapped direct module space
    u64 directSize;           //<! Address space (in bytes) of the direct module space

    s32 busNumber;            //<! Physical bus # of this device
    s32 devNumber;            //<! Physical device # of this device (= slot number in bus)

    s32 interrupt;            //<! Interrupt Number
    s32 IRQHandle;            //<! IRQ Handle returned by Interrupt-Hookup routine
    u64 intrptLocalAddr;      //<! Local address of device interrupt register
    s32 alarmMaskPattern;     //<! Alarm mask            (i.e. alarms that are enabled)
    s32 alarmEvntPattern;     //<! Alarm-event record    (i.e. alarms that occurred)

    char name[DDR_NAME_SIZE]; //<! ID of this device (Windows 95/98 ONLY?)

} DDrResources_x64;


//! Resource descriptor for Acqiris devices on 32Bits OS.
/*! IMPORTANT:  We should avoid to EVER change the size of 'DDrResources_xx'. Doing so introduces
                a potential incompatibility between old/new kernel-mode drivers and old/new
                user-mode drivers. */
typedef struct
{
    u32 controlAddr;          //<! Physical address of memory-mapped iface control regs
    u32 controlBase;          //<! Base address of memory-mapped interface control registers
    u32 controlSize;          //<! Address space (in bytes) of the control registers
    u32 directAddr;           //<! Physical address of memory-mapped direct module space
    u32 directBase;           //<! Base address of memory-mapped direct module space
    u32 directSize;           //<! Address space (in bytes) of the direct module space

    s32 busNumber;            //<! Physical bus # of this device
    s32 devNumber;            //<! Physical device # of this device (= slot number in bus)

    s32 interrupt;            //<! Interrupt Number
    s32 IRQHandle;            //<! IRQ Handle returned by Interrupt-Hookup routine
    u32 intrptLocalAddr;      //<! Local address of device interrupt register
    s32 alarmMaskPattern;     //<! Alarm mask            (i.e. alarms that are enabled)
    s32 alarmEvntPattern;     //<! Alarm-event record    (i.e. alarms that occurred)

    char name[DDR_NAME_SIZE]; //<! ID of this device (Windows 95/98 ONLY?)

} DDrResources_x32;


#ifdef __cplusplus // For the USER MODE ONLY !


#ifdef __linux // the structures do not have the same sizes on Windows
static_assert(sizeof(DDrResources_x32) == 104);
static_assert(sizeof(DDrResources_x64) == 132 || sizeof(DDrResources_x64) == 136);
#endif


#if defined(_WIN64) || defined(_LP64)

// Only expected with 64 bits compilations.
static_assert(sizeof(size_t) == 8);

// If we compile 32 bits, we have the resource 32 bits above. But if we compile 64 bits, we
// use the 64bits one directly. To be simpler, we could use the 64 bits 'DDrResources_x64' always, but
// it would brake the backwards compatibility between old kernels and new user driver versions.
typedef DDrResources_x64 DDrResources;

#else // _WIN64 and _LP64

// VxWorks 5.5 compiler understand the static assert as an instruction, and forbids an instruction
// in a declaration block (compile time problem)
#ifndef __vxworks
// Only expected with 32 bits compilations. If this fail, you probably need to add a preprocessor condition above.
static_assert(sizeof(size_t) == 4);
#endif // __vxworks

typedef DDrResources_x32 DDrResources;

#endif // _WIN64 and _LP64

#endif //__cplusplus


#if defined(_WIN64) || defined(_LP64)
typedef DDrResources_x64 DDrResources;
#else // _WIN64 and _LP64
typedef DDrResources_x32 DDrResources;
#endif // _WIN64 and _LP64


#if defined(__linux)

//! Adaptor of Windows DeviceIoControl function to Linux
typedef struct
{
    u64 inBufferP;
    u64 nInBufferSize;
    u64 outBufferP;
    u64 nOutBufferSize;
    u64 bytesReturnedP;
    u32  errorCode;
} DDrLinuxIO_x64;

typedef struct
{
    u32 inBufferP;
    u32 nInBufferSize;
    u32 outBufferP;
    u32 nOutBufferSize;
    u32 bytesReturnedP;
    u32 errorCode;
} DDrLinuxIO_x32;

#if defined(_LP64)
typedef DDrLinuxIO_x64 DDrLinuxIO;
#else // _LP64
typedef DDrLinuxIO_x32 DDrLinuxIO;
#endif // _LP64

#endif


//////////////////////////////////////////////////////////////////////////////////////////
// Command Codes for dispersed Read/Write operations with IOCTL_READ_WRITE
//////////////////////////////////////////////////////////////////////////////////////////
#define DDR_WRITE_DEV       0x0          // Write to  device space: MUST be zero!
#define DDR_READ_DEV_INC    0x10000000   // Read from device space, increment addresses
#define DDR_READ_DEV_FIXED  0x20000000   // Read from device space, keep address fixed
#define DDR_WRITE_IFACE     0x30000000   // Write to  interface space
#define DDR_READ_IFACE      0x40000000   // Read from interface space (1 register only)
#define DDR_WAIT_1USEC      0x50000000   // Wait 1 micro-second (in kernel-driver)
#define DDR_WRITE_DIRECT    0x60000000   // Write to  'direct' space, i.e. address is used 'as is'
#define DDR_READ_DIRECT     0x70000000   // Read from 'direct' space, i.e. address is used 'as is'
#define DDR_WRITE_CONFIG    0x80000000   // Write to interface space using configuration access
#define DDR_READ_CONFIG     0x90000000   // Read from interface space using cfg access (1 reg only)
#define DDR_READ_WRITE_MASK 0xf0000000


//////////////////////////////////////////////////////////////////////////////////////////
// User-defined IOCTL control codes for use in the DeviceIoControl() function calls
// to ALL our kernel-mode device drivers.
//////////////////////////////////////////////////////////////////////////////////////////
//
// The following Macro definition for IOCTL control codes has been copied from
// <winioctl.h> in order to avoid including too much stuff (when compiling with
// the user-mode components which shouldn't include <wdm.h> or <winioctl.h>).
// Since this file is also included in the kernel-mode driver files, which DO
// include <wdm.h>, we rename CTL_CODE as OUR_CODE to shut up the compiler. Otherwise,
// it complains about 'macro redefinition'.
// Note that function codes 0-2047 are reserved for Microsoft Corporation, and
// 2048-4095 are reserved for customers.

#define OUR_CODE(DeviceType, Function, Method, Access) \
 (u32) ( ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) )

//
// Define the method codes for how buffers are passed for I/O and FS controls
//
#define METHOD_BUFFERED                 0
#define METHOD_IN_DIRECT                1
#define METHOD_OUT_DIRECT               2
#define METHOD_NEITHER                  3

//
// Define the access check value for any access
//
#define FILE_ANY_ACCESS             0
#define FILE_READ_ACCESS          ( 0x0001 )    // file & pipe
#define FILE_WRITE_ACCESS         ( 0x0002 )    // file & pipe


// By choosing a single device type, we will use the additional 'address space' in
// the 16-bit device type for the (dynamically assigned) device number (WINDOWS95 ONLY!!!)

#define DEV_TYPE    0x8000  // Device type MUST be in range 0x8000 to 0xffff
                            // Since we add the 12-bit device # to the device type
                            // DEV_TYPE must be in the range 0x8000 - 0xf000  !!!!!

//////////////////////////////////////////////////////////////////////////////////////////
// Control Codes for Device Control and Identification
//////////////////////////////////////////////////////////////////////////////////////////
// IMPORTANT: DO NOT change the function number! Otherwise, you lose compatibility with
//              with earlier versions of the kernel-mode drivers!

#define IOCTL_ACQEND_WAITFOR    OUR_CODE(DEV_TYPE, 0x0800, METHOD_BUFFERED, FILE_ANY_ACCESS)
// Usage:   DeviceIoControl(handle, IOCTL_ACQEND_WAITFOR, &timeOut, 4L,
//                          &timeOutOccurred, 4L, &BytesReturned, NULL)
// Input:   timeOut in milliseconds
// Output:  timeOutOccurred: -1 if timeout, 0 if end-of-acquisition interrupt
// Action:  Waits for 'end-of-acquisition' interrupt, with timeout

#define IOCTL_EEPROM_GET        OUR_CODE(DEV_TYPE, 0x0804, METHOD_BUFFERED, FILE_ANY_ACCESS)
// Usage:   DeviceIoControl(handle, IOCTL_EEPROM_GET, cmdP, 8L,
//                          bufP, xxxL, &BytesReturned, NULL)
// Input:   'cmdP' buffer, consisting of 2 long 'header'
//                1st long:    (8-bit) address in EEPROM, right-justified
//                2nd long:    'nbr' = number of longs to read
// Output:  4*nbr bytes ('nbr' longs) of EEPROM  ('xxxL' must be no smaller than 4*'nbr')
// Action:  Reads the EEPROM associated with the PCI interface chip

#define IOCTL_EEPROM_PUT        OUR_CODE(DEV_TYPE, 0x0805, METHOD_BUFFERED, FILE_ANY_ACCESS)
// Usage:   DeviceIoControl(    handle, IOCTL_EEPROM_PUT, bufP, 256L,
//                              NULL, 0, &BytesReturned, NULL)
// Input:   'cmdP' buffer, consisting of 2 long 'header' + 'nbr' of longs to write
//              1st long:    (8-bit) address in EEPROM, right-justified
//              2nd long:    number of ints to read
//              'nbr' long:  data to write to EEPROM
// Output:  None
// Action:  Writes the EEPROM associated with the PCI interface chip

#define IOCTL_FIND_DEVICES        OUR_CODE(DEV_TYPE, 0x0808, METHOD_BUFFERED, FILE_ANY_ACCESS)
// Usage:   DeviceIoControl(    handle, IOCTL_FIND_DEVICES, vendorStringP, 10L,
//                              &NDevices, 4, &BytesReturned, NULL)
// Input:   8 character vendorID string, if NULL --> accept all devices
// Output:  Number of Devices
// Action:  Reads from the registry the number and I/O resources of relevant devices
// FOR WINDOWS95 ONLY!!!

#define IOCTL_GET_NUMBER_DEVICES    OUR_CODE(DEV_TYPE, 0x0809, METHOD_BUFFERED, FILE_ANY_ACCESS)
// Usage:   DeviceIoControl(    handle, IOCTL_GET_NUMBER_DEVICES, NULL, 0,
//                              &NDevices, 4, &BytesReturned, NULL)
// Input:   None
// Output:  Number of Devices
// Action:  Rereads the number of devices handled by the kernel-mode driver
// Comment: We might not need this function, and delete it
// FOR WINDOWS95 ONLY!!!

#define IOCTL_GET_RESOURCES        OUR_CODE(DEV_TYPE, 0x0810, METHOD_BUFFERED, FILE_ANY_ACCESS)
// Usage:   DeviceIoControl(    handle, IOCTL_GET_RESOURCES, NULL, 0,
//                              resourceP, sizeof(DDrResources_x32), &BytesReturned, NULL)
// Input:   None
// Output:  Resource data of the device
// Action:  Reports the resources (I/O port, IRQ etc.) of the requested device
// Comment: These data are stored by the device driver when
//              - the IOCTL_FIND_DEVICES command is executed (WINDOWS95)
//              - the device is started on power-up (WDM)

#define IOCTL_GET_VERSION        OUR_CODE(DEV_TYPE, 0x0811, METHOD_BUFFERED, FILE_ANY_ACCESS)
// Usage:   DeviceIoControl(    handle, IOCTL_GET_VERSION, NULL, 0,
//                              &Version, 4, &BytesReturned, NULL)
// Input:   None
// Output:  Version # of device driver, coded as 0x100*MAJOR + MINOR
// Action:  Reports the version of the device driver

#define IOCTL_INIT                OUR_CODE(DEV_TYPE, 0x0814, METHOD_BUFFERED, FILE_ANY_ACCESS)
// Usage:   DeviceIoControl(handle, IOCTL_INIT, &localInterruptAddr, 4L,
//                          NULL, 0, &BytesReturned, NULL)
// Input:   Address of the device interrupt register
// Output:  None
// Action:  Initializes the device, i.e. connects the interrupt vector, clears the device
//          interrupts, establishes interrupt semaphore

#define IOCTL_INTRPT_STATUS        OUR_CODE(DEV_TYPE, 0x0823, METHOD_BUFFERED, FILE_ANY_ACCESS)
// Usage:   DeviceIoControl(handle, IOCTL_INTRPT_WAITFOR, NULL, 0L, &Status, 4, &BytesReturned, NULL)
// Input:   None
// Output:  Interrupt status register
// Action:  Returns the current contents of the interrupt status register

#define IOCTL_PCI_CONFIG        OUR_CODE(DEV_TYPE, 0x0816, METHOD_BUFFERED, FILE_ANY_ACCESS)
// Usage:   DeviceIoControl(    handle, IOCTL_PCI_CONFIG, NULL, 0L,
//                              bufP, xxxL, &BytesReturned, NULL)
// Input:   None
// Output:  PCI Configuration Space data (256 bytes)
// Action:  Reads the PCI Configuration Space of the device (!= EEPROM of device!)

#define IOCTL_PROCESS_WAITFOR    OUR_CODE(DEV_TYPE, 0x0817, METHOD_BUFFERED, FILE_ANY_ACCESS)
// Usage:   DeviceIoControl(    handle, IOCTL_PROCESS_WAITFOR, &timeOut, 4L,
//                              &timeOutOccurred, 4L, &BytesReturned, NULL)
// Input:   timeOut in milliseconds
// Output:  timeOutOccurred: -1 if timeout, 0 if end-of-processing interrupt
// Action:  Waits for 'end-of-processing' interrupt, with timeout

#define IOCTL_RESTORE_BARS        OUR_CODE(DEV_TYPE, 0x0819, METHOD_BUFFERED, FILE_ANY_ACCESS)
// Usage:   DeviceIoControl(    handle, IOCTL_RESTORE_BARS, NULL, 0,
//                              NULL, 0, &BytesReturned, NULL)
// Input:   'cmdP' buffer 1st long: 0x00000000 for 'PowerOn', 0xFFFFFFFF for 'PowerOff'.
// Output:  None
// Action:  Restores the base address registers 0 and 2 of the specified device

#define IOCTL_GET_RESOURCES_64    OUR_CODE(DEV_TYPE, 0x0820, METHOD_BUFFERED, FILE_ANY_ACCESS)
// Usage:   DeviceIoControl(    handle, IOCTL_GET_RESOURCES_64, NULL, 0,
//                              resourceP, sizeof(DDrResources_x64), &BytesReturned, NULL)
// Input:   None
// Output:  Resource data of the device
// Action:  Reports the resources (I/O port, IRQ etc.) of the requested device
// Comment: These data are stored by the device driver when
//              - the IOCTL_FIND_DEVICES command is executed (WINDOWS95)
//              - the device is started on power-up (WDM)


//////////////////////////////////////////////////////////////////////////////////////////
// Control Codes for Data Transfers to/from supported devices
//////////////////////////////////////////////////////////////////////////////////////////

#define IOCTL_READ                OUR_CODE(DEV_TYPE, 0x0840, METHOD_BUFFERED, FILE_ANY_ACCESS)
// Usage:   DeviceIoControl(handle, IOCTL_READ, cmdP, 8L,
//                          bufP, nbrBytes, &BytesReturned, NULL) )
// Input:   'cmdP' buffer, consisting of 2 values
//              1st long:    address on local bus of device
//              2nd long:    nbrLong = number of 32-bit values to transfer
// Output:  32-bit Long data, returned into 'bufP' (whose length is 'nbrBytes' bytes)
// Action:  Reads a specified number of longs from a given address in the specified device
//          The local address must be incremented after each value.

#define IOCTL_READ_BLK            OUR_CODE(DEV_TYPE, 0x0841, METHOD_BUFFERED, FILE_ANY_ACCESS)
// Usage:   DeviceIoControl(handle, IOCTL_READ_BLK, cmdP, 8L,
//                          bufP, nbrBytes, &BytesReturned, NULL) )
// Input:   'cmdP' buffer, consisting of 2 values
//              1st long:    address on local bus of device
//              2nd long:    nbrLong = number of 32-bit values to transfer
// Output:  32-bit Long data, returned into 'bufP' (whose length is 'nbrBytes' bytes)
// Action:  Reads a specified number of longs from a given address in the specified device
//          The local address must be kept constant

#define IOCTL_READ_DMA            OUR_CODE(DEV_TYPE, 0x0842, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)
// Usage:   DeviceIoControl(handle, IOCTL_READ_DMA, cmdP, 8L,
//                          bufP, nbrBytes, &BytesReturned, NULL) )
// Input:   'cmdP' buffer, consisting of 2 values
//              1st long:    address on local bus of device
//              2nd long:    nbrLong = number of 32-bit values to transfer
// Output:  32-bit Long data, returned into 'bufP' (whose length is 'nbrBytes' bytes)
// Action:  Reads a specified number of longs from a given address in the specified device
//          Use DMA for the transfer. The local address must be kept constant

#define IOCTL_READ_WRITE        OUR_CODE(DEV_TYPE, 0x0845, METHOD_BUFFERED, FILE_ANY_ACCESS)
// Usage:   DeviceIoControl(handle, IOCTL_READ_WRITE, cmdP, 8L + 8*nbrCmd,
//                          bufP, 4*nbrRead, &BytesReturned, NULL) )
// Input:   'cmdP' buffer, consisting of (2 + 2*nbrCmd) u32 values
//              1st long:    nbrCmd  = number of 2-long commands in 'cmdP' buffer
//              2nd long:    nbrRead = total number of 'longs' to read from device
//          followed by 2*'nbrCmd' u32 of transfer commands.
//
//          The first u32 of each command has the format:
//              command = ((XferCode << 28) | (LocalAddress) ),
//              where   - the XferCode can have the values DDR_WRITE_DEV, DDR_READ_DEV_INC
//                          DDR_READ_DEV_FIXED, DDR_WRITE_IFACE or DDR_READ_IFACE
//                      - the LocalAddress is the device-specific local register address
//
//          The second u32 of the command is:
//              for DDR_WRITE_DEV       value to write to the device register
//              for DDR_READ_DEV_INC    number of values to read
//              for DDR_READ_DEV_FIXED  number of values to read
//              for DDR_WRITE_IFACE     value to write to the register
//              for DDR_READ_IFACE      ignored (read only 1 value!)
//              for DDR_WAIT_1USEC      number of micro-seconds
//
//          Only one single u32 per command can be written. However, several read
//          operations from the same register (or from incrementing register addresses)
//          can be executed.
// Output:  if there are any DDR_READ_xx commands, the output consists of 32-bit
//          data, returned into 'bufP' (whose length is '4*nbrRead' bytes)
// Action:  Reads/writes a number of u32, according to a list of commands
//          supplied in 'cmdP'
// Purpose: Read/write a list of control registers whose addresses are dispersed.

#define IOCTL_WRITE                OUR_CODE(DEV_TYPE, 0x0850, METHOD_BUFFERED, FILE_ANY_ACCESS)
// Usage:   DeviceIoControl(handle, IOCTL_WRITE, cmdP, 8L + 4*nbrLong,
//                          NULL, 0, &BytesReturned, NULL) )
// Input:   'cmdP' buffer, consisting of (2 + nbrLong) values
//              1st long:    address on local bus of device
//              2nd long:    nbrLong = number of 32-bit values to transfer
//          followed by nbrLong u32 of data to be written
// Output:  None
// Action:  Writes a specified number of longs to a given address in the specified device
//          The local address must be incremented after each value.

#define IOCTL_WRITE_BLK            OUR_CODE(DEV_TYPE, 0x0851, METHOD_BUFFERED, FILE_ANY_ACCESS)
// Usage:   DeviceIoControl(handle, IOCTL_WRITE_BLK, cmdP, 8L + 4*nbrLong,
//                          NULL, 0, &BytesReturned, NULL) )
// Input:   'cmdP' buffer, consisting of (2 + nbrLong) values
//              1st long:    address on local bus of device
//              2nd long:    nbrLong = number of 32-bit values to transfer
//          followed by nbrLong u32 of data to be written
// Output:  None
// Action:  Writes a specified number of longs to a given address in the specified device
//          The local address must be kept constant.

#define IOCTL_WRITE_DMA            OUR_CODE(DEV_TYPE, 0x0852, METHOD_IN_DIRECT, FILE_ANY_ACCESS)
// Usage:   DeviceIoControl(handle, IOCTL_WRITE_DMA, cmdP, 8L,
//                          bufP, 4*nbrLong, &BytesReturned, NULL) )
// Input:   'cmdP' buffer, consisting of 2 values
//              1st long:    address on local bus of device
//              2nd long:    nbrLong = number of 32-bit values to transfer
//          'bufP' buffer containing nbrLong 32-bit data values to write
// Output:  None
// Action:  Writes a specified number of longs to a given address in the specified device
//          Use DMA for the transfer. The local address must be kept constant

#define IOCTL_NOP                  OUR_CODE(DEV_TYPE, 0x0853, METHOD_BUFFERED, FILE_ANY_ACCESS)
// Usage:   DeviceIoControl(handle, IOCTL_NOP, cmdP, 8L,
//                          NULL, NULL, NULL, NULL) )
// Action:  Does nothing. Only intended for benchmark purposes, for estimating the user mode
//          to the kernel mode transition.

//////////////////////////////////////////////////////////////////////////////////////////
// The following macros are only used for the coding of the device number
// into the driver handle under Windows 95, Linux or VxWorks
//////////////////////////////////////////////////////////////////////////////////////////

#define IOCTL_DEVICE_MASK    0xF000FFFF        // to mask the device number out of the IOCTL code
#define IOCODE(IOCTLCode, device)   (IOCTLCode + ( (device << 16) & ~IOCTL_DEVICE_MASK) )


//////////////////////////////////////////////////////////////////////////////////////////
// The following macros are only used for Linux and VxWorks
// We define a number of macros, which are defined by Microsoft under Windows, and which
// we want to keep using in the Linux and VxWorks version.
//////////////////////////////////////////////////////////////////////////////////////////

#ifndef WIN32

// Error Codes in Kernel-mode Driver
#define ERROR_SUCCESS               0
#define ERROR_INVALID_FUNCTION      1
#define ERROR_FILE_NOT_FOUND        2
#define ERROR_PATH_NOT_FOUND        3

#define ERROR_INVALID_HANDLE        6
#define ERROR_NOT_ENOUGH_MEMORY     8
#define ERROR_NOT_READY             21
#define ERROR_NOT_SUPPORTED         50

#define ERROR_INVALID_PARAMETER     87
#define ERROR_BUFFER_OVERFLOW       111


#define ERROR_SEM_TIMEOUT           121
#define ERROR_NO_DATA               232
#define ERROR_NOACCESS              998
#define ERROR_TIMEOUT               1460

#endif

// Added error for signal interruption on Linux, thus must be defined for Windows also (random value).
#define ERROR_INTERRUPTED           2486


#endif // _DDRIORULES_H


