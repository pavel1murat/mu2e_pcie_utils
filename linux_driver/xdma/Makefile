obj-m := xdma_k7.o
xdma_k7-objs :=  xdma_base.o xdma.o xdma_bdring.o xdma_user.o

KDIR= /lib/modules/$(shell uname -r)/build

EXTRA_CFLAGS += -DK7_TRD -DX86_PC -DNWLDMA -DXENV_LINUX -DUSE_IO_MACROS
EXTRA_CFLAGS += -DXLITTLE_ENDIAN -DPCI_LINUX
# Note: ROOTDIR is defined when kbuild reads this Makefile (from $(MAKE) below)
EXTRA_CFLAGS += -I $(ROOTDIR)/xdma -I $(ROOTDIR)/include 

# Add the following to EXTRA_CFLAGS to enable interrupt mode
#EXTRA_CFLAGS += -DTH_BH_ISR

all:
	@echo "xdma MAKEFLAGS=$(MAKEFLAGS) CFLAGS=$(CFLAGS)"
	@echo "\$$(PWD)=$(PWD) \$$\$$PWD=$$PWD"
	$(MAKE) -C $(KDIR) M=$$PWD ROOTDIR=`dirname $$PWD`

clean: 
	   rm -rf *.o *.ko *.mod.* .c* .t* .*.mod.* .*.cmd *.ko.unsigned Module.symvers modules.order

# LOGGING: ---------------------------------------------------------------
# Add to EXTRA_CFLAGS, based on the logging level desired:
# -DDEBUG_NORMAL: To enable normal logging levels 
# -DDEBUG_VERBOSE: To enable verbose logging levels 
