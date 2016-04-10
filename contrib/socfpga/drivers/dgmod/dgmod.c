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
#include "hps.h"
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/ioctl.h>
#include <linux/irq.h>
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

static struct resource * __resource;
static uint32_t * __mapped_ptr;

struct irq_allocated {
    size_t size;
    int irqNumber[ 128 ];
};

struct gpio_allocated {
    size_t size;
    int gpioNumber[ 128 ];
};

static struct irq_allocated __irq = {
    .size = 0
    , .irqNumber = { 0 }
};

static struct gpio_allocated __gpio = {
    .size = 0
    , .gpioNumber = { 0 }
};

static int __debug_level__ = 5;

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
    if ( irq == __irq.irqNumber[ 0 ] ) {

        int key = gpio_get_value( gpio_user_key );
        gpio_set_value( gpio_user_led, key );

        if ( key == 0 ) // key pressed
            dgfsm_handle_irq( &__protocol_sequence ); // start trigger

        printk( KERN_INFO "IRQ %d handled; key=%d\n", irq, key );        
        
        return IRQ_HANDLED;
    }

    if ( __debug_level__ > 0 )
        printk( KERN_ALERT "Interrupt %d occured\n", irq );

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

        if ( __debug_level__ > 3 )
            printk( KERN_INFO "dgfsm_init: size= %d", sequence->size_ );

        __dgfsm.next_protocol = 0;
        __dgfsm.number_of_protocols =
            sequence->size_ <= countof( sequence->protocols_ ) ?
            sequence->size_ : countof( sequence->protocols_ );
        
        __dgfsm.replicates_remain = sequence->protocols_[ 0 ].replicates_;

    }
    return 0;
}

static int dgfsm_handle_irq( const struct dgmod_protocol_sequence * sequence )
{
    if ( __debug_level__ > 3 ) {
        printk( KERN_INFO "dgfsm_handle_irq: trig left:%u proto# %u/%u\n"
                , __dgfsm.replicates_remain
                , __dgfsm.next_protocol
                , __dgfsm.number_of_protocols );
    }
    
    if ( __dgfsm.replicates_remain && ( --__dgfsm.replicates_remain == 0 ) ) {

        if ( ++__dgfsm.next_protocol > __dgfsm.number_of_protocols )
            __dgfsm.next_protocol = 0;

        do {

            const struct dgmod_protocol * proto = &sequence->protocols_[ __dgfsm.next_protocol ];
            __dgfsm.replicates_remain = proto->replicates_;

            dgfsm_commit( proto );

            if ( __debug_level__ > 3 )
                printk( KERN_INFO "dgfsm_commit: #%d\n", __dgfsm.next_protocol );
            
        } while ( 0 );
            
    }

    return 0;
}

static int
dgmod_proc_read( struct seq_file * m, void * v )
{
    seq_printf( m, "Hello, Welcome to DGMOD --debugging--.\n" );

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

    seq_printf( m, " dipsw[%x]=\t0x%x\tdir:0x%x\tmask:0x%x\tedge:0x%x\n"
                , (pio_dipsw_base) * 4
                , __mapped_ptr[ pio_dipsw_base ] // value
                , __mapped_ptr[ pio_dipsw_base + 1 ] 
                , __mapped_ptr[ pio_dipsw_base + 2 ] // mask
                , __mapped_ptr[ pio_dipsw_base + 3 ] );

    seq_printf( m, "button[%x]=\t0x%x\tdir:0x%x\tmask:0x%x\tedge:0x%x\n"
                , (pio_button_base) * 4
                , __mapped_ptr[ pio_button_base ] // value
                , __mapped_ptr[ pio_button_base + 1 ] 
                , __mapped_ptr[ pio_button_base + 2 ] // mask
                , __mapped_ptr[ pio_button_base + 3 ] );    
    
    return 0;
}

static ssize_t
dgmod_proc_write( struct file * filep, const char * user, size_t size, loff_t * f_off )
{
    static char readbuf[256];

    if ( size >= sizeof( readbuf ) )
        size = sizeof( readbuf ) - 1;

    if ( copy_from_user( readbuf, user, size ) )
        return -EFAULT;

    readbuf[ size ] = '\0';

    if ( size >= 5 && strcasecmp( readbuf, "start" ) == 0 ) {
        dgfsm_start();
        if ( __debug_level__ > 1 )
            printk( KERN_INFO "" MODNAME " fsm started.\n" );
    } else if ( size >= 4 && strcasecmp( readbuf, "stop" ) == 0 ) {
        dgfsm_stop();
        if ( __debug_level__ > 1 )
            printk( KERN_INFO "" MODNAME " fsm stopped.\n" );
    } else if ( strncmp( readbuf, "on", 2 ) == 0 ) {
        __mapped_ptr[ pio_led_base ] = 1;
    } else if ( strncmp( readbuf, "off", 3 ) == 0 ) {
        __mapped_ptr[ pio_led_base ] = 0;
    } else {
        if ( __debug_level__ > 0 )
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
    if ( __debug_level__ )
        printk(KERN_INFO "open dgmod char device\n");
    return 0;
}

static int dgmod_cdev_release(struct inode *inode, struct file *file)
{
    if ( __debug_level__ )    
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

    if ( __debug_level__ > 1 )        
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

    if ( __debug_level__ > 1 )
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

static int dgmod_gpio_output_init( int gpio, const char * name )
{
    if ( gpio_is_valid( gpio ) ) {

        __gpio.gpioNumber[ __gpio.size++ ] = gpio;
        
        gpio_request( gpio, name );
        gpio_direction_output( gpio, 1 );
        gpio_export( gpio, true );

        return 1;
    }
    return 0;
}

static int dgmod_gpio_input_init( int gpio, const char * name, int debounce )
{
    int irqNumber = 0;
    
    if ( gpio_is_valid( gpio ) ) {

        __gpio.gpioNumber[ __gpio.size++ ] = gpio;
        
        gpio_request( gpio, name );
        gpio_direction_input( gpio );
        
        if ( debounce )
            gpio_set_debounce( gpio, 50 );  // debounce the button with a delay of 50ms
        
        gpio_export( gpio, false );     // export /sys/class, false prevents direction change

        irqNumber = gpio_to_irq( gpio );
        
        printk( KERN_INFO "" MODNAME " GPIO [%d] (%s) mapped to IRQ: %d\n", gpio, name, irqNumber );
        
        // irq
        if ( request_irq( irqNumber, handle_interrupt, 0, "dgmod", NULL) < 0 ) {
            printk( KERN_INFO "" MODNAME " GPIO IRQ: %d request failed\n", irqNumber );
            return 0;
        }
        
        // irq_set_irq_type( __irqNumber, IRQ_TYPE_EDGE_RISING );   // see linux/irq.h
        irq_set_irq_type( irqNumber, IRQ_TYPE_EDGE_BOTH );   // see linux/irq.h
        
        return irqNumber;
        
    } else {
        printk(KERN_INFO "" MODNAME " gpio %d not valid\n", gpio );
    }
    return 0;
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
    proc_create( "dgmod", 0666, NULL, &proc_file_fops );
    
    // IRQ
    if ( ( ret = request_irq( 74, handle_interrupt, 0, "dgmod", NULL) ) < 0 ) {
        return dgmod_dtor( ret );
    }
    __irq.irqNumber[ __irq.size++ ] = 74;

    // IOMAP
    __resource = request_mem_region( map_base_addr, map_size, "dgmod" );
    if ( __resource ) {

        printk(KERN_INFO "" MODNAME " requested memory resource: %x, %x, %s\n"
               , __resource->start, __resource->end, __resource->name );
        
        __mapped_ptr = (uint32_t *)( ioremap( __resource->start, map_size ) );

        if ( __mapped_ptr ) {

            // BUTTON_PIO_
            __mapped_ptr[ pio_button_base + 2 ] = 0x0f;  // PIO IRQ MASK CLEAR
            __mapped_ptr[ pio_dipsw_base + 2 ] = 0x0f;  // PIO IRQ MASK CLEAR

            /* for ( int i = 0; i < 4; ++i ) */
            /*     printk(KERN_INFO "" MODNAME " button[%d]=0x%08x\n", i, __mapped_ptr[ pio_button_base + i ] ); */

            // LED on
            __mapped_ptr[ pio_led_base ] = 0x02;
        }

        printk(KERN_INFO "" MODNAME " __mapped_ptr: %x\n", (unsigned int)(__mapped_ptr) );

        dgfsm_init( &__protocol_sequence );
    }

    // GPIO
    do {
        if ( ( __irq.irqNumber[ __irq.size ] = dgmod_gpio_input_init( gpio_user_key, "dgmod-key", 50 ) ) ) {
            ++__irq.size;
        }

        if ( ( __irq.irqNumber[ __irq.size ] = dgmod_gpio_input_init( gpio_button_pio, "dgmod-button", 0 ) ) ) {
            ++__irq.size;
        }

        if ( ( __irq.irqNumber[ __irq.size ] = dgmod_gpio_input_init( gpio_button_pio + 1, "dgmod-button", 0 ) ) ) {
            ++__irq.size;
        }

        if ( ( __irq.irqNumber[ __irq.size ] = dgmod_gpio_input_init( gpio_dipsw_pio, "dgmod-dipsw", 0 ) ) ) {
            ++__irq.size;
        }

        if ( ( __irq.irqNumber[ __irq.size ] = dgmod_gpio_input_init( gpio_dipsw_pio + 1, "dgmod-dipsw", 0 ) ) ) {
            ++__irq.size;
        }

        dgmod_gpio_output_init( gpio_user_led, "dgmod-led0" );

    } while ( 0 );
    
    //

    printk(KERN_ALERT "dgmod registed\n");
  
    return 0;
}

static void
dgmod_module_exit( void )
{
    dgfsm_stop();
    gpio_set_value( gpio_user_led, 0 );

    for ( int i = 0; i < __irq.size; ++i ) {
        free_irq( __irq.irqNumber[ i ], NULL );
        printk( KERN_INFO "free_irq( %d )\n", __irq.irqNumber[ i ] );        
    }

    for ( int i = 0; i < __gpio.size; ++i ) {
        gpio_free( __gpio.gpioNumber[ i ] );
        printk( KERN_INFO "gpio_free( %d )\n", __gpio.gpioNumber[ i ] );                
    }
    
    if ( __mapped_ptr && __resource ) {

        __mapped_ptr[ pio_led_base ] = 0;

        iounmap( __mapped_ptr );
        release_mem_region( map_base_addr, map_size );        
    }

    remove_proc_entry( "dgmod", NULL ); // proc_create

    device_destroy( __dgmod_class, dgmod_dev_t ); // device_creeate

    class_destroy( __dgmod_class ); // class_create

    cdev_del( __dgmod_cdev ); // cdev_alloc, cdev_init

    unregister_chrdev_region( dgmod_dev_t, 1 ); // alloc_chrdev_region
    //

    printk( KERN_INFO "" MODNAME " driver v%s unloaded\n", MOD_VERSION );
}

module_init( dgmod_module_init );
module_exit( dgmod_module_exit );
