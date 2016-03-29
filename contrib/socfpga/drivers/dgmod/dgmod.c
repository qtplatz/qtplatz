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

#include <linux/module.h>
#include <linux/proc_fs.h> // create_proc_entry
#include <linux/seq_file.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include "dgmod.h"

static char *devname = MODNAME;

MODULE_AUTHOR( "Toshinobu Hondo, Matsuoka" );
MODULE_DESCRIPTION( "Device Driver for MULTUM-II Control Delay Pulse Generator" );
MODULE_LICENSE("GPL");
MODULE_PARM_DESC(devname, "dgmod param");
module_param( devname, charp, S_IRUGO );

#define countof(x) (sizeof(x)/sizeof((x)[0]))

static struct resource * __resource;

enum {
    map_base_addr = 0xff200000
    , map_size = 0x20000
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
    seq_printf( m, "Hello this is DGMOD.  I have no mapped memory in my resouce.\n" );
    return 0;
}

static int
dgmod_proc_open( struct inode * inode, struct file * file )
{
    return single_open( file, dgmod_proc_read, NULL );
}


static const struct file_operations proc_file_fops = {
     .owner = THIS_MODULE,
     .open = dgmod_proc_open,
     .read = seq_read,
     .llseek = seq_lseek,
     .release = single_release,
};

static int __init
dgmod_module_init( void )
{
    int ret;
    proc_create( "dgmod", 0, NULL, &proc_file_fops );
    
    printk( KERN_INFO "" MODNAME " driver v%s loaded\n", MOD_VERSION );
    
    ret = request_irq( IRQ_NUM, handle_interrupt, 0, "dgmod", NULL);
    if (ret < 0)
        return ret;

    __resource = request_mem_region( map_base_addr, map_size, "dgmod" );
    if ( __resource )
        printk(KERN_INFO "" MODNAME " requested memory resource: %x, %x, %s\n", __resource->start, __resource->end, __resource->name );

    printk(KERN_ALERT "dgmod registed\n");
  
    return 0;
}

static void
dgmod_module_exit( void )
{
    free_irq( IRQ_NUM, NULL);

    printk(KERN_ALERT "exit from dgmod\n");    
    
    remove_proc_entry( "dgmod", NULL );
    
    printk( KERN_INFO "" MODNAME " driver v%s unloaded\n", MOD_VERSION );
}

module_init( dgmod_module_init );
module_exit( dgmod_module_exit );
