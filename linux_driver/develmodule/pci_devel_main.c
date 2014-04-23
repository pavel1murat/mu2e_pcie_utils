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
#include <linux/pci.h>          /* struct pci_dev *pci_get_device */
#include <linux/delay.h>	/* msleep */
#include <asm/uaccess.h>	/* access_ok, copy_to_user */

#include "trace.h"	/* TRACE */

#include "pcidvl_ioctl.h"

/* GLOBALS */

struct pci_dev *pci_dev_sp=0;

bar_info_t      pcie_bar_info;


//////////////////////////////////////////////////////////////////////////////



int dvl_ioctl(  struct inode *inode, struct file *filp
	       , unsigned int cmd, unsigned long arg )
{
    unsigned long       base;
    unsigned            jj;
    m_ioc_reg_access_t  reg_access;
    m_ioc_get_info_t	get_info;
    int			chn, dir, num;
    unsigned		myIdx;

    if(_IOC_TYPE(cmd) != DVL_IOC_MAGIC) return -ENOTTY;

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
	TRACE( 10, "dvl_ioctl: unknown cmd" );
	return (-1); // some error
    }
    return (0);
}   // dvl_ioctl


void free_mem( void )
{
    unsigned       chn, jj;

    for (chn=0; chn<2; ++chn)
    {
	// stop "app"
	Dma_mWriteReg( dvl_pcie_bar_info.baseVAddr, 0x9100+(0x100*chn), 0 );
	Dma_mWriteReg( dvl_pcie_bar_info.baseVAddr, 0x9108+(0x100*chn), 0 );
	msleep( 10 );

	// stop engines (both C2S and S2C channels)
	for (jj=0; jj<2; ++jj)  // this is "direction"
	{   Dma_mWriteChnReg( chn, jj, REG_DMA_ENG_CTRL_STATUS
			    , DMA_ENG_USER_RESET );
	    msleep( 10 );
	    Dma_mWriteChnReg( chn, jj, REG_DMA_ENG_CTRL_STATUS
			    , DMA_ENG_RESET );
	    msleep( 10 );
	}
    }

}   // free_mem




//////////////////////////////////////////////////////////////////////////////


static int __init init_dvl(void)
{
    int             ret=0;          /* SUCCESS */
    unsigned        chn, jj;
    void           *va;
    unsigned long   databuff_sz;
    unsigned long   buffdesc_sz;
    u32             descDmaAdr;

    TRACE( 2, "init_dvl" );

    // fs interface, pci, memory, events(i.e polling)

    ret = dvl_fs_up();
    ret = dvl_pci_up();

    dvl_pci_dev = pci_get_device( XILINX_VENDOR_ID, XILINX_DEVICE_ID, NULL );

    return (ret);

 out:
    TRACE( 0, "Error - freeing memory" );
    free_mem();
    dvl_pci_down();
    dvl_fs_down();
    return (-1);
}   // init_dvl


static void __exit exit_dvl(void)
{
    TRACE( 1, "exit_dvl() called");

    dvl_pci_down();
    dvl_fs_down();
}   // exit_dvl


module_init(init_dvl);
module_exit(exit_dvl);

MODULE_AUTHOR("Ron Rechenmacher");
MODULE_DESCRIPTION("dvl pcie driver");
MODULE_LICENSE("GPL"); /* Get rid of taint message by declaring code as GPL */
