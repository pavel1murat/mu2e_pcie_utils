/*  This file (mu2e_event.c) was created by Ron Rechenmacher <ron@fnal.gov> on
	Feb  5, 2014. "TERMS AND CONDITIONS" governing this file are in the README
	or COPYING file. If you do not have such a file, one can be obtained by
	contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
	$RCSfile: .emacs.gnu,v $
	rev="$Revision: 1.23 $$Date: 2012/01/23 15:32:40 $";
	*/

#include <linux/timer.h>	/* del_timer_sync  */
#include <linux/mm.h>
#include "trace.h"		/* TRACE */

#include "xdma_hw.h"		/* S2C, C2S, Dma_mReadChReg, Dma_mWriteReg */
#include "mu2e_proto_globals.h"	/* mu2e_channel_info */
#include "mu2e_pci.h"		/* bar_info_t, extern mu2e_pci*  */
#include "mu2e_event.h"

#define PACKET_POLL_HZ 1000

struct timer_list packets_timer;


static void poll_packets(unsigned long __opaque)
{
	unsigned long       base;
	int                 offset;
	int			chn, dir;
	unsigned            nxtCachedCmpltIdx;
	mu2e_buffdesc_C2S_t *buffdesc_C2S_p;

	/* DMA registers are offset from BAR0 */
	base = (unsigned long)(mu2e_pcie_bar_info.baseVAddr);

	// check channel 0 reciever
	TRACE(22, "poll_packets: "
		"CNTL=0x%08x "
		"H_NEXT=%u "
		"S_NEXT=%u "
		"H_CPLT=%u "
		"CPBYTS=0x%08x "
		, Dma_mReadChnReg(0, C2S, REG_DMA_ENG_CTRL_STATUS)
		, descDmaAdr2idx(Dma_mReadChnReg(0, C2S, REG_HW_NEXT_BD), 0, C2S)
		, descDmaAdr2idx(Dma_mReadChnReg(0, C2S, REG_SW_NEXT_BD), 0, C2S)
		, descDmaAdr2idx(Dma_mReadChnReg(0, C2S, REG_HW_CMPLT_BD), 0, C2S)
		, Dma_mReadChnReg(0, C2S, REG_DMA_ENG_COMP_BYTES)
		);
	TRACE(23, "poll_packets: App0: gen=0x%x pktlen=0x%04x chk/loop=0x%x"
		, Dma_mReadReg(base, 0x9100), Dma_mReadReg(base, 0x9104)
		, Dma_mReadReg(base, 0x9108)
		);

	dir = C2S;
	for (chn = 0; chn < MU2E_MAX_CHANNELS; ++chn)
	{   // Read the HW register and convert (Dma) addr in reg to idx.
		u32 newCmpltIdx = descDmaAdr2idx(Dma_mReadChnReg(chn, dir, REG_HW_CMPLT_BD), chn, dir);

		u32 do_once = 0;

		if (newCmpltIdx >= MU2E_NUM_RECV_BUFFS)
		{
			TRACE(0, "poll_packets: newCmpltIdx (0x%lx) is above maximum sane value!!! (%lu) Current idx=0x%x", newCmpltIdx, MU2E_NUM_RECV_BUFFS, mu2e_channel_info_[chn][dir].hwIdx);
			continue;
		}
		// check just-read-HW-val (converted to idx) against "cached" copy
		while (newCmpltIdx != mu2e_channel_info_[chn][dir].hwIdx/*ie.cachedCmplt*/)
		{   // NEED TO UPDATE Receive Byte Counts
			int * BC_p = (int*)mu2e_mmap_ptrs[chn][dir][MU2E_MAP_META];
			nxtCachedCmpltIdx = idx_add(mu2e_channel_info_[chn][dir].hwIdx, 1, chn, dir);
			buffdesc_C2S_p = idx2descVirtAdr(nxtCachedCmpltIdx, chn, dir);
			BC_p[nxtCachedCmpltIdx] = buffdesc_C2S_p->ByteCount;
			TRACE(4
				, "poll_packets: chn=%d dir=%d %p[idx=%u]=byteCnt=%d newCmpltIdx=%u"
				, chn, dir, (void*)BC_p, nxtCachedCmpltIdx
				, buffdesc_C2S_p->ByteCount, newCmpltIdx);
			mu2e_channel_info_[chn][dir].hwIdx = nxtCachedCmpltIdx;
			// Now system SW can see another buffer with valid meta data
			do_once = 1;
		}
		if (do_once)
		{   /* and wake up the user process waiting for data */
			wake_up_interruptible(&get_info_wait_queue);
		}
	}

	// S2C checked in xmit ioctl or write

	// Reschedule poll routine.
	offset = HZ / PACKET_POLL_HZ;
	packets_timer.expires = jiffies + offset;
	add_timer(&packets_timer);
}


//////////////////////////////////////////////////////////////////////////////

int mu2e_event_up(void)
{
	init_timer(&packets_timer);
	packets_timer.expires = jiffies + (HZ / PACKET_POLL_HZ);
	//timer->data=(unsigned long) pdev;
	packets_timer.function = poll_packets;
	add_timer(&packets_timer);

	return (0);
}

void mu2e_event_down(void)
{
	del_timer_sync(&packets_timer);
}
