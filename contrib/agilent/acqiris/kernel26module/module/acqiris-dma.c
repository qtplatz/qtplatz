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
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <asm/page.h>

#include "acqiris.h"


//////////////////////////////////////////////////////////////////////////////////////////
static int free_dma_desc(struct acqiris_device* aq_dev, char *desc_base, int nbr_pages);
static int make_dma_desc(struct acqiris_device* aq_dev, char *desc_base, u32 local_addr,
                         void __user *dest, size_t size, struct page **pages, int nbr_pages);

static int free_dma_desc_dev_0(struct acqiris_device* aq_dev, char *desc_base, int nbr_pages);
static int make_dma_desc_dev_0(struct acqiris_device* aq_dev, char *desc_base, u32 local_addr,
                               void __user *dest, size_t size, struct page **pages, int nbr_pages);

static int free_dma_desc_dev_1(struct acqiris_device* aq_dev, char *desc_base, int nbr_pages);
static int make_dma_desc_dev_1(struct acqiris_device* aq_dev, char *desc_base, u32 local_addr,
                               void __user *dest, size_t size, struct page **pages, int nbr_pages);

static int free_dma_desc_dev_2(struct acqiris_device* aq_dev, char *desc_base, int nbr_pages);
static int make_dma_desc_dev_2(struct acqiris_device* aq_dev, char *desc_base, u32 local_addr,
                               void __user *dest, size_t size, struct page **pages, int nbr_pages);


//////////////////////////////////////////////////////////////////////////////////////////
static void release_user_pages(int nbr_pages, struct page **pages, int set_dirty)
{
    int page = 0;

    for (page = 0 ; page < nbr_pages ; ++page)
    {
        if (set_dirty)
            set_page_dirty_lock(pages[page]);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0)        
        put_page( pages[page] ); // 2017-09-17
#else
        page_cache_release(pages[page]);
#endif

    }

}


//////////////////////////////////////////////////////////////////////////////////////////
int acqiris_dma_read(struct acqiris_device* aq_dev, u32 local_addr, void __user *dest, size_t size)
{
    long retval = 0;
    char buffer[20];
    long timeToExpire;

    DECLARE_WAITQUEUE(wait, current);
    // DECLARE_WAITQUEUE( wait, get_current() );

    dma_addr_t desc_addr = virt_to_phys(aq_dev->dma_desc);
    u32 desc_addr_lo = desc_addr;
    u32 desc_addr_hi = desc_addr >> 16 >> 16; /* >>32 in nop in 32 bits */
    ulong value;
    int nbr_pages;
    int nbr_pages_needed;

    nbr_pages_needed = (((unsigned long)dest + size - 1) / PAGE_SIZE) - ((unsigned long)dest / PAGE_SIZE) + 1;

    down_read(&current->mm->mmap_sem);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0)
    nbr_pages = get_user_pages((unsigned long)dest, nbr_pages_needed, 0, aq_dev->pages, NULL );
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(4, 6, 0)
    nbr_pages = get_user_pages((unsigned long)dest, nbr_pages_needed, 1, 0, aq_dev->pages, NULL );
#else  // 2.6
    nbr_pages = get_user_pages(current, current->mm, (unsigned long)dest, nbr_pages_needed, 1, 0, aq_dev->pages, NULL);
#endif
    
    up_read(&current->mm->mmap_sem);

    if (nbr_pages < 0)
    {
        printk(ACQRS_ERR "get_user_pages failed. 0x%x \n", nbr_pages);
        return -ENOMEM;
    }
    else if (nbr_pages < nbr_pages_needed)
    {
        printk(ACQRS_ERR "get_user_pages failed. Only %d pages out of %d.\n", nbr_pages, nbr_pages_needed);
        release_user_pages(nbr_pages, aq_dev->pages, 0);
        return -ENOMEM;
    }

    if(dbgl&DDMA) printk(ACQRS_INFO "DMA Read addr %#06x dest %p size %#010zx (desc %08x:%08x)\n", \
                         local_addr, dest, size, desc_addr_hi, desc_addr_lo);

    /* init the dma chain */
    retval = make_dma_desc(aq_dev, aq_dev->dma_desc, local_addr, dest, size, aq_dev->pages, nbr_pages);
    if (retval < 0)
    {
        release_user_pages(nbr_pages, aq_dev->pages, 0);
        return retval;
    }

    WRITE_REG32(aq_dev->res.controlBase, DMA0DESC_OFFSET, desc_addr_lo + 0x9);
    if (aq_dev->vendor_id == PCI_DEVICE_ID_ACQIRIS_2)
        WRITE_REG32(aq_dev->res.controlBase, DMA0DESC_OFFSET + 4, desc_addr_hi);

    /* Set the DMA mode */
    value = READ_REG32(aq_dev->res.controlBase, DMA0MODE_OFFSET);
    value |= DMA0MODE;
    WRITE_REG32(aq_dev->res.controlBase, DMA0MODE_OFFSET, value);

    /* first add yourself to the wait queue then inable the interrupt
     * otherwise the interrupt handler will crash */
    add_wait_queue(&aq_dev->wait_dma, &wait);
    set_current_state(TASK_INTERRUPTIBLE);

    /* Enable DMA interrupt at the level of the DMA controller */
    value = (DMA0CSR_CLRINTRPT | DMA0CSR_ENABLE);
    WRITE_REG32(aq_dev->res.controlBase, DMA0CSR_OFFSET, value);

    /* Enable DMA interrupt at the level of the global interrupt */
    value  = READ_REG32(aq_dev->res.controlBase, INTSRC_OFFSET);
    value |= INTSRC_DMA_INT_EN;
    WRITE_REG32(aq_dev->res.controlBase, INTSRC_OFFSET, value);

    /* Start actual DMA transfer by enabling device */
    value = (DMA0CSR_ENABLE | DMA0CSR_START);
    WRITE_REG32(aq_dev->res.controlBase, DMA0CSR_OFFSET, value);

    /* DMA transfer speed is about 100 MB/sec
     * value should correspond to a transfer time needed to transfer 100 MB + requested amount of MB */

    value = (100 + (size>>22))* HZ/100;
    timeToExpire = schedule_timeout( value ); /* value is given in jiffies */

    remove_wait_queue(&aq_dev->wait_dma, &wait);

    if (!timeToExpire)
    {   /* Did we return because of timeout? */
        u32 value0  = READ_REG32(aq_dev->res.controlBase, DMA0CSR_OFFSET);    /* Test only! */

        /* Abort the DMA */
        WRITE_REG32(aq_dev->res.controlBase, DMA0CSR_OFFSET, 0);              /* Disable DMA0 */
        value0  = READ_REG32(aq_dev->res.controlBase, DMA0CSR_OFFSET);
        if ((value0 & DMA0CSR_CHAN_DONE) == 0)
        {         /* If DMA not finished yet */
            value = DMA0CSR_CLRINTRPT | DMA0CSR_ABORT;
            WRITE_REG32(aq_dev->res.controlBase, DMA0CSR_OFFSET, value);      /* Abort + Clear Interrupt */
        }
        else
        {
            value = DMA0CSR_CLRINTRPT;
            WRITE_REG32(aq_dev->res.controlBase, DMA0CSR_OFFSET, value);      /* Clear Interrupt only */
        }

        printk(ACQRS_WARNING "DMA read on device(%s) (%#010x): Timeout occurred.\n", format_dev_t(buffer, aq_dev->dev), value0);

        retval = -ETIME;
    }

    free_dma_desc(aq_dev, aq_dev->dma_desc, nbr_pages);

    release_user_pages(nbr_pages, aq_dev->pages, (retval==0));

    return retval;
}


//////////////////////////////////////////////////////////////////////////////////////////
#if LINUX_VERSION_CODE < 0x20613 /* irq_handler_t has changed in 2.6.19 */
irqreturn_t acqiris_interrupt_handler(int irq, void* private, struct pt_regs* regs)
#else
irqreturn_t acqiris_interrupt_handler(int irq, void* private)
#endif
{
    struct acqiris_device* aq_dev = (struct acqiris_device*)private;
    char buffer[20];
    u32 plxIntrptReg, value;

    if (aq_dev == NULL)
        return IRQ_NONE;

    plxIntrptReg  = READ_REG32(aq_dev->res.controlBase, INTSRC_OFFSET); /* Read interrupt status */

    /* Test the case for device_id 2 cards where reading a register during a DMA transfer
     * is not supported. Interrupts can still occur because of shared interrupts. And in
     * that case, it must be related to another card and ignored. */
    if (plxIntrptReg == 0xffffffff)
    {
        if (dbgl&DINT) printk(ACQRS_NOTICE "Unknown Interrupt occured during DMA on device(%s).\n", format_dev_t(buffer, aq_dev->dev));

        return IRQ_NONE;
    }

    /* We must check every enabled interrupt separately
     * Note that we ONLY service ONE interrupt at a time!! If more than 1 interrupt is set,
     * we expect the second (non-serviced) interrupt to become active again. */

    if ( (plxIntrptReg & INTSRC_DMA_ACTIVE) != 0) /* Treat a DMA interrupt */
    {
        if(dbgl&DINT) printk(ACQRS_INFO "PLX DMA Interrupt %#010x on device(%s).\n", plxIntrptReg, format_dev_t(buffer, aq_dev->dev));

        /* Wake up and disable DMA interrupt at the level of the global interrupt */
        value = READ_REG32(aq_dev->res.controlBase, DMA0CSR_OFFSET);  /* Read to CLEAR! */
        value |=  DMA0CSR_CLRINTRPT;
        WRITE_REG32(aq_dev->res.controlBase, DMA0CSR_OFFSET, DMA0CSR_CLRINTRPT); /* if it is still running Abort */

        wake_up_interruptible(&aq_dev->wait_dma);

        return IRQ_HANDLED;
    }

    if ( (plxIntrptReg & INTSRC_LOCAL_ACTIVE) != 0) /* Treat a device interrupt */
    {
        if (aq_dev->res.intrptLocalAddr != 0) /* If the device has been initialized */
        {
            value = READ_REG32(aq_dev->res.intrptLocalAddr, 0); /* Read to CLEAR! */

            if(dbgl&DINT) printk(ACQRS_INFO "PLX Local Interrupt %#010x (%#08x) on device(%s).\n", plxIntrptReg, value, format_dev_t(buffer, aq_dev->dev));

            if ( (aq_dev->mask_int & DDR_ACQEND_INTERRUPT) && (value & DDR_ACQEND_INTERRUPT) ) /* End-of-Acquisition? */
            {
                value = DISABLE_AUTOCLEAR + DDR_ACQEND_INTERRUPT;
                WRITE_REG32(aq_dev->res.intrptLocalAddr, DEVICE_INTERRUPTCLEAR_OFFSET, value); /* Clear EofA interrupt */

                wake_up_interruptible(&aq_dev->wait_acq);
                aq_dev->mask_int = 0;

                return IRQ_HANDLED;
            }

            if ( (aq_dev->mask_int & DDR_PROCEND_INTERRUPT) && (value & DDR_PROCEND_INTERRUPT) ) /* End-of-Processing? */
            {
                value = DISABLE_AUTOCLEAR + DDR_PROCEND_INTERRUPT;
                WRITE_REG32(aq_dev->res.intrptLocalAddr, DEVICE_INTERRUPTCLEAR_OFFSET, value); /* Clear EofP interrupt */

                wake_up_interruptible(&aq_dev->wait_proc);
                aq_dev->mask_int = 0;

                return IRQ_HANDLED;
            }

        }
        else
        {
            if(dbgl&DINT) printk(ACQRS_WARNING "PLX Local Interrupt %#010x on ininitialized device(%s).\n", plxIntrptReg, format_dev_t(buffer, aq_dev->dev));

            return IRQ_NONE;
        }

    }
    if (dbgl&DINT) printk(ACQRS_NOTICE "Unknown PLX Interrupt %#010x on device(%s).\n", plxIntrptReg, format_dev_t(buffer, aq_dev->dev));

    return IRQ_NONE;
}


//////////////////////////////////////////////////////////////////////////////////////////
int free_dma_desc(struct acqiris_device* aq_dev, char *desc_base, int nbr_pages)
{
    switch (aq_dev->device_id)
    {
        case PCI_DEVICE_ID_ACQIRIS_0:
            return free_dma_desc_dev_0(aq_dev, aq_dev->dma_desc, nbr_pages);
        case PCI_DEVICE_ID_ACQIRIS_1:
            return free_dma_desc_dev_1(aq_dev, aq_dev->dma_desc, nbr_pages);
        case PCI_DEVICE_ID_ACQIRIS_2:
            return free_dma_desc_dev_2(aq_dev, aq_dev->dma_desc, nbr_pages);

    }

    return -ENOENT;
}

//////////////////////////////////////////////////////////////////////////////////////////
int make_dma_desc(struct acqiris_device* aq_dev, char *desc_base, u32 local_addr, void __user *dest, size_t size, struct page **pages, int nbr_pages)
{
    switch (aq_dev->device_id)
    {
        case PCI_DEVICE_ID_ACQIRIS_0:
            return make_dma_desc_dev_0(aq_dev, aq_dev->dma_desc, local_addr, dest, size, aq_dev->pages, nbr_pages);
        case PCI_DEVICE_ID_ACQIRIS_1:
            return make_dma_desc_dev_1(aq_dev, aq_dev->dma_desc, local_addr, dest, size, aq_dev->pages, nbr_pages);
        case PCI_DEVICE_ID_ACQIRIS_2:
            return make_dma_desc_dev_2(aq_dev, aq_dev->dma_desc, local_addr, dest, size, aq_dev->pages, nbr_pages);

    }

    return -ENOENT;
}


//////////////////////////////////////////////////////////////////////////////////////////
struct dma_desc_dev_0
{
	u32 dest;
	u32 src;
	u32 size;
	u32 next;
};


//////////////////////////////////////////////////////////////////////////////////////////
int free_dma_desc_dev_0(struct acqiris_device* aq_dev, char *desc_base, int nbr_pages)
{
    int result = 0;
    int page;
    struct dma_desc_dev_0 *dma_desc = (struct dma_desc_dev_0 *)desc_base;

    for (page = 0 ; page < nbr_pages ; ++page)
    {
        dma_addr_t dma_addr;
        dma_addr_t dma_size;

        dma_addr = dma_desc[page].dest;
        dma_size = dma_desc[page].size;

        dma_unmap_page(&aq_dev->pci_dev->dev, dma_addr, dma_size, PCI_DMA_FROMDEVICE);

    }

    return result;
}

//////////////////////////////////////////////////////////////////////////////////////////
int make_dma_desc_dev_0(struct acqiris_device* aq_dev, char *desc_base, u32 local_addr, void __user *dest, size_t size, struct page **pages, int nbr_pages)
{
    int result = 0;
    unsigned long offset;
    unsigned long length;
    int page;
    struct dma_desc_dev_0 *dma_desc = (struct dma_desc_dev_0 *)desc_base;
    dma_addr_t desc_addr = virt_to_phys(desc_base);

    offset = (unsigned long)dest % PAGE_SIZE;
    length = size;

    for (page = 0 ; page < nbr_pages ; ++page)
    {
        dma_addr_t dma_addr;

        /* The structure of a DMA descriptor block (consisting of 4 ulongs) is described
         * in the PLX9080 manual in Fig. 3-17, p.32
         * The last four bits of the 'Next Descriptor Pointer' are:
         *  0x1 :   1 = PCI-address space (we don't use any local memory for descriptors)
         *  0x2 :   1 = end of chain, 0 = another descriptor follows
         *  0x4 :   1 = interrupt after terminal-count of this descriptor
         *  0x8 :   1 = read from local memory to PCI-bus, 0 = write to local memory */

        dma_addr = dma_map_page(&aq_dev->pci_dev->dev, pages[page], offset, PAGE_SIZE - offset, PCI_DMA_FROMDEVICE);

        if (page == 0)
        {
            dma_desc[page].src = local_addr;
            dma_desc[page].dest = dma_addr;
            dma_desc[page].size = (PAGE_SIZE - offset > length) ? length : PAGE_SIZE - offset;
            dma_desc[page].next = (nbr_pages > 1) ? (desc_addr + 16*(page+1) + 0x9) : 0xf;
            offset = 0;
            length -= dma_desc[page].size;
        }
        else
        {
            dma_desc[page].src = local_addr;
            dma_desc[page].dest = dma_addr;
            dma_desc[page].size = (length > PAGE_SIZE) ? PAGE_SIZE : length;
            dma_desc[page].next = (length > PAGE_SIZE) ? (int)(desc_addr + 16*(page+1) +  0x9): (int)0x0f ;
            length -= dma_desc[page].size;
        }

        // terminate if the page desc gets full !!
        if (page >= aq_dev->dma_desc_size / sizeof(struct dma_desc_dev_0))
        {
            dma_desc[page].next = 0x0f;
            if(dbgl&DDMA) printk(ACQRS_WARNING "<1>Acqiris: DMA Page descriptor limit reached.\n");
            return 0;
        }

        if(dbgl&DDMA) printk(ACQRS_NOTICE "DMA DescRd %3d dest %#010x size %#010x next %#010x\n", \
                             page, dma_desc[page].dest, dma_desc[page].size, dma_desc[page].next);

        dma_desc[page].src = cpu_to_le32(dma_desc[page].src);
        dma_desc[page].dest = cpu_to_le32(dma_desc[page].dest);
        dma_desc[page].size = cpu_to_le32(dma_desc[page].size);
        dma_desc[page].next = cpu_to_le32(dma_desc[page].next);

    }

    if (length > 0)
    {
        int nbr_desc = page;

        printk(ACQRS_WARNING "DMA DescRd remaining length 0x%lx.\n", length);
        for (page = 0 ; page < nbr_desc ; ++page)
            printk(ACQRS_WARNING "DMA DescRd %3d dest %#010x size %#010x next %#010x\n", \
                   page, dma_desc[page].dest, dma_desc[page].size, dma_desc[page].next);

    }

    return result;
}


//////////////////////////////////////////////////////////////////////////////////////////
struct dma_desc_dev_1
{
	u32 destLo;
	u32 destHi;
	u32 size;
	u32 next;
};


//////////////////////////////////////////////////////////////////////////////////////////
int free_dma_desc_dev_1(struct acqiris_device* aq_dev, char *desc_base, int nbr_pages)
{
    int result = 0;
    int page;
    struct dma_desc_dev_1 *dma_desc = (struct dma_desc_dev_1 *)desc_base;

    for (page = 0 ; page < nbr_pages ; ++page)
    {
        dma_addr_t dma_addr;
        dma_addr_t dmaSize;

        dma_addr = dma_desc[page].destHi;
        dma_addr = (dma_addr << 16) << 16;
        dma_addr = dma_addr | dma_desc[page].destLo;

        dmaSize = dma_desc[page].size;

        dma_unmap_page(&aq_dev->pci_dev->dev, dma_addr, dmaSize, PCI_DMA_FROMDEVICE);

    }

    return result;
}

//////////////////////////////////////////////////////////////////////////////////////////
int make_dma_desc_dev_1(struct acqiris_device* aq_dev, char *desc_base, u32 local_addr, void __user *dest, size_t size, struct page **pages, int nbr_pages)
{
    int result = 0;
    unsigned long offset;
    unsigned long length;
    int page;
    struct dma_desc_dev_1 *dma_desc = (struct dma_desc_dev_1 *)desc_base;
    dma_addr_t desc_addr = virt_to_phys(desc_base);

    offset = (unsigned long)dest % PAGE_SIZE;
    length = size;

    /* For device 1, local addr is not stored in descriptor. Write it to register. */
    WRITE_REG32(aq_dev->res.controlBase, DMA0MODE_OFFSET + 0x8, local_addr);

    for (page = 0 ; page < nbr_pages ; ++page)
    {
        dma_addr_t dma_addr;

        dma_addr = dma_map_page(&aq_dev->pci_dev->dev, pages[page], offset, PAGE_SIZE - offset, PCI_DMA_FROMDEVICE);

        if (page == 0)
        {
            dma_desc[page].destHi = ((dma_addr >> 16) >> 16); /* >>32 on 32 bits is noop */
            dma_desc[page].destLo = (dma_addr & 0xffffffff);
            dma_desc[page].size = (PAGE_SIZE - offset > length) ? length : PAGE_SIZE - offset;
            dma_desc[page].next = (nbr_pages > 1) ? (desc_addr + 16*(page+1)) : 0x01;
            offset = 0;
            length -= dma_desc[page].size;
        }
        else
        {
            dma_desc[page].destHi = ((dma_addr >> 16) >> 16); /* >>32 on 32 bits is noop */
            dma_desc[page].destLo = (dma_addr & 0xffffffff);
            dma_desc[page].size = (length > PAGE_SIZE) ? PAGE_SIZE : length;
            dma_desc[page].next = (length > PAGE_SIZE) ? (desc_addr + 16*(page+1)): 0x01 ;
            length -= dma_desc[page].size;
        }

        // terminate if the page desc gets full !!
        if (page >= aq_dev->dma_desc_size / sizeof(struct dma_desc_dev_1))
        {
            dma_desc[page].next = 0x01;
            if(dbgl&DDMA) printk(ACQRS_WARNING "DMA Page descriptor limit reached \n");
            return 0;
        }

        if(dbgl&DDMA) printk(ACQRS_INFO "DMA DescRd %3d dest %#010x:%#010x size %#010x next %#010x\n", \
                             page, dma_desc[page].destHi, dma_desc[page].destLo, dma_desc[page].size, dma_desc[page].next);

        dma_desc[page].destHi = cpu_to_le32(dma_desc[page].destHi);
        dma_desc[page].destLo = cpu_to_le32(dma_desc[page].destLo);
        dma_desc[page].size = cpu_to_le32(dma_desc[page].size);
        dma_desc[page].next = cpu_to_le32(dma_desc[page].next);

    }

    if (length > 0)
    {
        int nbr_desc = page;

        printk(ACQRS_WARNING "DMA DescRd remaining length 0x%lx\n", length);
        for (page = 0 ; page < nbr_desc ; ++page)
            printk(ACQRS_INFO "DMA DescRd %3d dest %#010x:%#010x size %#010x next %#010x\n", \
                   page, dma_desc[page].destHi, dma_desc[page].destLo, dma_desc[page].size, dma_desc[page].next);

    }

    return result;
}


//////////////////////////////////////////////////////////////////////////////////////////
struct dma_desc_dev_2
{
	u32 destLo;
	u32 destHi;
	u32 size;
	u32 nextLo;
    u32 nextHi;
    u32 unused;
};


//////////////////////////////////////////////////////////////////////////////////////////
int free_dma_desc_dev_2(struct acqiris_device* aq_dev, char *desc_base, int nbr_pages)
{
    int result = 0;
    int page;
    struct dma_desc_dev_2 *dma_desc = (struct dma_desc_dev_2 *)desc_base;

    for (page = 0 ; page < nbr_pages ; ++page)
    {
        dma_addr_t dma_addr;
        dma_addr_t dmaSize;

        dma_addr = dma_desc[page].destHi;
        dma_addr = (dma_addr << 16) << 16;
        dma_addr = dma_addr | dma_desc[page].destLo;

        dmaSize = dma_desc[page].size;

        dma_unmap_page(&aq_dev->pci_dev->dev, dma_addr, dmaSize, PCI_DMA_FROMDEVICE);

    }

    return result;
}

//////////////////////////////////////////////////////////////////////////////////////////
int make_dma_desc_dev_2(struct acqiris_device* aq_dev, char *desc_base, u32 local_addr, void __user *dest, size_t size, struct page **pages, int nbr_pages)
{
    int result = 0;
    unsigned long offset;
    unsigned long length;
    int page;
    struct dma_desc_dev_2 *dma_desc = (struct dma_desc_dev_2 *)desc_base;
    dma_addr_t desc_addr = virt_to_phys(desc_base);

    offset = (unsigned long)dest % PAGE_SIZE;
    length = size;

    /* For device 2, local addr is not stored in descriptor. Write it to register. */
    WRITE_REG32(aq_dev->res.controlBase, DMA0MODE_OFFSET + 0x8, local_addr);

    for (page = 0 ; page < nbr_pages ; ++page)
    {
        dma_addr_t dma_addr;

        dma_addr = dma_map_page(&aq_dev->pci_dev->dev, pages[page], offset, PAGE_SIZE - offset, PCI_DMA_FROMDEVICE);

        if (page == 0)
        {
            dma_addr_t nextAddr = (nbr_pages > 1) ? (desc_addr + 24*(page+1)) : 0x01;
            dma_desc[page].destHi = ((dma_addr >> 16) >> 16); /* >>32 on 32 bits is noop */
            dma_desc[page].destLo = (dma_addr & 0xffffffff);
            dma_desc[page].size = (PAGE_SIZE - offset > length) ? length : PAGE_SIZE - offset;
            dma_desc[page].nextHi = ((nextAddr >> 16) >> 16); /* >>32 on 32 bits is noop */
            dma_desc[page].nextLo = (nextAddr & 0xffffffff);
            dma_desc[page].unused = 0;
            offset = 0;
            length -= dma_desc[page].size;
        }
        else
        {
            dma_addr_t nextAddr = (length > PAGE_SIZE) ? (desc_addr + 24*(page+1)): 0x01 ;
            dma_desc[page].destHi = ((dma_addr >> 16) >> 16); /* >>32 on 32 bits is noop */
            dma_desc[page].destLo = (dma_addr & 0xffffffff);
            dma_desc[page].size = (length > PAGE_SIZE) ? PAGE_SIZE : length;
            dma_desc[page].nextHi = ((nextAddr >> 16) >> 16); /* >>32 on 32 bits is noop */
            dma_desc[page].nextLo = (nextAddr & 0xffffffff);
            length -= dma_desc[page].size;
        }

        // terminate if the page desc gets full !!
        if (page >= aq_dev->dma_desc_size / sizeof(struct dma_desc_dev_2))
        {
            dma_desc[page].nextLo = 0x01;
            dma_desc[page].nextHi = 0x00;
            if(dbgl&DDMA) printk(ACQRS_WARNING "DMA Page descriptor limit reached \n");
            return 0;
        }

        if(dbgl&DDMA) printk(ACQRS_INFO "DMA DescRd %3d dest %#010x:%#010x size %#010x next %#010x:%#010x\n", page, dma_desc[page].destHi, \
                    dma_desc[page].destLo, dma_desc[page].size, dma_desc[page].nextHi, dma_desc[page].nextLo);

        dma_desc[page].destHi = cpu_to_le32(dma_desc[page].destHi);
        dma_desc[page].destLo = cpu_to_le32(dma_desc[page].destLo);
        dma_desc[page].size = cpu_to_le32(dma_desc[page].size);
        dma_desc[page].nextHi = cpu_to_le32(dma_desc[page].nextHi);
        dma_desc[page].nextLo = cpu_to_le32(dma_desc[page].nextLo);

    }

    if (length > 0)
    {
        int nbr_desc = page;

        printk(ACQRS_WARNING "DMA DescRd remaining length 0x%lx\n", length);
        for (page = 0 ; page < nbr_desc ; ++page)
            printk(ACQRS_INFO "DMA DescRd %3d dest %#010x:%#010x size %#010x next %#010x:%#010x\n", page, dma_desc[page].destHi, \
                    dma_desc[page].destLo, dma_desc[page].size, dma_desc[page].nextHi, dma_desc[page].nextLo);

    }

    return result;
}

