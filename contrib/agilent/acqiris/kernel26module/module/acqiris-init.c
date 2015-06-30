//////////////////////////////////////////////////////////////////////////////////////////
//
//  LinuxConfigPCI.c    Implementation of Configuration for PCI9080-based Acqiris Devices
//
//----------------------------------------------------------------------------------------
//  Copyright Agilent Technologies, Inc. 2000, 2001-2009
//
//  $Id: acqiris-init.c 37124 2009-11-12 10:02:50Z bdonnier $
//
//  Started:     6 JUN 2000
//      26/06/01 esch: set PCI bus master, needed for HP
//
//  Owned by:   V. Hungerbuhler
//
//////////////////////////////////////////////////////////////////////////////////////////
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/pagemap.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/types.h>

#include <asm/uaccess.h>

#include "acqiris.h"


#if LINUX_VERSION_CODE < 0x20616
// SA_* flags have been deprecated and replaced by IRQF_* flags
// since version 2.6.22
#define IRQF_SHARED (SA_SHIRQ | SA_INTERRUPT)
#endif


//////////////////////////////////////////////////////////////////////////////////////////
// Declarations of module and parameters

MODULE_AUTHOR("Viktor Hungerbuhler, Erich Schaefer & Didier Trosset");
MODULE_DESCRIPTION("Device Driver for Agilent - Acqiris Data Converters");
MODULE_LICENSE("Proprietary");

// Linux kernel class types are available only to GPL licenced modules
// since version 2.6.17
//#define AQ_USE_CLASSES


int dbgl = 0;
int major = 0;
int nbrdev = 0;
int maxdev = 15;

module_param(dbgl, int, 0644);
module_param(major, int, 0444);
module_param(nbrdev, int, 0444);
module_param(maxdev, int, 0444);



//////////////////////////////////////////////////////////////////////////////////////////
// Internal variables

static struct pci_device_id acqiris_pci_tbl[] = {
    { PCI_VENDOR_ID_ACQIRIS, PCI_DEVICE_ID_ACQIRIS_0, PCI_ANY_ID, PCI_ANY_ID, },
    { PCI_VENDOR_ID_ACQIRIS, PCI_DEVICE_ID_ACQIRIS_1, PCI_ANY_ID, PCI_ANY_ID, },
    { PCI_VENDOR_ID_ACQIRIS, PCI_DEVICE_ID_ACQIRIS_2, PCI_ANY_ID, PCI_ANY_ID, },
    { PCI_VENDOR_ID_ADLINK,  PCI_DEVICE_ID_ADLINK_0,  PCI_ANY_ID, PCI_ANY_ID, },
    { PCI_VENDOR_ID_ADLINK,  PCI_DEVICE_ID_ADLINK_1,  PCI_ANY_ID, PCI_ANY_ID, },
    { PCI_VENDOR_ID_PLXTECH, PCI_DEVICE_ID_PLXTECH_0, PCI_ANY_ID, PCI_ANY_ID, },
    { PCI_VENDOR_ID_PLXTECH, PCI_DEVICE_ID_PLXTECH_1, PCI_ANY_ID, PCI_ANY_ID, },
    { 0, 0, 0, 0 }
};

MODULE_DEVICE_TABLE(pci, acqiris_pci_tbl);

static struct pci_driver aq_pci_driver =
{
    .name       = MOD_NAME,
    .id_table   = acqiris_pci_tbl,
    .probe      = acqiris_device_probe,
    .remove     = acqiris_device_remove,
};


static struct file_operations aq_fops =
{
    .owner          = THIS_MODULE,
#ifdef CONFIG_COMPAT
    .compat_ioctl   = acqiris_ioctl_compat,
#endif
#if HAVE_UNLOCKED_IOCTL
    .unlocked_ioctl = acqiris_ioctl_unlocked,
#else
    .ioctl          = acqiris_ioctl,
#endif
    .open           = acqiris_open,
    .release        = acqiris_release,
};

struct acqiris_driver aq_drv;

#ifdef AQ_USE_CLASSES
static struct class *acqiris_classP = NULL;
static struct device *acqiris_deviceP = NULL;
#endif // AQ_USE_CLASSES


//////////////////////////////////////////////////////////////////////////////////////////
static int __init acqiris_driver_init(struct acqiris_driver *aq_drv)
{
    aq_drv->nbr_devices = 0;

    if (maxdev > MAX_DEVICES)
        printk(ACQRS_WARNING "Maximum number of device 'maxdev' is limited to %d\n", maxdev=MAX_DEVICES);

    aq_drv->max_devices = maxdev;
    aq_drv->devices = kmalloc(maxdev * sizeof(struct acqiris_device *), GFP_KERNEL);
    if (!aq_drv->devices)
        return printk(ACQRS_ERR "Cannot allocate space for array of devices.\n"), -ENOMEM;

    memset(aq_drv->devices, 0, maxdev * sizeof(struct acqiris_device *));

    aq_drv->dev = MKDEV(major, CDEV_MINOR_FIRST);
    cdev_init(&aq_drv->cdev, &aq_fops);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
static void acqiris_driver_cleanup(struct acqiris_driver *aq_drv)
{
    if (aq_drv->devices)
        kfree(aq_drv->devices);

}


//////////////////////////////////////////////////////////////////////////////////////////
static int __init acqiris_init(void)
{
    int result;

    printk(ACQRS_INFO "Initializing module " MOD_NAME " (Acqiris PCI I/O Driver) version " MOD_VERSION "\n");
    printk(ACQRS_INFO "    compiled for linux kernel version %d.%d.%d\n", (LINUX_VERSION_CODE>>16)&0xff, (LINUX_VERSION_CODE>>8)&0xff, LINUX_VERSION_CODE&0xff);
    printk(ACQRS_INFO "- debug level = %#04x (%d) (modprobe " MOD_NAME " dbgl=N)\n", dbgl, dbgl);
    printk(ACQRS_INFO "    N: 0x1 DMA; 0x2 interrupt function; 0x4 module r/w ioctrl; 0x8 end of acquisition \n");
    printk(ACQRS_INFO "    N: 0x10 initialization; 0x20 memory allocations; 0x40 all operations \n");
    printk(ACQRS_INFO "- maximum devices = %d (modprobe " MOD_NAME " maxdev=N) (absolute max is 255)\n", maxdev);

    /* Creating the acqiris driver structure */
    result = acqiris_driver_init(&aq_drv);
    if (result < 0)
        return result;

    /* Attempt to register major region */
    if (MAJOR(aq_drv.dev) > 0)
        result = register_chrdev_region(aq_drv.dev, CDEV_MINOR_COUNT, MOD_NAME);
    else
        result = alloc_chrdev_region(&aq_drv.dev, CDEV_MINOR_FIRST, CDEV_MINOR_COUNT, MOD_NAME);

    if (result < 0)
    {
        printk(ACQRS_ERR "Cannot register chrdev region for major %d (%d).\n", MAJOR(aq_drv.dev), result);
        goto fail_region;
    }

    printk(ACQRS_INFO "- major number = %d (modprobe " MOD_NAME " major=N)\n", MAJOR(aq_drv.dev));

    /* Attempt to add character device */
    result = cdev_add(&aq_drv.cdev, aq_drv.dev, CDEV_MINOR_COUNT);
    if (result < 0)
    {
        printk(ACQRS_ERR "Unable to add character device.\n");
        kobject_put(&aq_drv.cdev.kobj);
        goto fail_cdev;
    }

    /* Attempt to register PCI driver */
    result = pci_register_driver(&aq_pci_driver);
    if (result < 0)
    {
        printk(ACQRS_ERR "Cannot register PCI driver (%d).\n", result);
        goto fail_pci;
    }

#ifdef AQ_USE_CLASSES
    acqiris_classP = class_create(THIS_MODULE, MOD_NAME);

    if (acqiris_classP == NULL)
    {
        printk(ACQRS_ERR "Unable to create class '%s'\n", MOD_NAME);
        result = -ENODEV;
        goto fail_all;
     }

    device_create(acqiris_classP, acqiris_deviceP, aq_drv.dev, MOD_NAME);

#endif

    nbrdev = aq_drv.nbr_devices;

    /* All is OK */
    return 0;

    goto fail_all; /* to prevent warning */
fail_all:
    pci_unregister_driver(&aq_pci_driver);
fail_pci:
    cdev_del(&aq_drv.cdev);
fail_cdev:
    unregister_chrdev_region(aq_drv.dev, CDEV_MINOR_COUNT);
fail_region:
    acqiris_driver_cleanup(&aq_drv);

    return result;
}

module_init(acqiris_init);


//////////////////////////////////////////////////////////////////////////////////////////
static void __exit acqiris_cleanup(void)
{
    if (dbgl&DINIT) printk(ACQRS_INFO "Cleaning module %s  \n", MOD_NAME);

    /* Attempt to unregister PCI driver */
    pci_unregister_driver(&aq_pci_driver);

#ifdef AQ_USE_CLASSES
    device_destroy(acqiris_classP, aq_drv.dev);
    class_destroy(acqiris_classP);
#endif

    /* Attempt to delete character device */
    cdev_del(&aq_drv.cdev);

    /* Attempt to unregister major region */
    unregister_chrdev_region(aq_drv.dev, CDEV_MINOR_COUNT);

    /* Destroying the acqiris driver structure */
    acqiris_driver_cleanup(&aq_drv);

    printk(ACQRS_INFO "Leaving module %s.\n", MOD_NAME);

}

module_exit(acqiris_cleanup);


//////////////////////////////////////////////////////////////////////////////////////////
int acqiris_open(struct inode* inode, struct file* filp)
{
    struct acqiris_driver *drv;
    struct acqiris_device *aq_dev;
    char buffer[20];
    int minor;
    int dev_num;

    minor = MINOR(inode->i_rdev);

    if (dbgl&DINIT) printk(ACQRS_INFO "acqiris_open(%s)\n", format_dev_t(buffer, inode->i_rdev));

    /* Opening the main device is always possible */
    if (minor == 0)
    {
        drv = container_of(inode->i_cdev, struct acqiris_driver, cdev);
        filp->private_data = drv;

        return 0;
    }

    dev_num = minor - 1;
    if (!aq_drv.devices || dev_num < 0 || dev_num >= aq_drv.max_devices)
        return -ENXIO;

    aq_dev = aq_drv.devices[dev_num];

    if (aq_dev == NULL)
        return -ENXIO;

    /* if dev is already assigned, it is already open: refuse */
    if (aq_dev->dev != 0)
        return -EBUSY;

    aq_dev->dev = inode->i_rdev;
    filp->private_data = aq_dev;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
int acqiris_release(struct inode* inode, struct file* filp)
{
    struct acqiris_device *aq_dev;
    char buffer[20];
    int minor;

    minor = MINOR(inode->i_rdev);

    if (dbgl&DINIT) printk(ACQRS_INFO "acqiris_release(%s)\n", format_dev_t(buffer, inode->i_rdev));

    /* Closing the main device is always possible */
    if (minor == 0)
        return 0;

    aq_dev = filp->private_data;
    aq_dev->dev = 0;

    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
int /* __devinit */ acqiris_device_probe(struct pci_dev *pci_dev, const struct pci_device_id *pci_id)
{
    struct acqiris_device* aq_dev;
    struct page *page;
    char name_device[32];
    int index_to_direct = 0;
    int retcode;

    snprintf(name_device, sizeof(name_device), "%04x:%04x, slot %d, bus %d", \
            pci_dev->vendor, pci_dev->device, PCI_SLOT(pci_dev->devfn), pci_dev->bus->number);

    printk(ACQRS_INFO "Found PCI device %s.\n", name_device);

    if (aq_drv.nbr_devices >= aq_drv.max_devices)
        return printk(ACQRS_ERR "Cannot drive more than %d devices. Use maxdev=<N> when loading module.\n", aq_drv.max_devices), -EIO;

    // Get extension memory for it
    aq_dev = kmalloc(sizeof(struct acqiris_device), GFP_KERNEL);
    aq_drv.devices[aq_drv.nbr_devices] = aq_dev;
    pci_set_drvdata(pci_dev, aq_dev);

    if (aq_dev == NULL)
        return printk(ACQRS_ERR "Cannot allocate memory for device %s.\n", name_device), -ENOMEM;

    // Enable the device
    if (pci_enable_device(pci_dev))
        return printk(ACQRS_ERR "Cannot enable device %s.\n", name_device), -EIO;

    pci_set_master(pci_dev);

    // Device info
    aq_dev->dev = 0;
    aq_dev->dev_num = aq_drv.nbr_devices;
    aq_dev->pci_dev = pci_dev;
    aq_dev->devfn = pci_dev->devfn;
    aq_dev->vendor_id = pci_dev->vendor;
    aq_dev->device_id = pci_dev->device;
    aq_dev->res.busNumber = (long)pci_dev->bus->number;
    aq_dev->res.devNumber = (long)PCI_SLOT(pci_dev->devfn);

    strcpy(aq_dev->res.name, "acqiris");

    // Control addresses
    aq_dev->res.controlAddr = pci_resource_start(pci_dev, 0);
    aq_dev->res.controlSize = pci_resource_len(pci_dev, 0);
    aq_dev->res.controlBase = 0;

    if (aq_dev->res.controlAddr != 0)
    {
        request_mem_region(aq_dev->res.controlAddr, aq_dev->res.controlSize, MOD_NAME);
        aq_dev->res.controlBase = (unsigned long)ioremap_nocache(aq_dev->res.controlAddr, aq_dev->res.controlSize);
    }

    if (dbgl&DINIT) printk(ACQRS_INFO "  Control addresses (BAR0) %p %p (%p).\n",
            (void *)aq_dev->res.controlAddr, (void *)aq_dev->res.controlSize, (void *)aq_dev->res.controlBase);

    // Direct addresses
    if ((pci_resource_flags(pci_dev, 1) & IORESOURCE_IO) && (pci_resource_flags(pci_dev, 2) & IORESOURCE_MEM))
        index_to_direct = 2;
    else if ((pci_resource_flags(pci_dev, 1) & IORESOURCE_MEM))
        index_to_direct = 1;
    else
        printk(ACQRS_ERR "Invalid IO Resources.\n" );

    if (index_to_direct > 0)
    {
        aq_dev->res.directAddr  = pci_resource_start(pci_dev, index_to_direct);
        aq_dev->res.directSize  = pci_resource_len(pci_dev, index_to_direct); // Acqiris Module memory addr space
        aq_dev->res.directBase = 0;

        if (aq_dev->res.directAddr != 0)
        {
            request_mem_region(aq_dev->res.directAddr, aq_dev->res.directSize, MOD_NAME);
            aq_dev->res.directBase = (unsigned long)ioremap_nocache(aq_dev->res.directAddr, aq_dev->res.directSize);
        }

        if (dbgl&DINIT) printk(ACQRS_INFO "  Direct addresses (BAR%d) %p %p (%p).\n", index_to_direct,
                (void *)aq_dev->res.directAddr, (void *)aq_dev->res.directSize, (void *)aq_dev->res.directBase);
    }


    // Allocate descriptor space
    aq_dev->dma_desc_size = (pci_dev->device == 2) ? DMA_DESC_SIZE_2 : DMA_DESC_SIZE_01;
    aq_dev->dma_desc = kmalloc(aq_dev->dma_desc_size, GFP_KERNEL | GFP_DMA);
    aq_dev->pages = kmalloc(1024 * sizeof(struct page *), GFP_KERNEL);

    if (dbgl&DMEM) printk(ACQRS_INFO "kmalloc DMA descriptor buffer %p \n", aq_dev->dma_desc);

    if (aq_dev->dma_desc == NULL)
        printk(ACQRS_ERR "Cannot kmalloc DMA descriptor memory.\n");

    // now we've got DMA_DESC_SIZE bytes of kernel memory, but it can still be
    // swapped out. We need to stop the VM system from removing our
    // pages from main memory. To do this we just need to set the PG_reserved
    // bit on each page, via mem_map_reserve() macro.

    for (page = virt_to_page(aq_dev->dma_desc) ; page <= virt_to_page(aq_dev->dma_desc + aq_dev->dma_desc_size - 1) ; ++page)
        SetPageReserved(page);

    if (dbgl&DMEM) printk(ACQRS_INFO "kmalloc DMA descriptor buffer %p \n", aq_dev->dma_desc);

    // Internal kernel objects for interrupts
    init_waitqueue_head(&aq_dev->wait_acq);
    init_waitqueue_head(&aq_dev->wait_proc);
    init_waitqueue_head(&aq_dev->wait_dma);

    // Interrupts
    aq_dev->res.interrupt       = pci_dev->irq;
    aq_dev->res.intrptLocalAddr = 0;
    pci_write_config_byte(pci_dev, PCI_INTERRUPT_LINE, aq_dev->res.interrupt);
    pci_write_config_byte(pci_dev, PCI_COMMAND, 0x07);

    if (dbgl&DINIT) printk(ACQRS_INFO "  Interrupt irq %d.\n", pci_dev->irq);

    WRITE_REG32(aq_dev->res.controlBase, INTSRC_OFFSET, 0x00000000);
    retcode = request_irq(aq_dev->res.interrupt, &acqiris_interrupt_handler, IRQF_SHARED, MOD_NAME , aq_dev);
    if (retcode != 0)
        printk(ACQRS_ERR "Cannot request IRQ %d for device %s: error %d.\n", aq_dev->res.interrupt, name_device, retcode);

    ++aq_drv.nbr_devices;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
void /* __devexit */ acqiris_device_remove(struct pci_dev *pci_dev)
{
    struct acqiris_device *aq_dev;
    struct page *page;
    char name_device[32];

    snprintf(name_device, sizeof(name_device), "%04x:%04x, slot %d, bus %d", \
            pci_dev->vendor, pci_dev->device, PCI_SLOT(pci_dev->devfn), pci_dev->bus->number);

    aq_dev = pci_get_drvdata(pci_dev);

    if(dbgl&DINIT) printk(ACQRS_INFO "Removing PCI device %s (%p).\n", name_device, aq_dev);

    if (aq_dev == NULL)
    {
        printk(ACQRS_ERR "Cannot release resources for device %s.\n", name_device);
    }
    else
    {
        if (aq_dev->res.controlAddr != 0)
        {
            iounmap((void *)aq_dev->res.controlBase);
            release_mem_region(aq_dev->res.controlAddr, aq_dev->res.controlSize);
        }

        if (aq_dev->res.directAddr != 0)
        {
            iounmap((void *)aq_dev->res.directBase);
            release_mem_region(aq_dev->res.directAddr, aq_dev->res.directSize);
        }

        if (dbgl&DMEM) printk(ACQRS_INFO "kfree DMA descriptor buffer %p \n", aq_dev->dma_desc);

        for (page = virt_to_page(aq_dev->dma_desc) ; page <= virt_to_page(aq_dev->dma_desc + aq_dev->dma_desc_size - 1) ; ++page)
            ClearPageReserved(page);

        kfree(aq_dev->dma_desc);
        kfree(aq_dev->pages);

        free_irq(aq_dev->res.interrupt, aq_dev);

        if (aq_drv.devices && aq_dev->dev_num >= 0 && aq_dev->dev_num < aq_drv.max_devices)
            aq_drv.devices[aq_dev->dev_num] = NULL;
        pci_set_drvdata(pci_dev, NULL);
        kfree(aq_dev);

    }

    pci_disable_device(pci_dev);

}


