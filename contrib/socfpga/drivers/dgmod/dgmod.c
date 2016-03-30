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
#include "dgfsm.h"
#include "dgmod_delay_pulse.h"
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/ioctl.h>
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

static struct pulse_addr pulse_register [] = {
    {   (0x10200u / sizeof( uint32_t )), (0x10220u / sizeof( uint32_t )) }
    , { (0x10240u / sizeof( uint32_t )), (0x10260u / sizeof( uint32_t )) }
    , { (0x10280u / sizeof( uint32_t )), (0x102a0u / sizeof( uint32_t )) }
    , { (0x102c0u / sizeof( uint32_t )), (0x102e0u / sizeof( uint32_t )) }
    , { (0x103c0u / sizeof( uint32_t )), (0x103e0u / sizeof( uint32_t )) }
    , { (0x10400u / sizeof( uint32_t )), (0x10420u / sizeof( uint32_t )) }
};

static struct dgmod_protocol_sequence __protocol_sequence = {
    .interval_ = 100000, // 1ms @ 10ns resolution
    .size_ = 1,
    .protocols_[ 0 ].replicates_ = (-1),
    .protocols_[ 0 ].delay_pulses_ = { { 0, 1 }, { 0, 1 }, { 0, 1 }, { 0, 1 }, { 0, 1 }, { 0, 1 } },
};

static struct dgfsm __dgfsm = {
    .state = fsm_stop,
    .next_protocol = 0,
    .replicates_remain = (-1)
};

static int dgfsm_handle_irq( const struct dgmod_protocol_sequence * sequence );
static int dgfsm_init( const struct dgmod_protocol_sequence * sequence );

static irqreturn_t
handle_interrupt(int irq, void *dev_id)
{
    if ( irq != IRQ_NUM )
        return IRQ_NONE;
    
    // printk(KERN_ALERT "Interrupt %d occured\n",IRQ_NUM);  
    dgfsm_handle_irq( &__protocol_sequence ); // start trigger
    
    return IRQ_HANDLED;
}

static void dgfsm_start( void )
{
    dgfsm_init( &__protocol_sequence );
    dgfsm_handle_irq( &__protocol_sequence ); // commit trigger
    
    __mapped_ptr[ addr_machine_state ] = fsm_start;
    
    __dgfsm.state = 1;
}

static void dgfsm_stop( void )
{
    __dgfsm.state = 0;    
    __mapped_ptr[ addr_machine_state ] = fsm_stop;
}

static void dgfsm_commit( const struct dgmod_protocol * proto )
{
    if ( __mapped_ptr ) {

        for ( size_t ch = 0; ch < number_of_channels; ++ch ) {
            __mapped_ptr[ pulse_register[ ch ].delay ] = proto->delay_pulses_[ ch ].delay_;
            __mapped_ptr[ pulse_register[ ch ].width ] = proto->delay_pulses_[ ch ].width_;
        }

        __mapped_ptr[ addr_submit ] = 0;
        __mapped_ptr[ addr_submit ] = 0; // nop
        __mapped_ptr[ addr_submit ] = 1;
        
    }
}

static int dgfsm_init( const struct dgmod_protocol_sequence * sequence )
{
    if ( sequence ) {

        __dgfsm.next_protocol = 0;
        __dgfsm.number_of_protocols = sequence->size_ & 0x03; // make sure < 4
        __dgfsm.replicates_remain = sequence->protocols_[ 0 ].replicates_;

    }
    return 0;
}

static int dgfsm_handle_irq( const struct dgmod_protocol_sequence * sequence )
{
    if ( __dgfsm.replicates_remain && ( --__dgfsm.replicates_remain == 0 ) ) {

        if ( ++__dgfsm.next_protocol > __dgfsm.number_of_protocols )
            __dgfsm.next_protocol = 0;

        do {
            const struct dgmod_protocol * proto = &sequence->protocols_[ __dgfsm.next_protocol ];
            __dgfsm.replicates_remain = proto->replicates_;

            dgfsm_commit( proto );
            
        } while ( 0 );
            
    }
    return 0;
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
        seq_printf( m, "Has %d protocol sequence, To = %d\n"
                    , __protocol_sequence.size_, __protocol_sequence.interval_ );
        
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

static ssize_t
dgmod_proc_write( struct file * filep, const char * user, size_t size, loff_t * f_off ) //unsigned long size, void * data )
{
    static char readbuf[256];

    if ( size >= sizeof( readbuf ) )
        size = sizeof( readbuf ) - 1;

    if ( copy_from_user( readbuf, user, size ) )
        return -EFAULT;

    readbuf[ size ] = '\0';

    if ( size >= 5 && strcasecmp( readbuf, "start" ) == 0 ) {
        dgfsm_start();
        printk( KERN_INFO "" MODNAME " fsm started.\n" );
    } else if ( size >= 4 && strcasecmp( readbuf, "stop" ) == 0 ) {
        dgfsm_stop();
        printk( KERN_INFO "" MODNAME " fsm stopped.\n" );
    } else {
        printk( KERN_INFO "" MODNAME " proc write received unknown command: %s.\n", readbuf );
    }

    return size;
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
    .write   = dgmod_proc_write,
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

#if 0
static long dgmod_cdev_ioctl( struct file* filp, unsigned int code, unsigned long args)
{
    printk( KERN_INFO "dgmod_cdev_ioctl, code=%x, args=%lx\n", code, args );
    return 0;
}
#endif

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

    // following code will cause an infinite loop for 'cat' command
    // if ( *f_pos >= sizeof( __protocol_sequence ) )
    //     *f_pos = 0; // recycle
        
    // todo: up(&dev-dem);
    return count;
}

static ssize_t dgmod_cdev_write(struct file *file, const char __user *data, size_t size, loff_t *f_pos)
{
    static struct dgmod_protocol_sequence protocol_sequence;
    size_t count = sizeof( protocol_sequence ) - ( *f_pos );
    
    printk(KERN_INFO "dgmod_cdev_write size=%d\n", size );
    
    if ( *f_pos >= sizeof( protocol_sequence ) ) {
        printk(KERN_INFO "dgmod_cdev_write size overrun size=%d, offset=%lld\n", size, *f_pos );
        return 0;
    }

    if ( copy_from_user( (char *)(&protocol_sequence) + *f_pos, data, count ) )
        return -EFAULT;

    *f_pos += count;

    if ( *f_pos >= sizeof( protocol_sequence ) ) {

        __protocol_sequence = protocol_sequence;

        dgfsm_init( &__protocol_sequence ); // it will be loaded at next irq

        if ( __dgfsm.state == 0 ) { // fsm 'stopped'
            dgfsm_handle_irq( &__protocol_sequence ); // force commit
            dgfsm_start();
        }
            
    }
    
    return count;
}

static struct file_operations dgmod_cdev_fops = {
    .owner   = THIS_MODULE,
    .open    = dgmod_cdev_open,
    .release = dgmod_cdev_release,
    // .unlocked_ioctl = dgmod_cdev_ioctl,
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
    proc_create( "dgmod", 0644, NULL, &proc_file_fops );
    
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

        dgfsm_init( &__protocol_sequence );
    }

    printk(KERN_ALERT "dgmod registed\n");
  
    return 0;
}

static void
dgmod_module_exit( void )
{
    dgfsm_stop();
    
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
