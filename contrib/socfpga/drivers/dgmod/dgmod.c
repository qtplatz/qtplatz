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

#include "dgmod.h"
#include "dgmod_delay_pulse.h"
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h> // create_proc_entry
#include <linux/seq_file.h>
#include <linux/string.h>
#include <linux/uaccess.h>

static char *devname = MODNAME;

MODULE_AUTHOR( "Toshinobu Hondo, Matsuoka" );
MODULE_DESCRIPTION( "Device Driver for MULTUM-II Control Delay Pulse Generator" );
MODULE_LICENSE("GPL");
MODULE_PARM_DESC(devname, "dgmod param");
module_param( devname, charp, S_IRUGO );

#define countof(x) (sizeof(x)/sizeof((x)[0]))

//static struct dgmod_device __device;

static struct resource * __resource;
static uint32_t * __mapped_ptr;

enum {
    map_base_addr = 0xff200000
    , map_size = 0x20000
    , pio_base = 0x10040
    , addr_machine_state = 0x10100 / sizeof(uint32_t)
    , addr_submit = 0x10120 / sizeof(uint32_t)
    , addr_interval = 0x10180 / sizeof(uint32_t)
    , addr_revision = 0x101a0 / sizeof(uint32_t)
};

enum fsm_state {
    fsm_stop = 0x0000
    , fsm_start = 0x0001
    , fsm_update = 0x0001
};

struct pulse_addr {  uint32_t delay; uint32_t width; };

static struct pulse_addr pulse_register [] = {
    {   (0x10200u / sizeof( uint32_t )), (0x10220u / sizeof( uint32_t )) }
    , { (0x10240u / sizeof( uint32_t )), (0x10260u / sizeof( uint32_t )) }
    , { (0x10280u / sizeof( uint32_t )), (0x102a0u / sizeof( uint32_t )) }
    , { (0x102c0u / sizeof( uint32_t )), (0x102e0u / sizeof( uint32_t )) }
    , { (0x103c0u / sizeof( uint32_t )), (0x103e0u / sizeof( uint32_t )) }
    , { (0x10400u / sizeof( uint32_t )), (0x10420u / sizeof( uint32_t )) }
};

static struct dgmod_protocol_sequence __protocol_sequence = {
    .size_ = 1,
    .protocols_[ 0 ].replicates_ = (-1),
    .protocols_[ 0 ].delay_pulses_ = { { 0, 1 }, { 0, 1 }, { 0, 1 }, { 0, 1 }, { 0, 1 }, { 0, 1 } },
};

static irqreturn_t
handle_interrupt(int irq, void *dev_id)
{
    if ( irq != IRQ_NUM )
        return IRQ_NONE;
    
    printk(KERN_ALERT "Interrupt %d occured\n",IRQ_NUM);  

    return IRQ_HANDLED;
}

static int
dgmod_proc_read( struct seq_file * m, void * v )
{
    seq_printf( m, "Hello, Welcome to DGMOD.\n" );

    if ( __mapped_ptr ) {

        seq_printf( m
                    , "Delay pulse generator Rev. %x\n\tTo: 0x%08x\tSubmission: 0x%04x\n"
                    , __mapped_ptr[ addr_revision ]
                    , __mapped_ptr[ addr_interval ]
                    , __mapped_ptr[ addr_submit ] );
        
        for ( size_t i = 0; i < countof( pulse_register ); ++i ) {
            uint32_t delay = __mapped_ptr[ pulse_register[ i ].delay ];
            uint32_t width = __mapped_ptr[ pulse_register[ i ].width ];
            seq_printf( m, "CH-%d: 0x%08x, 0x%08x\n", i, delay, width );
        }
    }
    
    if ( __protocol_sequence.size_ > 0 ) {
        seq_printf( m, "Has %d protocol sequence\n", __protocol_sequence.size_ );
        
        for ( size_t i = 0; i < __protocol_sequence.size_; ++i ) {
            seq_printf( m, "[%d] Replicates: %d\n", i, __protocol_sequence.protocols_[ i ].replicates_ );
            for ( size_t ch = 0; ch < number_of_channels; ++ch )
                seq_printf( m
                            , "\t[CH-%d] 0x%08x, 0x%08x\n"
                            , ch
                            , __protocol_sequence.protocols_[ i ].delay_pulses_[ ch ].delay_
                            , __protocol_sequence.protocols_[ i ].delay_pulses_[ ch ].width_ );
        }

    }
    
    return 0;
}

static int
dgmod_proc_open( struct inode * inode, struct file * file )
{
    return single_open( file, dgmod_proc_read, NULL );
}

static const struct file_operations proc_file_fops = {
    .owner   = THIS_MODULE,
    .open    = dgmod_proc_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};

static int dgmod_cdev_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "open dgmod char device\n");
    return 0;
}

static int dgmod_cdev_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "release dgmod char device\n");
    return 0;
}

static ssize_t dgmod_cdev_read(struct file *file, char __user *data, size_t size, loff_t *f_pos )
{
    size_t count;

    printk(KERN_INFO "dgmod_cdev_read size=%d, offset=%lld\n", size, *f_pos );
    // todo: down_interruptible( &dev->sem )

    if ( *f_pos >= sizeof( __protocol_sequence ) )
        return 0;
    
    count = sizeof( __protocol_sequence ) - (*f_pos);

    if ( copy_to_user( data, ( const void * )(&__protocol_sequence ), count ) ) {
        return -EFAULT;
    }

    *f_pos += count;
    // todo: up(&dev-dem);
    return count;
}

static ssize_t dgmod_cdev_write(struct file *file, const char __user *data, size_t size, loff_t *f_pos)
{
    static struct dgmod_protocol_sequence protocol_sequence;
    size_t count = sizeof( protocol_sequence ) - ( *f_pos );
    
    printk(KERN_INFO "dgmod_cdev_write size=%d\n", size );
    
    if ( *f_pos >= sizeof( protocol_sequence ) )
        return 0;

    if ( copy_from_user( (char *)(&protocol_sequence) + *f_pos, data, count ) )
        return -EFAULT;

    *f_pos += count;

    if ( *f_pos >= sizeof( protocol_sequence ) )
        __protocol_sequence = protocol_sequence;
    
    return count;
}

static struct file_operations dgmod_cdev_fops = {
    .owner   = THIS_MODULE,
    .open    = dgmod_cdev_open,
    .release = dgmod_cdev_release,
    .read    = dgmod_cdev_read,
    .write   = dgmod_cdev_write,
};

static dev_t dgmod_dev_t = 0;
static struct cdev * __dgmod_cdev;
static struct class * __dgmod_class;

static int dgmod_dtor( int errno )
{
    if ( __dgmod_class ) {
        class_destroy( __dgmod_class );
        printk( KERN_INFO "" MODNAME " class_destry done\n" );
    }

    if ( __dgmod_cdev ) {
        cdev_del( __dgmod_cdev );
        printk( KERN_INFO "" MODNAME " cdev_del done\n" );
    }

    if ( dgmod_dev_t ) {
        unregister_chrdev_region( dgmod_dev_t, 1 );
        printk( KERN_INFO "" MODNAME " unregister_chrdev_region done\n" );
    }

    return errno;
}

static int __init
dgmod_module_init( void )
{
    int ret;

    printk( KERN_INFO "" MODNAME " driver v%s loaded\n", MOD_VERSION );

    // DEVICE
    if ( alloc_chrdev_region(&dgmod_dev_t, 0, 1, "dgmod-cdev" ) < 0 ) {
        printk( KERN_ERR "" MODNAME " failed to alloc chrdev region\n" );
        return -1;
    }
    
    __dgmod_cdev = cdev_alloc();
    if ( !__dgmod_cdev ) {
        printk( KERN_ERR "" MODNAME " failed to alloc cdev\n" );
        return -ENOMEM;
    }

    cdev_init( __dgmod_cdev, &dgmod_cdev_fops );
    if ( cdev_add( __dgmod_cdev, dgmod_dev_t, 1 ) < 0 ) {
        printk( KERN_ERR "" MODNAME " failed to add cdev\n" );
        return dgmod_dtor( -1 );
    }

    __dgmod_class = class_create( THIS_MODULE, "dgmod" );
    if ( !__dgmod_class ) {
        printk( KERN_ERR "" MODNAME " failed to create class\n" );
        return dgmod_dtor( -1 );
    }

    // make_nod /dev/dgmod0
    if ( !device_create( __dgmod_class, NULL, dgmod_dev_t, NULL, "dgmod%d", MINOR( dgmod_dev_t ) ) ) {
        printk( KERN_ERR "" MODNAME " failed to create device\n" );
        return dgmod_dtor( -1 );
    }

    // /proc create
    proc_create( "dgmod", 0, NULL, &proc_file_fops );
    
    // IRQ
    if ( ( ret = request_irq( IRQ_NUM, handle_interrupt, 0, "dgmod", NULL) ) < 0 )
        return dgmod_dtor( ret );

    // IOMAP
    __resource = request_mem_region( map_base_addr, map_size, "dgmod" );
    if ( __resource ) {

        printk(KERN_INFO "" MODNAME " requested memory resource: %x, %x, %s\n"
               , __resource->start, __resource->end, __resource->name );
        
        __mapped_ptr = (uint32_t *)( ioremap( __resource->start, map_size ) );
        printk(KERN_INFO "" MODNAME " __mapped_ptr: %x\n", (unsigned int)(__mapped_ptr) );
        
    }

    printk(KERN_ALERT "dgmod registed\n");
  
    return 0;
}

static void
dgmod_module_exit( void )
{
    free_irq( IRQ_NUM, NULL);

    if ( __mapped_ptr )
        iounmap( __mapped_ptr );

    if ( __resource )
        release_mem_region( map_base_addr, map_size );

    printk( KERN_ALERT "exit from dgmod\n" );

    remove_proc_entry( "dgmod", NULL );
    
    device_destroy( __dgmod_class, dgmod_dev_t );
    dgmod_dtor( 0 );
    
    printk( KERN_INFO "" MODNAME " driver v%s unloaded\n", MOD_VERSION );
}

module_init( dgmod_module_init );
module_exit( dgmod_module_exit );
