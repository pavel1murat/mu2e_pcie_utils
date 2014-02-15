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

#include "xdma_hw.h"		/* struct BuffDesc */

#include "../trace/trace.h"	/* TRACE */
#include "mu2e_fs.h"
#include "mu2e_pci.h"		/* bar_info_t, extern mu2e_pci*  */
#include "mu2e_event.h"
#include "mu2e_mmap_ioctl.h"
#include "mu2e_proto_globals.h"	/* MU2E_MAX_CHANNEL, etc. */


/* GLOBALS */

struct pci_dev *mu2e_pci_dev=0;

bar_info_t      mu2e_pcie_bar_info;


pci_sender_t mu2e_pci_sender[MU2E_NUM_SEND_CHANNELS]={{0},};

pci_recver_t mu2e_pci_recver[MU2E_NUM_RECV_CHANNELS]={{0},};

// This variable name is used in a macro that expects the same
// variable name in the user-space "library"
m_ioc_get_info_t mu2e_channel_info_[MU2E_MAX_CHANNELS][2];

// - - - - so far, just in this file - - - -
//    ch,dir,buffers/meta
volatile void *  mu2e_mmap_ptrs[MU2E_MAX_CHANNELS][2][2];


//////////////////////////////////////////////////////////////////////////////




int mu2e_mmap( struct file *file, struct vm_area_struct *vma )
{
    int	          ch, dir, map;
    unsigned long phys_addr;
    int           sts;

    page2chDirMap( vma->vm_pgoff, ch, dir, map );
    TRACE( 1, "mu2e_mmap: vm_pgoff:%u ch:%d dir:%d map:%d: %p"
	  , vma->vm_pgoff, ch, dir, map, mu2e_mmap_ptrs[ch][dir][map] );
    if (map == MU2E_MAP_META)
	vma->vm_flags&=~VM_WRITE;

    phys_addr = virt_to_phys( mu2e_mmap_ptrs[ch][dir][map] );
    sts = io_remap_pfn_range(  vma, vma->vm_start
                             , phys_addr >> PAGE_SHIFT
                             , vma->vm_end - vma->vm_start
                             , vma->vm_page_prot );

    return (0);
}   // mu2e_mmap

int mu2e_ioctl(  struct inode *inode, struct file *filp
	       , unsigned int cmd, unsigned long arg )
{
    unsigned long       base;
    unsigned            jj;
    m_ioc_reg_access_t  reg_access;
    m_ioc_get_info_t	get_info;
    int			chn, dir, num;
    unsigned		myIdx;

    if(_IOC_TYPE(cmd) != MU2E_IOC_MAGIC) return -ENOTTY;

    /* Check read/write and corresponding argument */
    if(_IOC_DIR(cmd) & _IOC_READ)
        if(!access_ok(VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd)))
            return -EFAULT;
    if(_IOC_DIR(cmd) & _IOC_WRITE)
        if(!access_ok(VERIFY_READ, (void *)arg, _IOC_SIZE(cmd)))
            return -EFAULT;

    /* DMA registers are offset from BAR0 */
    base = (unsigned long)(mu2e_pcie_bar_info.baseVAddr);

    switch(cmd)
    {
    case M_IOC_GET_TST_STATE:
	TRACE( 11, "mu2e_ioctl: cmd=GET_TST_STATE" );
	break;
    case M_IOC_TEST_START:
	TRACE( 11, "mu2e_ioctl: cmd=TEST_START" );
	// enable dma ch0/C2S w/GENERATOR
	Dma_mWriteChnReg( 0,C2S, REG_DMA_ENG_CTRL_STATUS, DMA_ENG_ENABLE );
	msleep( 20 );
	Dma_mWriteReg( base, 0x9100, 1 );  // 1=enable generator
	break;
    case M_IOC_TEST_STOP:
	TRACE( 11, "mu2e_ioctl: cmd=TEST_STOP" );
	break;
    case M_IOC_GET_PCI_STATE:
	TRACE( 11, "mu2e_ioctl: cmd=GET_PCI_STATE" );
	break;
    case M_IOC_GET_ENG_STATE:
	TRACE( 11, "mu2e_ioctl: cmd=GET_ENG_STATE" );
	break;
    case M_IOC_GET_DMA_STATS:
	TRACE( 11, "mu2e_ioctl: cmd=GET_DMA_STATS" );
	break;
    case M_IOC_GET_TRN_STATS:
	TRACE( 11, "mu2e_ioctl: cmd=GET_TRN_STATS" );
	break;
    case M_IOC_GET_SW_STATS:
	TRACE( 11, "mu2e_ioctl: cmd=GET_SW_STATS" );
	break;
    case M_IOC_REG_ACCESS:
	if(copy_from_user(&reg_access, (void*)arg, sizeof(reg_access)))
        {   printk("copy_from_user failed\n"); return (-EFAULT);
        }
	if (reg_access.access_type)
	{   // write
	    Dma_mWriteReg( base, reg_access.reg_offset, reg_access.val );
	}
	else
	{   // read
	    reg_access.val = Dma_mReadReg( base, reg_access.reg_offset );
	    if(copy_to_user((void*)arg, &reg_access, sizeof(reg_access)))
	    {   printk("copy_to_user failed\n"); return (-EFAULT);
	    }
	}
	break;
    case M_IOC_RECV:
	TRACE( 11, "mu2e_ioctl: cmd=RECV" );
	break;
    case M_IOC_SEND:		/* for this mu2e application, the drive
				   can automatically give send buffer to
				   the recv list WHEN the send dma is
				   complete */
	TRACE( 11, "mu2e_ioctl: cmd=SEND" );
	break;
    case M_IOC_GET_INFO:
	TRACE( 11, "mu2e_ioctl: cmd=GET_INFO" );
	if(copy_from_user(&get_info, (void*)arg, sizeof(m_ioc_get_info_t)))
        {   printk("copy_from_user failed\n"); return (-EFAULT);
        }
	get_info = mu2e_channel_info_[get_info.chn][get_info.dir];
	if(copy_to_user((void*)arg, &get_info, sizeof(m_ioc_get_info_t)))
	{   printk("copy_to_user failed\n"); return (-EFAULT);
	}
	break;
    case M_IOC_BUF_GIVE:
	chn= arg>>24;
	dir=(arg>>16)&1;
	num= arg&0xffff;
	TRACE( 11, "mu2e_ioctl: BUF_GIVE chn:%u dir:%u num:%u",chn,dir,num );
	myIdx = idx_add( mu2e_channel_info_[chn][dir].swIdx, num, chn, dir );
	Dma_mWriteChnReg( chn,dir, REG_SW_NEXT_BD, idx2descDmaAdr(myIdx,chn,dir) );
	mu2e_channel_info_[chn][dir].swIdx = myIdx; 
	break;
    case M_IOC_DUMP:
	for (jj=0; jj<MU2E_NUM_RECV_BUFFS; ++jj)
	{    TRACE( 10, "%3u %2x 0x%08x", jj, 0
		   , ((u32*)&(mu2e_pci_recver[0].buffdesc_ring[jj]))[0] );
	     TRACE( 10, "%3u %2x 0x%08x", jj, 4
		   , ((u32*)&(mu2e_pci_recver[0].buffdesc_ring[jj]))[1] );
	     TRACE( 10, "%3u %2d 0x%08x", jj, 8
		   , ((u32*)&(mu2e_pci_recver[0].buffdesc_ring[jj]))[2] );
	     TRACE( 10, "%3u %2x 0x%08x", jj, 12
		   , ((u32*)&(mu2e_pci_recver[0].buffdesc_ring[jj]))[3] );
	     TRACE( 10, "%3u %2x 0x%08x", jj, 16
		   , ((u32*)&(mu2e_pci_recver[0].buffdesc_ring[jj]))[4] );
	     TRACE( 10, "%3u %2x 0x%08x", jj, 20
		   , ((u32*)&(mu2e_pci_recver[0].buffdesc_ring[jj]))[5] );
	     TRACE( 10, "%3u %2x 0x%08x", jj, 24
		   , ((u32*)&(mu2e_pci_recver[0].buffdesc_ring[jj]))[6] );
	     TRACE( 10, "%3u %2x 0x%08x", jj, 28
		   , mu2e_pci_recver[0].buffdesc_ring[jj].NextDescPtr );
	     TRACE( 10, "%3u meta@%p[%d]=%u", jj
		   , mu2e_mmap_ptrs[0][C2S][MU2E_MAP_META], jj
		   , ((u32*)(mu2e_mmap_ptrs[0][C2S][MU2E_MAP_META]))[jj] );
	     TRACE( 10, "%3u 0x%08x 0x%08x 0x%08x 0x%08x", jj
		   , ((u32*)&(mu2e_pci_recver[0].databuffs[jj]))[0]
		   , ((u32*)&(mu2e_pci_recver[0].databuffs[jj]))[1]
		   , ((u32*)&(mu2e_pci_recver[0].databuffs[jj]))[2]
		   , ((u32*)&(mu2e_pci_recver[0].databuffs[jj]))[3] );
	}
	break;
    default:
	TRACE( 10, "mu2e_ioctl: unknown cmd" );
	return (-1); // some error
    }
    return (0);
}   // mu2e_ioctl


void free_mem( void )
{
    unsigned       chn, jj;

    for (chn=0; chn<2; ++chn)
    {
	// stop "app"
	Dma_mWriteReg( mu2e_pcie_bar_info.baseVAddr, 0x9100+(0x100*chn), 0 );
	Dma_mWriteReg( mu2e_pcie_bar_info.baseVAddr, 0x9108+(0x100*chn), 0 );
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

    for (chn=0; chn<MU2E_NUM_RECV_CHANNELS; ++chn)
    {
	if (mu2e_pci_recver[chn].databuffs)
	    dma_free_coherent( &mu2e_pci_dev->dev
			      , sizeof(mu2e_databuff_t)*MU2E_NUM_RECV_BUFFS
			      , mu2e_pci_recver[chn].databuffs
			      , mu2e_pci_recver[chn].databuffs_dma );
	if (mu2e_pci_recver[chn].buffdesc_ring)
	    dma_free_coherent(  &mu2e_pci_dev->dev
			      , sizeof(mu2e_buffdesc_C2S_t)*MU2E_NUM_RECV_BUFFS
			      , mu2e_pci_recver[chn].buffdesc_ring
			      , mu2e_pci_recver[chn].buffdesc_ring_dma );
	free_pages( (unsigned long)mu2e_mmap_ptrs[chn][C2S][MU2E_MAP_META], 0 );
    }
    for (chn=0; chn<MU2E_NUM_SEND_CHANNELS; ++chn)
    {
	if (mu2e_pci_sender[chn].databuffs)
	    dma_free_coherent( &mu2e_pci_dev->dev
			      , sizeof(mu2e_databuff_t)*MU2E_NUM_SEND_BUFFS
			      , mu2e_pci_sender[chn].databuffs
			      , mu2e_pci_sender[chn].databuffs_dma );
	if (mu2e_pci_sender[chn].buffdesc_ring)
	    dma_free_coherent(  &mu2e_pci_dev->dev
			      , sizeof(mu2e_buffdesc_S2C_t)*MU2E_NUM_SEND_BUFFS
			      , mu2e_pci_sender[chn].buffdesc_ring
			      , mu2e_pci_sender[chn].buffdesc_ring_dma );
	free_pages( (unsigned long)mu2e_mmap_ptrs[chn][S2C][MU2E_MAP_META], 0 );
    }
}   // free_mem


#define descIdx2dmaAddr( idx, ch, dir )


//////////////////////////////////////////////////////////////////////////////


static int __init init_mu2e(void)
{
    int             ret=0;          /* SUCCESS */
    unsigned        chn, jj;
    void           *va;
    unsigned long   databuff_sz;
    unsigned long   buffdesc_sz;
    u32             descDmaAdr;

    TRACE( 2, "init_mu2e" );

    // fs interface, pci, memory, events(i.e polling)

    ret = mu2e_fs_up();
    ret = mu2e_pci_up();

    mu2e_pci_dev = pci_get_device( XILINX_VENDOR_ID, XILINX_DEVICE_ID, NULL );

    for (chn=0; chn<MU2E_NUM_RECV_CHANNELS; ++chn)
    {
	databuff_sz = sizeof(mu2e_databuff_t)*MU2E_NUM_RECV_BUFFS;
	buffdesc_sz = sizeof(mu2e_buffdesc_C2S_t)*MU2E_NUM_RECV_BUFFS;
	TRACE( 1,"init_mu2e dma_alloc databuff_sz=%lu buffdesc_sz=%lu"
	      , databuff_sz, buffdesc_sz  );
	va = dma_alloc_coherent(  &mu2e_pci_dev->dev, databuff_sz
				, &mu2e_pci_recver[chn].databuffs_dma
				, GFP_KERNEL );
	if (va == NULL) goto out;
	mu2e_pci_recver[chn].databuffs = va;
	mu2e_mmap_ptrs[chn][C2S][MU2E_MAP_BUFF] = va;
	va = dma_alloc_coherent(  &mu2e_pci_dev->dev, buffdesc_sz
				, &mu2e_pci_recver[chn].buffdesc_ring_dma
				, GFP_KERNEL );
	if (va == NULL) goto out;
	mu2e_pci_recver[chn].buffdesc_ring = va;
	mu2e_mmap_ptrs[chn][C2S][MU2E_MAP_META]
	    = (void*)__get_free_pages(GFP_KERNEL,0);
	TRACE( 1, "mu2e_pci_recver[%u].databuffs=%p databuffs_dma=0x%llx "
	      "buffdesc_ring_dma=0x%llx meta@%p"
	      , chn, mu2e_pci_recver[chn].databuffs
	      , mu2e_pci_recver[chn].databuffs_dma
	      , mu2e_pci_recver[chn].buffdesc_ring_dma
	      , mu2e_mmap_ptrs[chn][C2S][MU2E_MAP_META] );
	mu2e_channel_info_[chn][C2S].chn = chn;
	mu2e_channel_info_[chn][C2S].dir = C2S;
	mu2e_channel_info_[chn][C2S].buff_size = sizeof(mu2e_databuff_t);
	mu2e_channel_info_[chn][C2S].num_buffs = MU2E_NUM_RECV_BUFFS;
	for (jj=0; jj<MU2E_NUM_RECV_BUFFS; ++jj)
	{   /* ring -> link to next (and last to 1st via modulus) */
	    mu2e_pci_recver[chn].buffdesc_ring[jj].NextDescPtr =
		mu2e_pci_recver[chn].buffdesc_ring_dma
		+ sizeof(mu2e_buffdesc_C2S_t) * ((jj+1)%MU2E_NUM_RECV_BUFFS);
	    /* put the _buffer_ address in the descriptor */
	    mu2e_pci_recver[chn].buffdesc_ring[jj].SystemAddress =
		mu2e_pci_recver[chn].databuffs_dma
		+ sizeof(mu2e_databuff_t) * jj;
	    /* and the size of the buffer also */
	    mu2e_pci_recver[chn].buffdesc_ring[jj].RsvdByteCnt =
		sizeof(mu2e_databuff_t);
	}
	TRACE( 1,"write 0x%llx to 32bit reg",mu2e_pci_recver[chn].buffdesc_ring_dma );
	Dma_mWriteChnReg( chn, C2S, REG_DMA_ENG_CTRL_STATUS, DMA_ENG_RESET );
	msleep( 20 );
	Dma_mWriteChnReg( chn, C2S, REG_HW_NEXT_BD
			, (u32)mu2e_pci_recver[chn].buffdesc_ring_dma );
	mu2e_channel_info_[chn][C2S].hwIdx = 0;	

	// set "DMA_ENG" (ie. HW) last/complete == SW NEXT to show "num avail" == 0
	descDmaAdr = idx2descDmaAdr( MU2E_NUM_RECV_BUFFS-1, chn, C2S );
	Dma_mWriteChnReg( chn, C2S, REG_SW_NEXT_BD,      descDmaAdr );
	Dma_mWriteChnReg( chn, C2S, REG_HW_CMPLT_BD, descDmaAdr );
	mu2e_channel_info_[chn][C2S].hwIdx = MU2E_NUM_RECV_BUFFS-1;
	mu2e_channel_info_[chn][C2S].swIdx = MU2E_NUM_RECV_BUFFS-1;
    }
    for (chn=0; chn<MU2E_NUM_SEND_CHANNELS; ++chn)
    {
	TRACE( 1,"init_mu2e alloc %ld",sizeof(mu2e_databuff_t)*MU2E_NUM_SEND_BUFFS );
	va = dma_alloc_coherent(  &mu2e_pci_dev->dev
				, sizeof(mu2e_databuff_t)*MU2E_NUM_SEND_BUFFS
				, &mu2e_pci_sender[chn].databuffs_dma
				, GFP_KERNEL );
	if (va == NULL) goto out;
	mu2e_pci_sender[chn].databuffs = va;
	mu2e_mmap_ptrs[chn][S2C][MU2E_MAP_BUFF] = va;
	va = dma_alloc_coherent(  &mu2e_pci_dev->dev
				, sizeof(mu2e_buffdesc_S2C_t)*MU2E_NUM_SEND_BUFFS
				, &mu2e_pci_sender[chn].buffdesc_ring_dma
				, GFP_KERNEL );
	if (va == NULL) goto out;
	mu2e_pci_sender[chn].buffdesc_ring = va;
	mu2e_mmap_ptrs[chn][S2C][MU2E_MAP_META]
	    = (void*)__get_free_pages(GFP_KERNEL,0);
	TRACE( 1, "mu2e_pci_sender[%u].databuffs=%p databuffs_dma=0x%llx "
	      "buffdesc_ring_dma=0x%llx"
	      , chn, mu2e_pci_sender[chn].databuffs
	      , mu2e_pci_sender[chn].databuffs_dma
	      , mu2e_pci_sender[chn].buffdesc_ring_dma );
	mu2e_channel_info_[chn][S2C].buff_size = sizeof(mu2e_databuff_t);
	mu2e_channel_info_[chn][S2C].num_buffs = MU2E_NUM_SEND_BUFFS;
	for (jj=0; jj<MU2E_NUM_SEND_BUFFS; ++jj)
	{   mu2e_pci_sender[chn].buffdesc_ring[jj].NextDescPtr =
		mu2e_pci_sender[chn].buffdesc_ring_dma
		+ sizeof(mu2e_buffdesc_S2C_t) * ((jj+1)%MU2E_NUM_SEND_BUFFS);
	}
    }

    ret = mu2e_event_up();
    return (ret);

 out:
    TRACE( 0, "Error - freeing memory" );
    free_mem();
    mu2e_pci_down();
    mu2e_fs_down();
    return (-1);
}   // init_mu2e


static void __exit exit_mu2e(void)
{
    TRACE( 1, "exit_mu2e() called");

    // events, memory, pci, fs interface
    mu2e_event_down();
    free_mem();
    mu2e_pci_down();
    mu2e_fs_down();
}   // exit_mu2e


module_init(init_mu2e);
module_exit(exit_mu2e);

MODULE_AUTHOR("Ron Rechenmacher");
MODULE_DESCRIPTION("mu2e pcie driver");
MODULE_LICENSE("GPL"); /* Get rid of taint message by declaring code as GPL */
