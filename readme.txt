
/*******************************************************************************
** © Copyright 2011 - 2012 Xilinx, Inc. All rights reserved.
** This file contains confidential and proprietary information of Xilinx, Inc. and
** is protected under U.S. and international copyright and other intellectual property laws.
*******************************************************************************
**   ____  ____
**  /   /\/   /
** /___/  \  /   Vendor: Xilinx
** \   \   \/
**  \   \        readme.txt 
**  /   /
** /___/   /\
** \   \  /  \   Kintex-7 PCIe-DMA-DDR3 Base Targeted Reference Design
**  \___\/\___\
**
**  Device: xc7k325t
**  Purpose: Targeted Reference Design
**  Reference: UG882 
**  TRD version: 1.0  
**
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
**
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

1.  REQUIREMENTS
    ------------
    a. Hardware
      i.  KC705 Evaluation Kit (Rev D board with GES silicon)
      ii. PC with PCI Express slot (x4/x8/x16 PCIe v2.0 compliant)
      iii.Keyboard & Mouse

    b. Software
      i. ISE Design Suite version 13.4
        - ISE install
        - Before running any command line scripts, refer to the ISE 13.4 Installation and Licensing
          document to learn how to set the appropriate environment variables for your operating system.
          All scripts mentioned in this readme file assume the XILINX environment variable has been set.

          For windows use ISE Design Suite command prompt (available in Accessories of ISE Design Suite installation)
          

2. DIRECTORY STRUCTURE
   -------------------

    k7_pcie_dma_ddr3_base : Main TRD folder
    |
    |--design : Design files and scripts for simulation & implementation
    |  |
    |  |
    |  |---ip_cores : IPs delivered from Xilinx CORE Generator and Third Party IPs
    |  |   |
    |  |   |----dma (IP provided by Northwest Logic - Xilinx Third Party Alliance version 1.07). This TRD is using
    |  |   |        the evaluation version of the DMA which times out after 12 hours. To use the full version 
    |  |   |        please contact Northwest Logic
    |  |   |
    |  |   |----pcie (Kintex-7 Integrated Block for PCI Express core version 1.3 generated through Xilinx CORE Generator using
    |  |   |          the xco file provided in this directory)
    |  |   |
    |  |   |----axi_ic (AXI Inteconnect core version 1.05.a generated through Xilinx CORE Generator using 
    |  |   |          the xco file provided in this directory)
    |  |   |
    |  |   |----fifo (Fifo_Generator core version 8.4 generated through Xilinx CORE Generator using 
    |  |   |          the xco file provided in this directory)
    |  |   |
    |  |   |----mig (source files for Memory Interface Generator(MIG) are not available in this folder. 
    |  |   |         Users are required to invoke Xilinx CORE Generator and populate this folder with version 1.4
    |  |   |         of the core using the xco file provided in this directory) 
    |  |   |
    |  |   |----reference ("golden" xco and prj files to regenerate Xilinx IP cores)
    |  |
    |  |
    |  |---implement  : Implementation scripts for Linux, UCF files specific to KC705. Sub folders for PlanAhead flow.
    |  |
    |  |---sim : Simulation folder
    |  |   |
    |  |   |----include : Common files for simulation
    |  |   |
    |  |   |----mti : ModelSim simulation script
    |  |
    |  |---source : Source deliverables for the TRD
    |  |   |
    |  |   |----modified_ip_files : IP source files that have been modified for this reference design 
    |  |        | 
    |  |        |---- mig  : mig_7x.v top level for the MIG core has been modified to support the   
    |  |        |             KC705 Evaluation board. 
    |  |        | 
    |  |        |---- dma : packet_dma_axi.v is top level dma file created for this TRD. It is based on top level 
    |  |        |           dma file provided by NWL. register_map.v has been modified to add TRD regsiters.
    |  |        |
    |  | 
    |  |---tb : Testbench for out-of-box simulation
    |      |
    |      |----dsport  : Wrapper files and tasks for downstream port
    |
    |---configuring_kc705 : bit file for x4 Gen2 PCIe configuration 
    |
    |--doc  : Getting Started Guide for the TRD
    |
    |--linux_driver : Software driver source and GUI related files
    |
    |--k7_lin_trd_quickstart : Script that automatically runs Makefiles for easy operation



3. IP CORE GENERATION
   ------------------
   The MIG IP core cannot be delivered as a part of the TRD because customers have to accept license on Micron Models
   required for simulating the TRD. Before trying to implement or simulate the TRD this step should be completed
   
   Generating the MIG IP core through Core Generator
   a. Open a terminal window (Linux) or a ISE Design Suite Command Prompt (Windows)
   b. Navigate to k7_pcie_dma_ddr3_base/design/ip_cores/mig (This directory has mig.xco, mig.prj and coregen.cgp files)
   c. Invoke Core Generator tool
      $ coregen 
   d. In the Core Generator tool Click on File --> Open project --> Select coregen.cgp. 
   e. Double click on Instance Name ‘mig_7x’. This will pop up the Memory Interface Geneartor GUI with the configuration defined 
      by mig.xco and mig.prj files. 
   f. Click on Next until the Micron Tech Inc Simulation Model  License Agreement page. Select Accept and Click on Next. 
      This selection will generate the memory models required for simulation.
   g. In the following page click on Next. Then click on Generate to create the MIG IP core. 
   h. Close the Readme Notes Window and then Core Generator GUI.
   

4. IMPLEMENTATION FLOW
   -------------------
   a. Command Line flow for Linux

      Navigate to 'design/implement' folder

      Execute the following to generate a design with x4 gen2 PCIe configuration
        $ ./implement.sh x4 gen2

      Execute the following to generate a design with x8 gen1 PCIe configuration
        $ ./implement.sh x8 gen1
      
      Note: If the configuration width selected is x8 gen1 then the following changes need to be made to the driver code to work 
      with the TRD
        - Set PCI_DEVICE_ID_DMA to 0x7081 in driver/dma/xdma_base.c

      The following command shows other options supported by the script
        $ ./implement.sh -help
      Since it is difficult to meet timing on -1 devices, users may need to run map and par with several cost table values.
      The implementation script allows the user to set a cost table value through the implementation script.  Use the tag
      option of implement.sh to make output directories unique.  Every effort has been made to have the default cost table
      meet timing, but due to varying conditions the default cost table cannot be guaranteed to meet timing.

   b. Command Line flow Windows

      Navigate to 'design/implement on a ISE Design Suite command window

      Execute the following to generate a design with x4 gen2 PCIe configuration
        $ implement.bat -lanemode x4gen2

      Execute the following to generate a design with x8 gen1 PCIe configuration
        $ implement.bat -lanemode x8gen1
      Note: If the configuration width selected is x8 gen1 then the following changes need to be made to the driver code to work 
      with the TRD
        - Set PCI_DEVICE_ID_DMA to 0x7081 in driver/dma/xdma_base.c


      The following command shows other options supported by the script
        $ implement.bat -help
     
   c. PlanAhead flow for Windows and Linux 

      Navigate to 'design/implement/planahead_flow_x4gen2 on a ISE Design Suite command window 
      Run the following command to invoke the PlanAhead GUI. The design with x4 gen2 PCIe configuration is loaded. 
        $ launch_pa_x4gen2.bat
        Click on Synthesize in the Project Manager window. A window with message "Synthesis Completed Successfully" will appear
        after XST generates a design netlist. Close the message window.

        Click on Implement in the Project Manager window. A window with message "Implementation Completed Successfully" will appear
        after translate, map and par processes are done. Close the message window.

        Click on Program & Debug -> Click on Generate Bitstream. An options window will appear.  
        In the column next to the -f field, browse to directory design/implement and select bitgen_option.ut. 
        
        Click on OK to generate bitstream.
        A window with message "Generate Bitstream Completed Successfully" will appear at the end of this process and a design bit file 
        will be available in design/implement/planahead_flow_x8gen1/planAhead_run_1/k7_pcie_dma_ddr3_base_x4gen2.runs/impl_1
       
      Close planAhead GUI.
      
      Run the following command to generate a mcs file
        
        Windows: $ genprom.bat
        Linux:   $ ./genprom.sh
       
      A promgen file will be available in design/implement/planahead_flow_x4gen2
      
      
      Navigate to 'design/implement/planahead_flow_x8gen1 on a command window 
      Run the following command to invoke the PlanAhead GUI. The design with x8 gen1 PCIe configuration is loaded. 
        $ launch_pa_x8gen1.bat
        Click on Synthesize in the Project Manager window. A window with message "Synthesis Completed Successfully" will appear
        after XST generates a design netlist. Close the message window.

        Click on Implement in the Project Manager window. A window with message "Implementation Completed Successfully" will appear
        after translate, map and par processes are done. Close the message window.

        Click on Program & Debug -> Click on Generate Bitstream. An options window will appear.  
        In the column next to the -f field, browse to directory design/implement and select bitgen_option.ut. 
        
        Click on OK to generate bitstream.
        A window with message "Generate Bitstream Completed Successfully" will appear at the end of this process and a design bit file 
        will be available in design/implement/planahead_flow_x8gen1/planAhead_run_1/k7_pcie_dma_ddr3_base_x8gen1.runs/impl_1
       
      Close planAhead GUI.
      
      Run the following command to generate a mcs file
        
        Windows: $ genprom.bat
        Linux:   $ ./genprom.sh
       
      A promgen file will be available in design/implement/planahead_flow_x8gen1

      Note: If the configuration width selected is x8 gen1 then the following changes need to be made to the driver code to work 
      with the TRD
        - Set PCI_DEVICE_ID_DMA to 0x7081 in driver/dma/xdma_base.c

5. SIMULATION FLOW
   ---------------
   Requires Xilinx tools and ModelSim.
   a. Simulation library compilation
        Compile Xilinx Simulation Libraries using compxlib.

   b. Simulation using the scripts provided
        Navigate to the 'design/sim/mti' folder.
        Set MODELSIM environment variable to point to the 'modelsim.ini' file which contains paths to the compiled libraries.
        Run 'simulate_mti_x4gen2.bat' script to simulate the design with PCIe core confgiured as x4 link.
        Run 'simulate_mti_x8gen1.bat' script to simulate the design with PCIe core confgiured as x8 link.

6. TESTING
   -------
   To test the design in hardware, refer to Getting Started Chapter in UG882 available in the 'doc' folder.


7. KNOWN RESTRICTIONS
   ------------------
   
   a. DDR3 may not calibrate successfully on some GES xc7k325t parts. If LED3 on the KC705 is OFF then memory calibration is not done. Apply the patch
      provided in AR45653 to resolve this issue.
   
   b. If design files are changed there is a possibility that timing will not be met. Users may need to run Map with different cost table values to meet
      timing. The implementation script allows the user to set a cost table value.  Use the tag option of implement.sh to make output directories unique. 
      Every effort has been made to have the default cost table meet timing, but due to varying conditions the  default cost table cannot be guaranteed 
      to meet timing.
   
   c. When implementing the design in the planAhead flow, 50 Critical warnings are reported with regards to clock period (REFCLK_PERIOD, MEMREFCLK_PERIOD,
      PHASEREFCLK_PERIOD). Ignore these warnings. This issue will be fixed in the next version of ISE
      
   d. On some Intel x58 chipsets the Base TRD driver doesn't unload successfully(driver module xdma_k7 is not removed). UG882 details two ways to remove the driver 
        i.    Using the  k7_trd_lin_quickstart script, which removes the driver when the Peformance Montior is closed.
        ii.   Using the make remove command (Appendix E).
      This issue can be resolved as follows.When Fedora 16 Live is booting, 
        i.    Press tab for full configuration options on menu items. 
        ii.   Add iommu=pt64 at the end of the line vmlinuz0 initrd=initrd0 ...rd.md=0 rd.dm=0
        iii.  Hit the enter key to boot with this option. 
  
   e. On some Intel x58 chipsets the DMA performance on Base TRD fluctuates and is lower than expected. 
      This issue can be resolved as follows.When Fedora 16 Live is booting, 
        i.    Press tab for full configuration options on menu items. 
        ii.   Add iommu=pt64 at the end of the line vmlinuz0 initrd=initrd0 ...rd.md=0 rd.dm=0
        iii.  Hit the enter key to boot with this option. 

   f. Fedora 16 may not boot on all PCs. Here are two scenarios.  
        i.    Fedora 16 was released November 8, 2011 and uses open source drivers. PC systems containing motherboards with chipsets or graphics cards that are developed 
              later than this date or from a vendor who does not support the open source drivers many not work. Note* NVIDIA does not directly support the open source drivers 
              so some cards may be not compatible with FC16. This link will give more details on NVIDIA HW families. http://nouveau.freedesktop.org.
        ii.   Fedora 16 uses GNOME 3  for its graphical environment and this requires 3d HW acceleration support. Very old graphics cards many not support 3d HW acceleration 
              and may not be compatible. 
      If your system has an incompatible graphics card then a new  low cost card such as ATI Radeon HD 6450 or Radeon HD 4670 or NVIDIA Geforce 210  can be purchased that 
      are compatible with Fedora 16. These cards have been tested at Xilinx. 
 
       
   
   
   
   
   
  
   


   




