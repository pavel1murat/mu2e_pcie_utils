#
#	Makefile for Xilinx Kintex-7 PCIe-DMA-DDR3 Base Targeted Reference Design
#

MKNOD=`awk '/xdma_stat/ {print $$1}' /proc/devices`

all::
	echo "MAKEFLAGS=$(MAKEFLAGS)"
	$(MAKE) -C trace
	$(MAKE) -C xdma
	$(MAKE) -C xrawdata0 KBUILD_EXTRA_SYMBOLS="$$PWD/xdma/Module.symvers $$PWD/trace/Module.symvers"
	$(MAKE) -C xrawdata1 KBUILD_EXTRA_SYMBOLS="$$PWD/xdma/Module.symvers $$PWD/trace/Module.symvers"
	@echo "***** Driver Compiled *****"

.PHONY: trace mymodule user_space
trace mymodule user_space:
	$(MAKE) -C $@

clean::
	@for dd in */.;do test $dd = include/. || $(MAKE) -C $$dd clean;done

insert:: xdma/xdma_k7.ko xrawdata0/xrawdata0.ko xrawdata1/xrawdata1.ko trace/TRACE.ko
		/sbin/insmod trace/Trace.ko
		/sbin/insmod xdma/xdma_k7.ko; sleep 1
		/bin/mknod /dev/xdma_stat c ${MKNOD} 0
		/sbin/insmod xrawdata0/xrawdata0.ko; sleep 1
		/sbin/insmod xrawdata1/xrawdata1.ko
		@echo "***** Driver Loaded *****"

remove::
		/sbin/rmmod xrawdata1.ko; sleep 1
		/sbin/rmmod xrawdata0.ko; sleep 1; rm -f /dev/xdma_stat
		/sbin/rmmod xdma_k7.ko
		@echo "***** Driver Unloaded *****"

