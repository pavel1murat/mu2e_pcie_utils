/*  This file (mu2e_event.c) was created by Ron Rechenmacher <ron@fnal.gov> on
    Feb  5, 2014. "TERMS AND CONDITIONS" governing this file are in the README
    or COPYING file. If you do not have such a file, one can be obtained by
    contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
    $RCSfile: .emacs.gnu,v $
    rev="$Revision: 1.23 $$Date: 2012/01/23 15:32:40 $";
    */

#include <linux/timer.h>	/* del_timer_sync  */
#include <linux/mm.h>

#include "xdma_hw.h"		/* S2C, C2S, Dma_mReadChReg, Dma_mWriteReg */
#include "../trace/trace.h"	/* TRACE */
#include "mu2e_pci.h"		/* bar_info_t, extern mu2e_pci*  */
#include "mu2e_event.h"

#define PACKET_POLL_HZ 100

struct timer_list packets_timer;


static void poll_packets(unsigned long __opaque)
{
    unsigned long       base;
    int                 offset;

    /* DMA registers are offset from BAR0 */
    base = (unsigned long)(mu2e_pcie_bar_info.baseVAddr);

    // check channel 0 reciever
    TRACE( 2, "poll_packets: "
	  "CNTL=0x%08x "
	  "HW_NEXT=0x%08x "
	  "SW_NEXT=0x%08x "
	  "HW_LAST=0x%08x "
	  "COMPBYTS=0x%08x "
	  , Dma_mReadChReg( 0, C2S, REG_DMA_ENG_CTRL_STATUS )
	  , Dma_mReadChReg( 0, C2S, REG_DMA_ENG_NEXT_BD )
	  , Dma_mReadChReg( 0, C2S, REG_SW_NEXT_BD )
	  , Dma_mReadChReg( 0, C2S, REG_DMA_ENG_LAST_BD )
	  , Dma_mReadChReg( 0, C2S, REG_DMA_ENG_COMP_BYTES )
	  );
    TRACE( 3, "poll_packets: App0: gen=0x%x pktlen=0x%04x chk/loop=0x%x"
	  , Dma_mReadReg(base,0x9100), Dma_mReadReg(base,0x9104)
	  , Dma_mReadReg(base,0x9108)
	  );

    // Reschedule poll routine.
    offset = HZ / PACKET_POLL_HZ;
    packets_timer.expires = jiffies + offset;
    add_timer( &packets_timer );
}


//////////////////////////////////////////////////////////////////////////////

int mu2e_event_up( void )
{
    init_timer( &packets_timer );
    packets_timer.expires = jiffies + (HZ/PACKET_POLL_HZ);
    //timer->data=(unsigned long) pdev;
    packets_timer.function = poll_packets;
    add_timer( &packets_timer );

    return (0);
}

void mu2e_event_down( void )
{
    del_timer_sync( &packets_timer );
}
