//////////////////////////////////////////////////////////////////////////////////////////
//
//  LinuxDriverPCI.c    Implementation of Linux Kernel-Space Device Driver
//                      for PCI9080&PCI9054-based and FPGA based Acqiris Devices
//
//----------------------------------------------------------------------------------------
//  Copyright Agilent Technologies, Inc. 2000, 2001-2009
//
//  $Id$
//
//  Started:    6 JUN 2000
//      20/04/01 esch: test on 2.2 kernel
//      26/04/01 esch: added needed changes to get it going on 2.4. kernel
//           esch: major changes are in the scheduler queue and in the module  initialization
//      22/06/01 esch: implemented use of DMA with kiobuf
//      26/06/01 esch: debug level can be set during load dbgl=N
//      17/07/01 esch: timing and return code in IOCTL_ACQEND_WAITFOR adapted to Windows driver
//      18/07/01 esch: only one application can open the driver succesfully
//  from here on this is DriverVersion 1.00  any changes increase the last digit by one
//      27/12/01 esch: added DMA write into module
//      15/01/02 esch: DDR_WAIT_1USEC set for 1000 loop cycles
//      26/04/02 esch: added interrupt handling for processing
//      20/11/03 esch: replaced page_address, pci_map_page to support virtual - physcal mapping of more than 760 MB
//      17/02/04 esch: sometimes we had interrupt timeouts reported even if the interrupt was cleared
//      17/02/04 esch: timeout value for DMA is now dynamically calculated in jiffies
//      17/03/05 tmd: on kernel 2.6, writing *outP = 4; with outP being a pointer to user
//                    space memory fails. put_user(4, outP) must be used instead. Thus convention:
//                    User memory pointer: U (don't *U), Kernel memory pointer: P (can *P).
//      16/12/05 tmd: Added macros to use class_create function and like instead of
//                    class_simple_create for kernels above 2.6.12
//      23/11/06 tmd: Added global lock to prevent accesses from more than 1 process.
//      23/01/07 tmd: Added compatibility mode for 64 bits kernels and 32 bits applications.
//      13/09/07 tmd: Changed IRQ handler according to 2.6.19 and removed IOCTL_RESTORE_BARS
//      04/02/08 tmd: Added changes for kernel version 2.6.22: changed pci_module_init() to
//                    pci_register_driver(), and SA_* flags to IRQF_* flags.
//      23/05/08 tmd: Added changes for kernel version 2.6.24: removed .name in .kobj initializetion
//      29/01/09 tmd: Fixed kerneloops related to dma_map_pages with bad size parameter.
//      21/08/09 tmd: Fixed DMA transfers on Linux 64 bits, and addr space leakage.
//      26/08/09 tmd: Merged 64 bits and 32 bits kernels.
//
//  Owned by:   V. Hungerbuhler, E. Schaefer & D. Trosset
//
//
//
//////////////////////////////////////////////////////////////////////////////////////////

#include <linux/version.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/pagemap.h>
#include <linux/interrupt.h>
#include <linux/pci.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/types.h>
#include <asm/uaccess.h>
#include <linux/sched.h>

#include "acqiris.h"

#if HAVE_UNLOCKED_IOCTL
# include <linux/mutex.h>
#else
# include <linux/smp_lock.h>
#endif

static DEFINE_MUTEX( fs_mutex );

//////////////////////////////////////////////////////////////////////////////////////////
int acqiris_dma_read(struct acqiris_device* aq_dev, u32 local_addr, void __user *dest, size_t size);


//////////////////////////////////////////////////////////////////////////////////////////
#if HAVE_UNLOCKED_IOCTL
long acqiris_ioctl_unlocked(struct file* filp, unsigned int code, unsigned long args)
#else
int acqiris_ioctl(struct inode *inode, struct file* filp, unsigned int code, unsigned long args)
#endif
{
    DDrLinuxIO __user *user_io = (DDrLinuxIO __user *)args;
    struct acqiris_ioop ioop;
    unsigned long in_buf;
    unsigned long out_buf;
    unsigned long ret_size;

    get_user(in_buf, &user_io->inBufferP);
    get_user(ioop.in_size, &user_io->nInBufferSize);
    get_user(out_buf, &user_io->outBufferP);
    get_user(ioop.out_size, &user_io->nOutBufferSize);
    get_user(ret_size, &user_io->bytesReturnedP);

    ioop.in_buf = (u32 __user *)in_buf;
    ioop.out_buf = (u32 __user *)out_buf;
    ioop.ret_size = 0;

    ioop.err_code = acqiris_user_operation(filp, code, &ioop);

    put_user(ioop.ret_size, (size_t __user *)ret_size);
    put_user(ioop.err_code, &user_io->errorCode);

    return ioop.err_code;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Entry point for all 'compat_ioctl' system calls to this driver
#ifdef CONFIG_COMPAT

long acqiris_ioctl_compat(struct file* filp, unsigned int code, unsigned long args)
{
    DDrLinuxIO_x32 __user *user_io = (DDrLinuxIO_x32 __user *)args;
    struct acqiris_ioop ioop;
    unsigned long in_buf;
    unsigned long out_buf;
    unsigned long ret_size;

#if HAVE_UNLOCKED_IOCTL
    mutex_lock(&fs_mutex);
#else
    lock_kernel();
#endif

    get_user(in_buf, &user_io->inBufferP);
    get_user(ioop.in_size, &user_io->nInBufferSize);
    get_user(out_buf, &user_io->outBufferP);
    get_user(ioop.out_size, &user_io->nOutBufferSize);
    get_user(ret_size, &user_io->bytesReturnedP);

    ioop.in_buf = (u32 __user *)in_buf;
    ioop.out_buf = (u32 __user *)out_buf;
    ioop.ret_size = 0;

    ioop.err_code = acqiris_user_operation(filp, code, &ioop);

    put_user(ioop.ret_size, (unsigned int __user *)ret_size);
    put_user(ioop.err_code, &user_io->errorCode);

#if HAVE_UNLOCKED_IOCTL
    mutex_unlock(&fs_mutex);
#else
    unlock_kernel();
#endif

    return ioop.err_code;
}

#endif


//////////////////////////////////////////////////////////////////////////////////////////
static long acqiris_ioop_getnumdevs(struct acqiris_ioop *ioop)
{
    if (ioop->out_size < 4)
        return -ENOMEM;

    put_user(aq_drv.nbr_devices, ioop->out_buf++);
    ioop->ret_size = 4;

    if (ioop->out_size > 4)
    {
        int dev = 0;
        while (ioop->out_size >= ioop->ret_size + 4 && dev < aq_drv.nbr_devices)
        {
            struct acqiris_device *aq_dev = aq_drv.devices[dev];
            if (aq_dev != NULL)
                put_user((aq_dev->vendor_id << 16) | aq_dev->device_id, ioop->out_buf++);
            else
                put_user(0, ioop->out_buf++);

            ioop->ret_size += 4;
            ++dev;
        }

    }

    if (dbgl&DINIT) printk(ACQRS_INFO "  IOCTL_FIND_DEVICES/GET_NUMBER_DEVICES: %d\n", aq_drv.nbr_devices);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
static long acqiris_ioop_getversion(struct acqiris_ioop *ioop)
{
    if (ioop->out_size < 4)
        return -ENOMEM;

    put_user((ulong)MOD_VERSION_CODE, ioop->out_buf);
    ioop->ret_size = 4;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
long acqiris_config_operation(struct file* filp, u32 code, struct acqiris_ioop *ioop)
{
    if (ioop->out_buf == NULL)
        ioop->out_size = 0;

    ioop->ret_size = 0;

    if (dbgl&DIRW) printk(ACQRS_INFO "acqiris_config_operation code %#010x %#010x\n", code, code>>2);

    switch (code)
    {
        case IOCTL_FIND_DEVICES:
        case IOCTL_GET_NUMBER_DEVICES: /* Report number of devices, and optionaly their vendor/device ID pairs */
            return acqiris_ioop_getnumdevs(ioop);

        case IOCTL_GET_VERSION: /* Report device driver version number */
            return acqiris_ioop_getversion(ioop);

        default: /* Unrecognized IOCTL code */
            return -EPERM;
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
static long acqiris_ioop_getres32(struct acqiris_device* aq_dev, struct acqiris_ioop *ioop)
{
    int copied;

    if (ioop->out_size < sizeof(DDrResources_x32) )
        return -ENOMEM;

    // Copy the resource block to the output
    if (sizeof(aq_dev->res) == sizeof(DDrResources_x32))
    {
        copied = copy_to_user(ioop->out_buf, &aq_dev->res, sizeof(DDrResources_x32));
    }
    else
    {
        DDrResources_x32 __user *res = (DDrResources_x32 __user *)ioop->out_buf;
        put_user(aq_dev->res.controlAddr, &res->controlAddr);
        put_user(aq_dev->res.controlBase, &res->controlBase);
        put_user(aq_dev->res.controlSize, &res->controlSize);
        put_user(aq_dev->res.directAddr, &res->directAddr);
        put_user(aq_dev->res.directBase, &res->directBase);
        put_user(aq_dev->res.directSize, &res->directSize);
        put_user(aq_dev->res.busNumber, &res->busNumber);
        put_user(aq_dev->res.devNumber, &res->devNumber);
        put_user(aq_dev->res.interrupt, &res->interrupt);
        put_user(aq_dev->res.IRQHandle, &res->IRQHandle);
        put_user(aq_dev->res.intrptLocalAddr, &res->intrptLocalAddr);
        put_user(aq_dev->res.alarmMaskPattern, &res->alarmMaskPattern);
        put_user(aq_dev->res.alarmEvntPattern, &res->alarmEvntPattern);
        copied = copy_to_user(res->name, aq_dev->res.name, DDR_NAME_SIZE);
    }

    ioop->ret_size = sizeof(DDrResources_x32);

    if (dbgl&DINIT) printk(ACQRS_INFO "  IOCTL_GET_RESOURCES: %u bytes\n", (int)sizeof(DDrResources_x32));

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
static long acqiris_ioop_getres64(struct acqiris_device* aq_dev, struct acqiris_ioop *ioop)
{
    int copied;

    if (ioop->out_size < sizeof(DDrResources_x64) )
        return -ENOMEM;

    // Copy the resource block to the output
    if (sizeof(aq_dev->res) == sizeof(DDrResources_x64))
    {
        copied = copy_to_user(ioop->out_buf, &aq_dev->res, sizeof(DDrResources_x64));
    }
    else
    {
        DDrResources_x64 __user *resU = (DDrResources_x64 __user *)ioop->out_buf;
        put_user(aq_dev->res.controlAddr, &resU->controlAddr);
        put_user(aq_dev->res.controlBase, &resU->controlBase);
        put_user(aq_dev->res.controlSize, &resU->controlSize);
        put_user(aq_dev->res.directAddr, &resU->directAddr);
        put_user(aq_dev->res.directBase, &resU->directBase);
        put_user(aq_dev->res.directSize, &resU->directSize);
        put_user(aq_dev->res.busNumber, &resU->busNumber);
        put_user(aq_dev->res.devNumber, &resU->devNumber);
        put_user(aq_dev->res.interrupt, &resU->interrupt);
        put_user(aq_dev->res.IRQHandle, &resU->IRQHandle);
        put_user(aq_dev->res.intrptLocalAddr, &resU->intrptLocalAddr);
        put_user(aq_dev->res.alarmMaskPattern, &resU->alarmMaskPattern);
        put_user(aq_dev->res.alarmEvntPattern, &resU->alarmEvntPattern);
        copied = copy_to_user(resU->name, aq_dev->res.name, DDR_NAME_SIZE);
    }
    ioop->ret_size = sizeof(DDrResources_x64);

    if (dbgl&DINIT) printk(ACQRS_INFO "  IOCTL_GET_RESOURCES_64: %u bytes\n", (int)sizeof(DDrResources_x64));

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
static long acqiris_ioop_waitacqend(struct acqiris_device* aq_dev, struct acqiris_ioop *ioop)
{
    long timeVal;           // cmdP[0] contains timeout in msec
    long timeout;           // timeout in 'jiffies'
    long timeToExpire;
    u32 value;

    DECLARE_WAITQUEUE(waitA, current);

    if (dbgl&DIRW) printk(ACQRS_INFO "  IOCTL_ACQEND_WAITFOR\n");

    if (ioop->out_size < 4)
        return -ENOMEM;

    get_user(timeVal, ioop->in_buf);                    // cmdP[0] contains timeout in msec
    timeout = ((timeVal*HZ)/1000) + 1;          // timeout in 'jiffies'
    timeToExpire =1;

    add_wait_queue(&aq_dev->wait_acq, &waitA);

    set_current_state(TASK_INTERRUPTIBLE);

    aq_dev->mask_int = DDR_ACQEND_INTERRUPT;

    // Enable the EofA interrupt
    WRITE_REG32(aq_dev->res.intrptLocalAddr, 0, DDR_ACQEND_INTERRUPT);

    timeToExpire = schedule_timeout(timeout);

    remove_wait_queue(&aq_dev->wait_acq, &waitA);

    // Disable all interrupts
    WRITE_REG32(aq_dev->res.intrptLocalAddr, 0, 0L);

    // If no timeout occurred, we report the current value of the interrupt
    // status register (which is never -1). The calling routine must interpret
    // the meaning of the bits.

    value = READ_REG32(aq_dev->res.intrptLocalAddr, 0);

    // in some cases we get a timout even if we cleared the interrupt (seen on RH 8.0)
    if (timeToExpire == 0 && (aq_dev->mask_int & DDR_ACQEND_INTERRUPT))              // Timeout occurred!
        value |= DDR_TIMEOUT_MARKER;        // set the TIMEOUT MARKER
    else
        value &= ~DDR_TIMEOUT_MARKER;

    put_user(value, ioop->out_buf);
    ioop->ret_size = 4;

    aq_dev->mask_int = 0;

    if (dbgl&DAEW) printk(ACQRS_INFO "IOCTL_ACQEND_WAITFOR: timeout set %ld reached %ld "
            "[%d ms] int reg: %#x timeval %ld \n", timeout, timeToExpire, 1000/HZ,
            value, timeVal);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
static long acqiris_ioop_waitprocend(struct acqiris_device* aq_dev, struct acqiris_ioop *ioop)
{
    long timeVal;           // cmdP[0] contains timeout in msec
    long timeout;           // timeout in 'jiffies'
    long timeToExpire;
    unsigned long value;

    DECLARE_WAITQUEUE(waitP, current);

    if (dbgl&DIRW) printk(ACQRS_INFO "  IOCTL_PROCESS_WAITFOR\n");

    get_user(timeVal, ioop->in_buf);                    // cmdP[0] contains timeout in msec
    timeout = ((timeVal*HZ)/1000) + 1;          // timeout in 'jiffies'
    timeToExpire =1;

    if (ioop->out_size < 4)
        return -ENOMEM;

    // See also the comments under IOCTL_ACQEND_WAITFOR above!
    add_wait_queue(&aq_dev->wait_proc, &waitP);

    set_current_state(TASK_INTERRUPTIBLE);

    aq_dev->mask_int = DDR_PROCEND_INTERRUPT;

    // Enable the EofP interrupt
    WRITE_REG32(aq_dev->res.intrptLocalAddr, 0, DDR_PROCEND_INTERRUPT);

    timeToExpire = schedule_timeout(timeout);

    remove_wait_queue(&aq_dev->wait_proc, &waitP);

    // Disable all interrupts
    WRITE_REG32(aq_dev->res.intrptLocalAddr, 0, 0L);

    // If no timeout occurred, we report the current value of the interrupt
    // status register (which is never -1). The calling routine must interpret
    // the meaning of the bits.

    value = READ_REG32(aq_dev->res.intrptLocalAddr, 0);

    // it can happen that we get a timout even if we cleared the interrupt (seen on RH 8.0)
    if (timeToExpire == 0 && (aq_dev->mask_int & DDR_PROCEND_INTERRUPT) )    // Timeout occurred!
        value |= DDR_TIMEOUT_MARKER;        // set the TIMEOUT MARKER
    else
        value &= ~DDR_TIMEOUT_MARKER;

    aq_dev->mask_int = 0;

    put_user(value, ioop->out_buf);
    ioop->ret_size = 4;

    if (dbgl&DAEW) printk(ACQRS_INFO "IOCTL_PROCESS_WAITFOR: timeout set %ld reached %ld "
            "[%d ms] int reg: 0x%lx timeval %ld \n", timeout, timeToExpire, 1000/HZ,
            value, timeVal);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
static long acqiris_ioop_init(struct acqiris_device* aq_dev, struct acqiris_ioop *ioop)
{
    u32 interrupt_offset;
    u32 value;
    char buffer[20];

    if (dbgl&DINIT) printk(ACQRS_INFO "  IOCTL_INIT\n");

    get_user(interrupt_offset, ioop->in_buf);

    aq_dev->res.intrptLocalAddr = (aq_dev->res.directBase + interrupt_offset);
    if(dbgl&DINIT) printk(ACQRS_INFO "  intrptLocalAddr{%p} (+%#x).\n", (void *)aq_dev->res.intrptLocalAddr, interrupt_offset);

    WRITE_REG32(aq_dev->res.controlBase, DMA0CSR_OFFSET, DMA0CSR_CLRINTRPT);

    // NOTE: The first line clears interrupts in devices without AutoClear capability
    //       The third line clears interrupts and disables AutoClear (is available)
    // A "non-applicable" operation does not disturb a device!
    value = READ_REG32(aq_dev->res.intrptLocalAddr, 0);      // Read to clear
    if(dbgl&DINIT) printk(ACQRS_INFO "  intrptLocalAddr = %#010x (%08x + 0x1fffffff).\n", value, DISABLE_AUTOCLEAR);
    value = DISABLE_AUTOCLEAR + 0x1fffffff;
    WRITE_REG32(aq_dev->res.intrptLocalAddr, DEVICE_INTERRUPTCLEAR_OFFSET, value);               // Clear all interrupts
    WRITE_REG32(aq_dev->res.intrptLocalAddr, 0, 0L);         // Disable all interrupts

    value = READ_REG32(aq_dev->res.controlBase, MARBR_OFFSET);
    if(dbgl&DINIT) printk(ACQRS_INFO "  MARBR_OFFSET = %#010x (|= 0x01000000).\n", value);
    value |= 0x01000000;    // for the rev 2.1 mode , bit 24
    WRITE_REG32(aq_dev->res.controlBase, MARBR_OFFSET, value);

    // Unmask the PLX interface interrupt, the module interrupt and the DMA interrupt
    value  = READ_REG32(aq_dev->res.controlBase, INTSRC_OFFSET);
    if(dbgl&DINIT) printk(ACQRS_INFO "  INTSRC_OFFSET = %#010x (|= %#010x | %#010x).\n", value, INTSRC_PCI_INT_EN, INTSRC_LOCAL_INT_EN);
    value |= (INTSRC_PCI_INT_EN | INTSRC_LOCAL_INT_EN);
    WRITE_REG32(aq_dev->res.controlBase, INTSRC_OFFSET, value);

    if(dbgl&DINIT) printk(ACQRS_INFO "IOCTL_INIT device(%s) initialized.\n", format_dev_t(buffer, aq_dev->dev));

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
static long acqiris_ioop_intstatus(struct acqiris_device* aq_dev, struct acqiris_ioop *ioop)
{
    ulong  value;

    if (dbgl&DIRW) printk(ACQRS_INFO "  IOCTL_INTRPT_STATUS\n");
    value = READ_REG32(aq_dev->res.intrptLocalAddr, 0);  // Read status register

    // Since we disable the interrupt-autoclear , we need to clear explicitly
    value += DISABLE_AUTOCLEAR;
    WRITE_REG32(aq_dev->res.intrptLocalAddr, DEVICE_INTERRUPTCLEAR_OFFSET, value);

    put_user(value, ioop->out_buf++);
    ioop->ret_size = 4;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
static long acqiris_ioop_readpio(struct acqiris_device* aq_dev, struct acqiris_ioop *ioop, int fixed)
{
    u32 local_addr;
    u32 nbr_longs;
    u32 nLong;

    if (dbgl&DIRW) printk(ACQRS_INFO "  IOCTL_READ%s\n", !fixed ? "" : "_BLK");

    get_user(local_addr, &ioop->in_buf[0]);
    get_user(nbr_longs, &ioop->in_buf[1]);

    ioop->ret_size = sizeof(u32) * nbr_longs;

    if (ioop->out_size <  ioop->ret_size)
        return -ENOMEM;

    for (nLong = 0 ; nLong < nbr_longs ; ++nLong)
    {
        u32 value = READ_REG32(aq_dev->res.directBase, local_addr);
        put_user(value, ioop->out_buf++);

        if (!fixed)
            local_addr += 4;

    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
static long acqiris_ioop_readdma(struct acqiris_device* aq_dev, struct acqiris_ioop *ioop)
{
    u32 local_addr;
    u32 nbr_longs;
    size_t size;
    void *dest;

    if (dbgl&DIRW) printk(ACQRS_INFO "  IOCTL_READ_DMA\n");

    get_user(local_addr, &ioop->in_buf[0]);
    get_user(nbr_longs, &ioop->in_buf[1]);

    size = nbr_longs * sizeof(u32);
    dest = (void __user *)ioop->out_buf;

    ioop->ret_size = size;

    if (ioop->out_size < ioop->ret_size)
        return -ENOMEM;

    if (nbr_longs > DDR_MAX_DMABLOCK)
        return -EINVAL;

    return acqiris_dma_read(aq_dev, local_addr, dest, size);
}


//////////////////////////////////////////////////////////////////////////////////////////
static long acqiris_ioop_readwrite(struct acqiris_device* aq_dev, struct acqiris_ioop *ioop)
{
    u32 nbr_cmds;      /* Number of commands to execute */
    u32 nbr_to_read;   /* Number of values to read */
    u32 cmd_num;
    u32 nbr_read = 0;

    if (dbgl&DIRW) printk(ACQRS_INFO "  IOCTL_READ_WRITE\n");

    get_user(nbr_cmds, &ioop->in_buf[0]);
    get_user(nbr_to_read, &ioop->in_buf[1]);

    ioop->ret_size = sizeof(u32) * nbr_to_read;

    if (ioop->out_size < ioop->ret_size)
        return -ENOMEM;

    for (cmd_num = 0 ; cmd_num < nbr_cmds ; ++cmd_num)
    {
        u32 n;
        u32 cmd;
        u32 value;
        u32 local_addr;

        get_user(cmd, &ioop->in_buf[2 + 2 * cmd_num]);
        get_user(value, &ioop->in_buf[3 + 2 * cmd_num]);
        local_addr = cmd & ~DDR_READ_WRITE_MASK;

        switch (cmd & DDR_READ_WRITE_MASK)
        {
            case DDR_WRITE_DEV: /* value = value to write to register */
                {
                    WRITE_REG32(aq_dev->res.directBase, local_addr, value);
                    WRITE_REG32(aq_dev->res.controlBase, MAILBOX7_OFFSET, value);
                }
                break;

            case DDR_READ_DEV_INC: /* value = number of read operations */
                for (n = 0 ; n < value ; ++n)
                {
                    ulong tmp = READ_REG32(aq_dev->res.directBase, local_addr);
                    put_user(tmp, ioop->out_buf++);

                    local_addr++;

                    if (++nbr_read > nbr_to_read)
                    {
                        return -ENOMEM;
                    }

                }
                break;

            case DDR_READ_DEV_FIXED: /* value = number of read operations */
                for (n = 0 ; n < value ; ++n)
                {
                    ulong tmp = READ_REG32(aq_dev->res.directBase, local_addr);
                    put_user(tmp, ioop->out_buf++);

                    if (++nbr_read > nbr_to_read)
                    {
                        return -ENOMEM;
                    }

                }
                break;

            case DDR_WRITE_IFACE: /* value = value to write to register */
                {
                    WRITE_REG32(aq_dev->res.controlBase, local_addr, value);
                }
                break;

            case DDR_READ_IFACE:
                {
                    ulong tmp = READ_REG32(aq_dev->res.controlBase, local_addr);
                    put_user(tmp, ioop->out_buf++);

                    if (++nbr_read > nbr_to_read)
                    {
                        return -ENOMEM;
                    }

                }
                break;

            case DDR_WAIT_1USEC: /* value = should be in micro-seconds */
                {
                    udelay(value);
                }

        }
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
static long acqiris_ioop_writepio(struct acqiris_device* aq_dev, struct acqiris_ioop *ioop, int fixed)
{
    u32 local_addr;
    u32 nbr_longs;
    u32 long_num;

    if (dbgl&DIRW) printk(ACQRS_INFO "  IOCTL_WRITE%s\n", !fixed ? "" : "_BLK");

    get_user(local_addr, &ioop->in_buf[0]); /* Local address in PCI module */
    get_user(nbr_longs, &ioop->in_buf[1]); /* Number of longs to write */

    ioop->in_buf += 2;
    for (long_num = 0 ; long_num < nbr_longs; ++long_num)
    {
        u32 value;
        get_user(value, ioop->in_buf++);
        WRITE_REG32(aq_dev->res.directBase, local_addr, value);

        if (!fixed)
            local_addr += 4;

    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
static long acqiris_ioop_writedma(struct acqiris_device* aq_dev, struct acqiris_ioop *ioop)
{
    /* DMA is currently not supported and implemented below as regular write operations */

    u32 local_addr;
    u32 nbr_longs;
    u32 long_num;

    if (dbgl&DIRW) printk(ACQRS_INFO "  IOCTL_WRITE_DMA\n");

    get_user(local_addr, &ioop->in_buf[0]); /* Local address in PCI module */
    get_user(nbr_longs, &ioop->in_buf[1]); /* Number of longs to write */

    for (long_num = 0 ; long_num < nbr_longs ; ++long_num)
    {
        u32 value;
        get_user(value, ioop->out_buf++);
        WRITE_REG32(aq_dev->res.directBase, local_addr, value);

    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
long acqiris_user_operation(struct file* filp, u32 code, struct acqiris_ioop *ioop)
{
    char buffer[20];

    struct acqiris_device* aq_dev = filp->private_data;

    if (aq_dev == NULL) // We are operating on the general device node
        return acqiris_config_operation(filp, code, ioop);

    if (ioop->out_buf == NULL)                   // non-existing output buffer ==> size is zero
        ioop->out_size = 0;

    ioop->ret_size = 0;

    if (dbgl&DOPS) printk(ACQRS_INFO "acqiris_user_operation(%s) code %#x  %#x\n", format_dev_t(buffer, aq_dev->dev), code, code>>2);

    switch (code)
    {
        case IOCTL_GET_RESOURCES: /* Report (already stored) device resources */
            return acqiris_ioop_getres32(aq_dev, ioop);

        case IOCTL_GET_RESOURCES_64: /* Report (already stored) device resources */
            return acqiris_ioop_getres64(aq_dev, ioop);

        case IOCTL_ACQEND_WAITFOR: /* Wait for end-of-aquisition interrupt */
            return acqiris_ioop_waitacqend(aq_dev, ioop);

        case IOCTL_PROCESS_WAITFOR: /* Wait for end-of-processing interrupt */
            return acqiris_ioop_waitprocend(aq_dev, ioop);

        case IOCTL_INIT: /* Initialize a device to enable the interrupts at given offset */
            return acqiris_ioop_init(aq_dev, ioop);

        case IOCTL_INTRPT_STATUS:
            return acqiris_ioop_intstatus(aq_dev, ioop);

        case IOCTL_READ:     /* Read 32 bits data from incrementing address in device */
        case IOCTL_READ_BLK: /* Read 32 bits data from fixed address in device */
            return acqiris_ioop_readpio(aq_dev, ioop, code == IOCTL_READ_BLK);

        case IOCTL_READ_DMA: /* Read 32 bits data from a device via DMA */
            return acqiris_ioop_readdma(aq_dev, ioop);

        case IOCTL_READ_WRITE: /* Read/Write dispersed data from/to a device */
            return acqiris_ioop_readwrite(aq_dev, ioop);

        case IOCTL_WRITE:     /* Write 32 bits data to incrementing address in device */
        case IOCTL_WRITE_BLK: /* Write 32 bits data to fixed address in device */
            return acqiris_ioop_writepio(aq_dev, ioop, code == IOCTL_WRITE_BLK);

        case IOCTL_WRITE_DMA: /* Write 32 bits data to a device via DMA */
            return acqiris_ioop_writedma(aq_dev, ioop);

    }

    return acqiris_config_operation(filp, code, ioop);
}


