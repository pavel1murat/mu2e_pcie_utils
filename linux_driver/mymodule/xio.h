/*******************************************************************************
** © Copyright 2011 - 2012 Xilinx, Inc. All rights reserved.
** This file contains confidential and proprietary information of Xilinx, Inc. and
** is protected under U.S. and international copyright and other intellectual property laws.
*******************************************************************************
**   ____  ____
**  /   /\/   /
** /___/  \  /   Vendor: Xilinx
** \   \   \/
**  \   \
**  /   /
** /___/   /\x
** \   \  /  \   Kintex-7 PCIe-DMA-DDR3 Base Targeted Reference Design
**  \___\/\___\
**
**  Device: xc7k325t
**  Reference: UG882
*******************************************************************************
**
**  Disclaimer:
**
**    This disclaimer is not a license and does not grant any rights to the materials
**    distributed herewith. Except as otherwise provided in a valid license issued to you
**    by Xilinx, and to the maximum extent permitted by applicable law:
**    (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL FAULTS,
**    AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY,
**    INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-INFRINGEMENT, OR
**    FITNESS FOR ANY PARTICULAR PURPOSE; and (2) Xilinx shall not be liable (whether in contract
**    or tort, including negligence, or under any other theory of liability) for any loss or damage
**    of any kind or nature related to, arising under or in connection with these materials,
**    including for any direct, or any indirect, special, incidental, or consequential loss
**    or damage (including loss of data, profits, goodwill, or any type of loss or damage suffered
**    as a result of any action brought by a third party) even if such damage or loss was
**    reasonably foreseeable or Xilinx had been advised of the possibility of the same.


**  Critical Applications:
**
**    Xilinx products are not designed or intended to be fail-safe, or for use in any application
**    requiring fail-safe performance, such as life-support or safety devices or systems,
**    Class III medical devices, nuclear facilities, applications related to the deployment of airbags,
**    or any other applications that could lead to death, personal injury, or severe property or
**    environmental damage (individually and collectively, "Critical Applications"). Customer assumes
**    the sole risk and liability of any use of Xilinx products in Critical Applications, subject only
**    to applicable laws and regulations governing limitations on product liability.

**  THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE AT ALL TIMES.

*******************************************************************************/
/*****************************************************************************/
/**
*
* @file xio.h
*
* This file contains the Input/Output functions, and the changes
* required for swapping endianness.
*
* @note
*
******************************************************************************/

#ifndef XIO_H           /* prevent circular inclusions */
#define XIO_H           /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xbasic_types.h"
#ifdef __KERNEL__
#include <asm/io.h>
#endif


/**************************** Type Definitions *******************************/

/**
 * Typedef for an I/O address.  Typically correlates to the width of the
 * address bus.
 */
typedef Xuint32 XIo_Address;


/* NWL DMA design is little-endian, so values need not be swapped.
 */
#define XIo_In32(addr) \
({ u32 xx=readl((unsigned int *)(addr));\
    TRACE( 1, "read: 0x%x=%p",xx,(void*)(addr));        \
    xx;\
 })

#define XIo_Out32(addr, data) \
    ({  TRACE( 1, "write %p=0x%x",(void*)(addr),data);\
        writel((data), (unsigned int *)(addr));      \
    })


#ifdef __cplusplus
}
#endif

#endif          /* end of protection macro */
