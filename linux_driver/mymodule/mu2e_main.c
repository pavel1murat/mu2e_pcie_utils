/*  This file (mu2e.c) was created by Ron Rechenmacher <ron@fnal.gov> on
    Feb  5, 2014. "TERMS AND CONDITIONS" governing this file are in the README
    or COPYING file. If you do not have such a file, one can be obtained by
    contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
    $RCSfile: .emacs.gnu,v $
    rev="$Revision: 1.23 $$Date: 2012/01/23 15:32:40 $";
    */
#include <linux/module.h>	// module_param, THIS_MODULE
#include <linux/init.h>		// module_init,_exit
#include <linux/kernel.h>	// KERN_INFO, printk
#include <linux/fs.h>		/* struct inode */

#include "../trace/trace.h"	/* TRACE */
#include "mu2e_fs.h"
#include "mu2e_pci.h"
#include "mu2e_event.h"

u8 *mu2e_memory=0;

int mu2e_mmap( struct file *file, struct vm_area_struct *vma )
{
    return (0);
}   // mu2e_mmap

int mu2e_ioctl(  struct inode *inode, struct file *filp
	       , unsigned int cmd, unsigned long arg )
{
    return (0);
}   // mu2e_ioctl



//////////////////////////////////////////////////////////////////////////////

static int __init init_mu2e(void)
{
    int  ret=0;          /* SUCCESS */
    TRACE( 2, "init_mu2e trace" );

    // memory, fs interface, pci, events(i.e polling)
    mu2e_memory = kmalloc( 0x100000, GFP_KERNEL );
    ret = mu2e_fs_up();
    ret = mu2e_pci_up();
    ret = mu2e_event_up();
    return (ret);
}   // init_mu2e


static void __exit exit_mu2e(void)
{
    TRACE( 0, "exit_mu2e() called");

    // events, pci, fs interface, memory
    mu2e_event_down();
    mu2e_pci_down();
    mu2e_fs_down();
    kfree( mu2e_memory );

}   // exit_mu2e


module_init(init_mu2e);
module_exit(exit_mu2e);

MODULE_AUTHOR("Ron Rechenmacher");
MODULE_DESCRIPTION("mu2e pcie driver");
MODULE_LICENSE("GPL"); /* Get rid of taint message by declaring code as GPL */
