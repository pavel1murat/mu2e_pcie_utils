/*  This file (pci_devel_main.c) was created by Ron Rechenmacher <ron@fnal.gov> on
    Apr 23, 2014. "TERMS AND CONDITIONS" governing this file are in the README
    or COPYING file. If you do not have such a file, one can be obtained by
    contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
    $RCSfile: .emacs.gnu,v $
    rev="$Revision: 1.23 $$Date: 2012/01/23 15:32:40 $";
    */
#include <linux/module.h>	// module_param, THIS_MODULE
#include <linux/init.h>		// module_init,_exit
#include <linux/kernel.h>	// KERN_INFO, printk
#include <linux/fs.h>		/* struct inode */
#include <linux/device.h>       /* class_create */
#include <linux/cdev.h>         /* cdev_add */
#include <linux/types.h>        /* dev_t */
#include <linux/pci.h>          /* struct pci_dev *pci_get_device */
#include <linux/delay.h>	/* msleep */
#include <asm/uaccess.h>	/* access_ok, copy_to_user */

#include "trace.h"	/* TRACE */

#include "pcidevl_ioctl.h"	/* PCIDEVL_DEV_FILE */

/* GLOBALS */

struct pci_dev *pci_dev_sp=0;

typedef struct
{   unsigned long basePAddr;    /**< Base address of device memory */
    unsigned long baseLen;      /**< Length of device memory */
    void __iomem * baseVAddr;   /**< VA - mapped address */
} bar_info_t;

bar_info_t      pcie_bar_info;


//////////////////////////////////////////////////////////////////////////////



int devl_ioctl(  struct inode *inode, struct file *filp
	       , unsigned int cmd, unsigned long arg )
{
    unsigned long       base;

    if(_IOC_TYPE(cmd) != DEVL_IOC_MAGIC) return -ENOTTY;

    /* Check read/write and corresponding argument */
    if(_IOC_DIR(cmd) & _IOC_READ)
        if(!access_ok(VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd)))
            return -EFAULT;
    if(_IOC_DIR(cmd) & _IOC_WRITE)
        if(!access_ok(VERIFY_READ, (void *)arg, _IOC_SIZE(cmd)))
            return -EFAULT;

    /* DMA registers are offset from BAR0 */
    base = (unsigned long)(pcie_bar_info.baseVAddr);

    switch(cmd)
    {
    default:
	TRACE( 10, "devl_ioctl: unknown cmd" );
	return (-1); // some error
    }
    return (0);
}   // devl_ioctl


static struct file_operations devl_file_ops =
{   .owner=   THIS_MODULE,
    .llseek=  NULL,             /* lseek        */
    .read=    NULL,             /* read         */
    .write=   NULL,             /* write        */
    .readdir= NULL,             /* readdir      */
    .poll=    NULL,             /* poll         */
    .ioctl=   devl_ioctl,        /* ioctl        */
    .mmap=    NULL,             /* mmap         */
    NULL,                       /* open         */
    NULL,                       /* flush        */
    NULL,                       /* release (close?)*/
    NULL,                       /* fsync        */
    NULL,                       /* fasync       */
    NULL,                       /* check_media_change */
    NULL,                       /* revalidate   */
    NULL                        /* lock         */
};

dev_t         devl_dev_number;
struct class *devl_dev_class;
struct cdev   devl_cdev;

int devl_fs_up( void )
{
    int sts;
    sts = alloc_chrdev_region( &devl_dev_number, 0, 1, "devl_drv" );

    if(sts < 0)
    {   TRACE( 3, "dcm_init(): Failed to get device numbers" );
        return (sts);
    }
    
    devl_dev_class = class_create( THIS_MODULE, "devl_dev" );
    
    cdev_init( &devl_cdev, &devl_file_ops );

    devl_cdev.owner = THIS_MODULE;
    devl_cdev.ops   = &devl_file_ops;

    sts = cdev_add ( &devl_cdev, devl_dev_number, 1 );
    device_create( devl_dev_class, NULL, devl_dev_number, NULL, PCIDEVL_DEV_FILE );

    return (0);
}   // devl_fs_up


void devl_fs_down( void )
{
    device_destroy( devl_dev_class, devl_dev_number);
    cdev_del( &devl_cdev );
    class_destroy( devl_dev_class);
    unregister_chrdev_region( devl_dev_number, 1 );
} // devl_fs_down



void free_mem( void )
{
}   // free_mem


//////////////////////////////////////////////////////////////////////////////


static int __init init_devl(void)
{
    int             ret=0;          /* SUCCESS */

    TRACE( 2, "init_devl" );

    // fs interface, pci, memory, events(i.e polling)

    ret = devl_fs_up();
    if (ret) goto out;

    return (ret);

 out:
    TRACE( 0, "Error - freeing memory" );
    devl_fs_down();
    return (-1);
}   // init_devl


static void __exit exit_devl(void)
{
    TRACE( 1, "exit_devl() called");


    devl_fs_down();
}   // exit_devl


module_init(init_devl);
module_exit(exit_devl);

MODULE_AUTHOR("Ron Rechenmacher");
MODULE_DESCRIPTION("devl pcie driver");
MODULE_LICENSE("GPL"); /* Get rid of taint message by declaring code as GPL */
