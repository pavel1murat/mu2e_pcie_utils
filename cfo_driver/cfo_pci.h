 // This file (cfo_init.h) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Feb  5, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: .emacs.gnu,v $
 // rev="$Revision: 1.23 $$Date: 2012/01/23 15:32:40 $";

#ifndef CFO_PCI_H
#define CFO_PCI_H

#define XILINX_VENDOR_ID 0x10EE
#define XILINX_DEVICE_ID 0x7043

 /// <summary>
 /// Information about the base memory address of the CFO register space
 /// </summary>
typedef struct
{   unsigned long basePAddr;    /**< Base address of device memory */
    unsigned long baseLen;      /**< Length of device memory */
    void __iomem * baseVAddr;   /**< VA - mapped address */
} bar_info_t;

extern bar_info_t      cfo_pcie_bar_info;
extern struct pci_dev *cfo_pci_dev;


int  cfo_pci_up( void );
void cfo_pci_down( void );

#endif // CFO_PCI_H
