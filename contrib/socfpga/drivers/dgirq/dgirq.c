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

static char *devname = "dgirq";

#define DGIRQ_VERSION  "1.0"
#define MODNAME        "dgirq"

#define DGIRQ_MAJOR    MISC_MAJOR
#define DGIRQ_MINOR    152

#define IRQ_NUM        72

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("dgirq");
module_param( devname, charp, S_IRUGO );
MODULE_PARM_DESC(devname, "dgirq param");

#define countof(x) (sizeof(x)/sizeof((x)[0]))

static irqreturn_t
handle_interrupt(int irq, void *dev_id)
{
    if ( irq != IRQ_NUM )
        return IRQ_NONE;

    printk(KERN_ALERT "Interrupt %d occured\n",IRQ_NUM);  

    return IRQ_HANDLED;
}

static int
dgirq_proc_read( struct seq_file * m, void * v )
{
    seq_printf( m, "Hello this is DGIRQ.  I have no mapped memory in my resouce.\n" );
    return 0;
}

static int
dgirq_proc_open( struct inode * inode, struct file * file )
{
    return single_open( file, dgirq_proc_read, NULL );
}


static const struct file_operations proc_file_fops = {
     .owner = THIS_MODULE,
     .open = dgirq_proc_open,
     .read = seq_read,
     .llseek = seq_lseek,
     .release = single_release,
};

static int __init
dgirq_module_init( void )
{
    int ret;
    proc_create( "dgirq", 0, NULL, &proc_file_fops );
    
    printk(KERN_INFO "" MODNAME " driver v%s loaded\n", DGIRQ_VERSION);
    
    ret = request_irq( IRQ_NUM, handle_interrupt, 0, "dgirq", NULL);
    if (ret < 0)
        return ret;

    printk(KERN_ALERT "dgirq registed\n");
  
    return 0;
}

static void
dgirq_module_exit( void )
{
    free_irq( IRQ_NUM, NULL);

    printk(KERN_ALERT "exit from dgirq\n");    
    
    remove_proc_entry( "dgirq", NULL );
    
    printk(KERN_INFO "" MODNAME " driver v%s unloaded\n", DGIRQ_VERSION);
}

module_init( dgirq_module_init );
module_exit( dgirq_module_exit );
