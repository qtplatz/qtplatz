//////////////////////////////////////////////////////////////////////////////////////////
//
//  LinuxDriverPCI.h	Declarations for Linux Kernel-Space Device Driver for PCI9080-based Acqiris Devices
//
//----------------------------------------------------------------------------------------
//  Copyright Agilent Technologies, Inc. 2000, 2001-2009
//
//  $Id: acqiris.h 37124 2009-11-12 10:02:50Z bdonnier $
//
//  Started:	 6 JUN 2000
//  Owned by:	V. Hungerbuhler
//
//////////////////////////////////////////////////////////////////////////////////////////
#ifndef LINUXDRIVERPCI_H
#define LINUXDRIVERPCI_H

#include <linux/version.h>
#include <linux/pci.h>
#include <linux/cdev.h>

#include "DDrIORules.h"


#define MOD_NAME    "acqiris"
#define MOD_VERSION "2.0.0"

#define MOD_VERSION_MAJOR  2
#define MOD_VERSION_MINOR  0
#define MOD_VERSION_CODE   ((0x100 * (MOD_VERSION_MAJOR)) + (MOD_VERSION_MINOR))

#define CDEV_NAME          MOD_NAME
#define CDEV_MINOR_FIRST   0
#define CDEV_MINOR_COUNT   256

#define MAX_DEVICES        (CDEV_MINOR_COUNT - 1)

#define DMA_DESC_SIZE_01 0x1000
#define DMA_DESC_SIZE_2  0x1800

typedef unsigned char	uchar;


//////////////////////////////////////////////////////////////////////////////////////////
// Definitions of PCI vendor constants

#define PCI_VENDOR_ID_ACQIRIS   0x14ec
#define PCI_DEVICE_ID_ACQIRIS_0 0x0000
#define PCI_DEVICE_ID_ACQIRIS_1 0x0001
#define PCI_DEVICE_ID_ACQIRIS_2 0x0002
#define PCI_VENDOR_ID_ADLINK    0x144a
#define PCI_DEVICE_ID_ADLINK_0  0x0000
#define PCI_DEVICE_ID_ADLINK_1  0x0001
#define PCI_VENDOR_ID_PLXTECH   0x10b5
#define PCI_DEVICE_ID_PLXTECH_0 0x9054
#define PCI_DEVICE_ID_PLXTECH_1 0x9080


//////////////////////////////////////////////////////////////////////////////////////////
// Definitions of prefixes for printk

#define ACQRS_EMERG   KERN_EMERG   MOD_NAME ": "  /* system is unusable                   */
#define ACQRS_ALERT   KERN_ALERT   MOD_NAME ": "  /* action must be taken immediately     */
#define ACQRS_CRIT    KERN_CRIT    MOD_NAME ": "  /* critical conditions                  */
#define ACQRS_ERR     KERN_ERR     MOD_NAME ": "  /* error conditions                     */
#define ACQRS_WARNING KERN_WARNING MOD_NAME ": "  /* warning conditions                   */
#define ACQRS_NOTICE  KERN_NOTICE  MOD_NAME ": "  /* normal but significant condition     */
#define ACQRS_INFO    KERN_INFO    MOD_NAME ": "  /* informational                        */
#define ACQRS_DEBUG   KERN_DEBUG   MOD_NAME ": "  /* debug-level messages                 */


#define DEVICE_INTERRUPTCLEAR_OFFSET	0xc			// Interrupt Clear register (offset wrt ctrl/status!)
#define DISABLE_AUTOCLEAR				0x80000000	// Disables interrupt clear on reading

//////////////////////////////////////////////////////////////////////////////////////////
//  Definition of debug output levels

extern int dbgl;

#define DDMA	0x01	// DMA debug
#define DINT	0x02	// interrupt function
#define DIRW	0x04	// read/write access
#define DAEW	0x08	// end of acquisition
#define DINIT	0x10	// initialization
#define DMEM	0x20	// memory allocations
#define DOPS	0x40	// ioctl operations


//////////////////////////////////////////////////////////////////////////////////////////
// acqiris device and driver structures

struct acqiris_device
{
    int dev_num;
    dev_t dev;

    struct pci_dev *pci_dev;

    void *dma_desc;
    u32 dma_desc_size;
    struct page **pages;

    DDrResources res;
    u32 devfn;
    u16 vendor_id;
    u16 device_id;

    wait_queue_head_t	wait_dma;
    wait_queue_head_t	wait_acq;
    wait_queue_head_t	wait_proc;

    u32 mask_int;

};


struct acqiris_driver
{
    int nbr_devices;
    int max_devices;
    struct acqiris_device **devices;

    dev_t dev;
    struct cdev cdev;

};

extern struct acqiris_driver aq_drv;


struct acqiris_ioop
{
	u32 __user * in_buf;   // input buffer: for data transfer TO kernel module
	size_t in_size;        // number of bytes in input buffer
	u32 __user * out_buf;  // output buffer: for data transfer FROM kernel module
	size_t out_size;       // number of bytes available in output buffer
	size_t ret_size;       // number of bytes actually filled in output buffer
    s32 err_code;          // error code to be returned FROM kernel module

};


//////////////////////////////////////////////////////////////////////////////////////////
// Define constants which are specific to the PLX9080/9054 PCI interface Chip

#define	MARBR_OFFSET		0x08			// Addr offset of Mode/Arbitration register
#define	BIGEND_OFFSET		0x0c			// Addr offset of big/little endian register
#define EROMBA				0x14			// Addr offset of BREQo control
#define DMPBAM				0x28			// Addr offset of direct master to PCI memory
#define MAILBOX7_OFFSET		0x5c			// Addr offset of Mailbox 7
#define	INTSRC_OFFSET		0x68			// Addr offset of Interrupt/Status register
#define CNTRL_OFFSET		0x6c			// Addr offset of CNTRL register
#define	DMA0MODE_OFFSET		0x80			// Addr offset of DMA0 mode register
#define	DMA0DESC_OFFSET		0x90			// Addr offset of DMA0 descriptor pointer
#define	DMA0CSR_OFFSET		0xa8			// Addr offset of DMA0 command/status register

#define	DMA_THR			0xb0			// Addr offset of DMA threshold register

#define	MARBR_PCI_REQ_MODE	0x00800000		// MARBR:  Enable REQ deassertion in master cycles

#define	INTSRC_DMA_INT_EN	0x00040000		// INTSRC: Enable bit for DMA0 interrupt
#define	INTSRC_LOCAL_INT_EN	0x00000800		// INTSRC: Enable bit for module interrupt
#define	INTSRC_PCI_INT_EN	0x00000100		// INTSRC: Enable bit for interrupt on PCI-bus
#define	INTSRC_DMA_ACTIVE	0x00200000		// INTSRC: Activity bit for DMA0 interrupt
#define	INTSRC_LOCAL_ACTIVE	0x00008000		// INTSRC: Activity bit for module interrupt

#define EEPROM_PROGENABLE	0x00010000		// CNTRL: Control bit for EEPROM program-enable
#define EEPROM_CLOCK		0x01000000		// CNTRL: Control bit for EEPROM clock
#define EEPROM_CHIPSEL		0x02000000		// CNTRL: Control bit for EEPROM chip-select
#define EEPROM_WRITE		0x04000000		// CNTRL: Control bit for EEPROM write
#define EEPROM_READ			0x08000000		// CNTRL: Control bit for EEPROM read
#define EEPROM_CLEARBITS	0x0f010000		// CNTRL: Combination of the 5 bits above
#define CNTRL_RESET			0x40000000		// CNTRL: PCI Adapter Software Reset

#define	DMA0MODE_READY_EN	0x00000040		// DMA0MODE: Ready Input Enable
#define	DMA0MODE_BTERM		0x00000080		// DMA0MODE: BTERM# input enable (needed!)
#define DMA0MODE_LOCALBRST	0x00000100		// DMA0MODE: Local Bursting enable
#define DMA0MODE_CHAINING	0x00000200		// DMA0MODE: Chaining enable
#define DMA0MODE_INTENABLE	0x00000400		// DMA0MODE: Interrupt enable (at end of Xfer)
#define DMA0MODE_CONSTANT	0x00000800		// DMA0MODE: 1 = keeps local address constant
#define DMA0MODE_PCIINTRPT	0x00020000		// DMA0MODE: 1 = sends interrupt to PCI bus

#define DMA0MODE (DMA0MODE_READY_EN		|	DMA0MODE_BTERM		|	\
				  DMA0MODE_CHAINING		|	DMA0MODE_LOCALBRST	|	\
				  DMA0MODE_INTENABLE	|	DMA0MODE_CONSTANT	|	\
				  DMA0MODE_PCIINTRPT	)

#define	DMA0CSR_ENABLE		0x00000001		// DMA0CSR: Enable transfer
#define	DMA0CSR_START		0x00000002		// DMA0CSR: Start transfer
#define	DMA0CSR_ABORT		0x00000004		// DMA0CSR: Abort transfer
#define	DMA0CSR_CLRINTRPT	0x00000008		// DMA0CSR: Clear DMA0 interrupts
#define	DMA0CSR_CHAN_DONE	0x00000010		// DMA0CSR: 'Channel Done' indicator

#define PCI_ID_OFFSET		0x00			// PCI-Config Space: deviceID + vendorID
#define PCI_CR_OFFSET		0x04			// PCI-Config Space: control register
#define PCI_HDR_OFFSET		0x0c			// PCI-Config Space: header type etc.
#define PCI_BAR0_OFFSET		0x10			// PCI-Config Space: base address register 0
#define PCI_BAR1_OFFSET		0x14			// PCI-Config Space: base address register 1
#define PCI_BAR2_OFFSET		0x18			// PCI-Config Space: base address register 2
#define PCI_INTRPT_OFFSET	0x3c			// PCI_Config Space: interrupts etc.



//////////////////////////////////////////////////////////////////////////////////////////
//  Definition of Global Function Prototypes

#if HAVE_UNLOCKED_IOCTL
long acqiris_ioctl_unlocked(struct file* fileP, unsigned int code, unsigned long args);
#else
int acqiris_ioctl(struct inode *inodeP, struct file* fileP, unsigned int code, unsigned long args);
#endif

#if HAVE_COMPAT_IOCTL
#ifdef CONFIG_COMPAT
long acqiris_ioctl_compat(struct file * fileP, unsigned int code, unsigned long args);
#endif
#endif

int acqiris_open(struct inode* inodeP, struct file* fileP);
int acqiris_release(struct inode* inodeP, struct file* fileP);

int /* __devinit */ acqiris_device_probe(struct pci_dev *devP, const struct pci_device_id *idDevP);
void /* __devexit */ acqiris_device_remove(struct pci_dev *devP);

long acqiris_config_operation(struct file* fileP, u32 cmd, struct acqiris_ioop *ioop);
long acqiris_user_operation(struct file* fileP, u32 cmd, struct acqiris_ioop *ioop);

#if LINUX_VERSION_CODE < 0x20613 // irq_handler_t has changed in 2.6.19
irqreturn_t acqiris_interrupt_handler(int irq, void* private, struct pt_regs* regs);
#else
irqreturn_t acqiris_interrupt_handler(int irq, void* private);
#endif


//////////////////////////////////////////////////////////////////////////////////////////
//  Definition of VxD Macros

#define READ_REG32(addr, offset) \
            le32_to_cpu(*(((volatile u32 *)(addr)) + ((offset) / 4)))

#define WRITE_REG32(addr, offset, value) \
            *(((volatile u32 *)(addr)) + ((offset) / 4)) = cpu_to_le32(value)


#endif // sentry

