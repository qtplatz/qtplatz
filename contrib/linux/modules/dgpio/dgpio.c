/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#include "dgpio.h"
#include "dgpio_ioctl.h"
#include <linux/cdev.h>
#include <linux/ctype.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/ioctl.h>
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/parport.h>
#include <linux/pci.h>
#include <linux/proc_fs.h> // create_proc_entry
#include <linux/seq_file.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

static char *devname = MODNAME;

MODULE_AUTHOR( "Toshinobu Hondo" );
MODULE_DESCRIPTION( "Device Driver for MULTUM-II Control Delay Pulse Generator" );
MODULE_LICENSE("GPL");
MODULE_PARM_DESC(devname, "dgpio param");
module_param( devname, charp, S_IRUGO );

#define countof(x) (sizeof(x)/sizeof((x)[0]))

static struct pci_device_id __ids[] = {
    { PCI_DEVICE( 0x9710 , 0x9900 ), }, // MCS9900CV-AA, NetMOS Single Parallel Port Card
    // { PCI_DEVICE( 0x8086, 0x1e22 ), }, // debug (GBE)
    { 0, }
};

MODULE_DEVICE_TABLE( pci, __ids );

struct dgpio_resource {
    struct resource res;
    void * iomem;
};

struct dgpio_driver {
    struct dgpio_resource port[2];
};

static struct dgpio_driver * __instance;

static int __debug_level__ = 0;

static uint8_t __last_data = 0;

static inline void
enable_bidirectional_port( struct dgpio_driver * drv )
{
    if ( drv && drv->port[ 0 ].iomem ) {
        uint8_t cntl = inb( drv->port[ 0 ].res.start + 2 ) | 0x20; // Enable bidirectional
        outb( cntl, drv->port[ 0 ].res.start + 2 );
    }
}

static int
probe( struct pci_dev *dev, const struct pci_device_id *id )
{
    dev_info( &dev->dev, "probe pci device '%s' [0x%04x:0x%04x] 0x%x\n"
              , dev->bus->name, dev->vendor, dev->device, dev->class );

    __instance = devm_kzalloc( &dev->dev, sizeof( struct dgpio_driver ), GFP_KERNEL );
    dev_set_drvdata( &dev->dev, __instance );

    if ( pci_enable_device( dev ) != 0 )
        dev_info( &dev->dev, "pci_enable_device failed" );

    u32 nio = 0;
    for ( int bar = 0; bar < 8; ++bar ) {

        u32 flags = pci_resource_flags( dev, bar );

        if ( ( flags & IORESOURCE_IO ) && ( nio < countof( __instance->port ) ) )  {
            void * port = pci_iomap( dev, bar, 0 );
            if ( port ) {
                __instance->port[ nio ].res.start = pci_resource_start( dev, bar );
                __instance->port[ nio ].res.end = pci_resource_end( dev, bar );
                __instance->port[ nio ].iomem = port;

                dev_info( &dev->dev, "bar=%d, port[%d] = %p, resource = <%llx, %llx>, %d\n"
                          , bar
                          , nio
                          , port
                          , __instance->port[ nio ].res.start
                          , __instance->port[ nio ].res.end
                          , (int)( __instance->port[ nio ].res.end - __instance->port[ nio ].res.start + 1 ) );

                
                ++nio;
            }
        }
    }
    
    enable_bidirectional_port( __instance );
    
    // print_pci_information( dev );
    return 0;
}

static void
remove(struct pci_dev * dev)
{
    dev_info( &dev->dev, "dgpio remove pci device [0x%x:0x%x] 0x%x\n", dev->vendor, dev->device, dev->class );
    
    pci_disable_device(dev);
}

static struct pci_driver __pci_driver = {
     .name = MODNAME,
     .id_table = __ids,
     .probe = probe,
     .remove = remove,
};

static int
dgpio_proc_read( struct seq_file * m, void * v )
{
    struct dgpio_driver * drv = __instance;
    if ( drv && drv->port[ 0 ].iomem ) {
        
        seq_printf( m, "iobase[0x%llx]: ", drv->port[ 0 ].res.start );
        for ( int i = 0; i < ( drv->port[ 0 ].res.end - drv->port[ 0 ].res.start + 1 ); ++i ) {
            uint8_t data = inb( drv->port[ 0 ].res.start + i );
            seq_printf( m, "0x%02x, ", data );
        }
        seq_printf( m, "\n" );

        /* seq_printf( m, "iomem[0x%p]: ", drv->mmap[ 0 ].iomem ); */
        /* const u32 * p = drv->mmap[ 0 ].iomem; */
        /* for ( int i = 0; i < 8; ++i ) { */
        /*     seq_printf( m, ", 0x%08x", *p ); */
        /* } */
        /* seq_printf( m, "\n" ); */
        
    } else {
        seq_printf( m, "device unavilable\n" );
    }
    return 0;
}

static ssize_t
dgpio_proc_write( struct file * filep, const char * user, size_t size, loff_t * f_off )
{
    static char readbuf[256];
    
    if ( size >= sizeof( readbuf ) )
        size = sizeof( readbuf ) - 1;
    
    if ( copy_from_user( readbuf, user, size ) )
        return -EFAULT;
    
    readbuf[ size ] = '\0';

    if ( strncmp( readbuf, "debug", 5 ) == 0 ) {
        
        unsigned long value = 0;
        const char * rp = &readbuf[5];
        while ( *rp && !isdigit( *rp ) )
            ++rp;
        if ( kstrtoul( rp, 10, &value ) == 0 )
            __debug_level__ = value;
        
        printk( KERN_INFO "" MODNAME " debug level is %d\n", __debug_level__ );
        
    } else {
        if ( __debug_level__ > 0 )
            printk( KERN_INFO "" MODNAME " proc write received unknown command[%ld]: %s.\n", size, readbuf );
    }
    return size;
}

static int
dgpio_proc_open( struct inode * inode, struct file * file )
{
    return single_open( file, dgpio_proc_read, NULL );
}

static const struct file_operations proc_file_fops = {
    .owner   = THIS_MODULE,
    .open    = dgpio_proc_open,
    .read    = seq_read,
    .write   = dgpio_proc_write,
    .llseek  = seq_lseek,
    .release = single_release,
};

static int dgpio_cdev_open(struct inode *inode, struct file *file)
{
    if ( __debug_level__ )
        printk(KERN_INFO "open dgpio char device\n");
    
    return 0;
}

static int dgpio_cdev_release(struct inode *inode, struct file *file)
{
    if ( __debug_level__ ) 
        printk(KERN_INFO "release dgpio char device\n");

    return 0;
}

static long dgpio_cdev_ioctl( struct file* file, unsigned int code, unsigned long args)
{
    const static int version = IOCTL_VERSION;

    switch( code ) {

    case DGPIO_GET_VERSION:
        if ( copy_to_user( (char *)(args), ( const void * )(&version), sizeof( int ) ) )
            return -EFAULT;        
        break;
    case DGPIO_GET_DATA:
        if ( __instance && __instance->port[ 0 ].iomem ) {
            __last_data = inb( __instance->port[ 0 ].res.start + 0 );
            if ( copy_to_user( (char *)(args), ( const void * )(&__last_data), sizeof( __last_data ) ) )
                return -EFAULT;
        }
        break;
    case DGPIO_SET_DATA:
        if ( copy_from_user( (char *)(&__last_data), (const void *)(args), sizeof( __last_data ) ) )
            return -EFAULT;
        if ( __instance && __instance->port[ 0 ].iomem )
            outb( __last_data, __instance->port[ 0 ].res.start );
        break;                
    }
    return 0;
}

static ssize_t
dgpio_cdev_read(struct file * file, char __user * data, size_t size, loff_t * f_pos )
{
    if ( __instance ) { // && ( *f_pos == 0 ) ) {

        struct dgpio_driver * drv = __instance;

        if ( drv->port[ 0 ].iomem ) {
            uint8_t b = inb( drv->port[ 0 ].res.start + 0 );

            if ( copy_to_user( data, ( const void * )(&b), 1 ) )
                return -EFAULT;

            (*f_pos)++;

            return sizeof(b);
        }
    }
    return 0;
}

static ssize_t
dgpio_cdev_write(struct file *file, const char __user *data, size_t size, loff_t *f_pos)
{
    return 0;
}

static struct file_operations dgpio_cdev_fops = {
    .owner   = THIS_MODULE,
    .open    = dgpio_cdev_open,
    .release = dgpio_cdev_release,
    .unlocked_ioctl = dgpio_cdev_ioctl,
    .read    = dgpio_cdev_read,
    .write   = dgpio_cdev_write,
};

static dev_t dgpio_dev_t = 0;
static struct cdev * __dgpio_cdev;
static struct class * __dgpio_class;

static int dgpio_dev_uevent( struct device * dev, struct kobj_uevent_env * env )
{
    add_uevent_var( env, "DEVMODE=%#o", 0644 );
    return 0;
}

static int dgpio_dtor( int errno )
{
    if ( __dgpio_class ) {
        class_destroy( __dgpio_class );
        printk( KERN_INFO "" MODNAME " class_destry done\n" );
    }

    if ( __dgpio_cdev ) {
        cdev_del( __dgpio_cdev );
        printk( KERN_INFO "" MODNAME " cdev_del done\n" );
    }

    if ( dgpio_dev_t ) {
        unregister_chrdev_region( dgpio_dev_t, 1 );
        printk( KERN_INFO "" MODNAME " unregister_chrdev_region done\n" );
    }

    return errno;
}

static int __init
dgpio_module_init( void )
{
    printk( KERN_INFO "" MODNAME " driver %s loaded\n", MOD_VERSION );
    
    // DEVICE
    if ( alloc_chrdev_region(&dgpio_dev_t, 0, 1, "dgpio-cdev" ) < 0 ) {
        printk( KERN_ERR "" MODNAME " failed to alloc chrdev region\n" );
        return -1;
    }
    
    __dgpio_cdev = cdev_alloc();
    if ( !__dgpio_cdev ) {
        printk( KERN_ERR "" MODNAME " failed to alloc cdev\n" );
        return -ENOMEM;
    }

    cdev_init( __dgpio_cdev, &dgpio_cdev_fops );
    if ( cdev_add( __dgpio_cdev, dgpio_dev_t, 1 ) < 0 ) {
        printk( KERN_ERR "" MODNAME " failed to add cdev\n" );
        return dgpio_dtor( -1 );
    }

    __dgpio_class = class_create( THIS_MODULE, "dgpio" );
    if ( !__dgpio_class ) {
        printk( KERN_ERR "" MODNAME " failed to create class\n" );
        return dgpio_dtor( -1 );
    }
    __dgpio_class->dev_uevent = dgpio_dev_uevent;

    // make_nod /dev/dgpio0
    if ( !device_create( __dgpio_class, NULL, dgpio_dev_t, NULL, "dgpio%d", MINOR( dgpio_dev_t ) ) ) {
        printk( KERN_ERR "" MODNAME " failed to create device\n" );
        return dgpio_dtor( -1 );
    }

    // /proc create
    proc_create( "dgpio", 0666, NULL, &proc_file_fops );

    // -- pci --
    do {
        int result = 0;
        if ( ( result = pci_register_driver(&__pci_driver) ) != 0 ) {
            printk(KERN_ERR "Could not register the " MODNAME " PCI driver(%d).\n", result );
            pci_unregister_driver(&__pci_driver);
            // return -EIO;          
        }
        printk(KERN_INFO "pci_register_driver success (%d).\n", result );
    } while ( 0 );

    return 0;
}

static void
dgpio_module_exit( void )
{
    pci_unregister_driver(&__pci_driver);
    
    //---
    remove_proc_entry( "dgpio", NULL ); // proc_create

    device_destroy( __dgpio_class, dgpio_dev_t ); // device_creeate

    class_destroy( __dgpio_class ); // class_create

    cdev_del( __dgpio_cdev ); // cdev_alloc, cdev_init

    unregister_chrdev_region( dgpio_dev_t, 1 ); // alloc_chrdev_region
    //
    printk( KERN_INFO "" MODNAME " driver %s unloaded\n", MOD_VERSION );
}

module_init( dgpio_module_init );
module_exit( dgpio_module_exit );
