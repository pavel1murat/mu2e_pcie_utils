 // This file (cfo_proto_globals.h) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Feb 12, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: .emacs.gnu,v $
 // rev="$Revision: 1.23 $$Date: 2012/01/23 15:32:40 $";
#ifndef CFO_PROTO_GLOBALS_H
#define CFO_PROTO_GLOBALS_H

#include "cfo_mmap_ioctl.h"		// m_ioc_get_info_t
#include "xdma_hw.h"			// cfo_buffdesc_C2S_t

/* At 10 Gigabits-per-second... (approx. 1 GigaByte/s)
   60*65536 ~= 3.9 MB / 1000 MB/s ~= 3.9 ms.
   250 Hz should work.
 */
#define CFO_NUM_RECV_BUFFS      4 /*4*/  /*20*/
#define CFO_NUM_RECV_CHANNELS   2
#define CFO_RECV_INTER_ENABLED  1
extern cfo_databuff_t       *cfo_recv_databuff_rings[CFO_NUM_RECV_CHANNELS];
extern cfo_buffdesc_C2S_t   *cfo_recv_buffdesc_rings[CFO_NUM_RECV_CHANNELS];

#define CFO_NUM_SEND_BUFFS     4 /*20*/ /*60*/ /*Maximum value: ~60*/
#define CFO_NUM_SEND_CHANNELS   2
extern cfo_databuff_t       *cfo_send_databuff_rings[CFO_NUM_SEND_CHANNELS];
extern cfo_buffdesc_S2C_t   *cfo_send_buffdesc_rings[CFO_NUM_SEND_CHANNELS];

/// <summary>
/// PCI Sending interface data
/// </summary>
typedef struct
{   cfo_databuff_t     *databuffs; ///< Data buffers for transmitting
    dma_addr_t           databuffs_dma; ///< DMA address for buffers for transmitting
    cfo_buffdesc_S2C_t *buffdesc_ring; ///< Descriptors for transmitting
    dma_addr_t           buffdesc_ring_dma; ///< DMA address for descriptors for transmitting
} pci_sender_t;

extern pci_sender_t cfo_pci_sender[CFO_NUM_SEND_CHANNELS];

/// <summary>
/// PCI Receiving interface data
/// </summary>
typedef struct
{   cfo_databuff_t     **databuffs; ///< pointers to data buffers for receiving
    dma_addr_t          *databuffs_dma; ///< DMA address of data buffers for receiving
    cfo_buffdesc_C2S_t **buffdesc_ring; ///< Pointers to buffer descriptors for receiving
    dma_addr_t          *buffdesc_ring_dma; ///< DMA Address of buffer descriptor ring for receiving
} pci_recver_t;

extern pci_recver_t cfo_pci_recver[CFO_NUM_RECV_CHANNELS];

extern volatile void *  cfo_mmap_ptrs[CFO_MAX_CHANNELS][2][2];

// This variable name is used in a macro that expects the same
// variable name in the user-space "library"
extern m_ioc_get_info_t cfo_channel_info_[CFO_MAX_CHANNELS][2];



#endif // CFO_PROTO_GLOBALS_H
