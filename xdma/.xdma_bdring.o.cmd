cmd_/home/ron/work/mu2ePrj/PCIe_module/k7_pcie_dma_ddr3_base/linux_driver/xdma/xdma_bdring.o := gcc -Wp,-MD,/home/ron/work/mu2ePrj/PCIe_module/k7_pcie_dma_ddr3_base/linux_driver/xdma/.xdma_bdring.o.d  -nostdinc -isystem /usr/lib/gcc/x86_64-redhat-linux/4.4.6/include -Iinclude  -I/usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include -include include/linux/autoconf.h -D__KERNEL__ -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -Wno-format-security -fno-delete-null-pointer-checks -O2 -m64 -mtune=generic -mno-red-zone -mcmodel=kernel -funit-at-a-time -maccumulate-outgoing-args -fstack-protector -DCONFIG_AS_CFI=1 -DCONFIG_AS_CFI_SIGNAL_FRAME=1 -pipe -Wno-sign-compare -fno-asynchronous-unwind-tables -mno-sse -mno-mmx -mno-sse2 -mno-3dnow -Wframe-larger-than=2048 -Wno-unused-but-set-variable -fno-omit-frame-pointer -fno-optimize-sibling-calls -g -pg -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -fno-dwarf2-cfi-asm -fconserve-stack -DDEBUG_VERBOSE -DK7_TRD -DX86_PC -DNWLDMA -DXENV_LINUX -DUSE_IO_MACROS -DXLITTLE_ENDIAN -DPCI_LINUX -I /home/ron/work/mu2ePrj/PCIe_module/k7_pcie_dma_ddr3_base/linux_driver/xdma -I /home/ron/work/mu2ePrj/PCIe_module/k7_pcie_dma_ddr3_base/linux_driver/include  -DMODULE -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(xdma_bdring)"  -D"KBUILD_MODNAME=KBUILD_STR(xdma_k7)" -D"DEBUG_HASH=12" -D"DEBUG_HASH2=21" -c -o /home/ron/work/mu2ePrj/PCIe_module/k7_pcie_dma_ddr3_base/linux_driver/xdma/.tmp_xdma_bdring.o /home/ron/work/mu2ePrj/PCIe_module/k7_pcie_dma_ddr3_base/linux_driver/xdma/xdma_bdring.c

deps_/home/ron/work/mu2ePrj/PCIe_module/k7_pcie_dma_ddr3_base/linux_driver/xdma/xdma_bdring.o := \
  /home/ron/work/mu2ePrj/PCIe_module/k7_pcie_dma_ddr3_base/linux_driver/xdma/xdma_bdring.c \
  include/linux/kernel.h \
    $(wildcard include/config/lbdaf.h) \
    $(wildcard include/config/preempt/voluntary.h) \
    $(wildcard include/config/debug/spinlock/sleep.h) \
    $(wildcard include/config/prove/locking.h) \
    $(wildcard include/config/printk.h) \
    $(wildcard include/config/dynamic/debug.h) \
    $(wildcard include/config/ring/buffer.h) \
    $(wildcard include/config/tracing.h) \
    $(wildcard include/config/numa.h) \
    $(wildcard include/config/ftrace/mcount/record.h) \
  /usr/lib/gcc/x86_64-redhat-linux/4.4.6/include/stdarg.h \
  include/linux/linkage.h \
  include/linux/compiler.h \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/enable/must/check.h) \
    $(wildcard include/config/enable/warn/deprecated.h) \
  include/linux/compiler-gcc.h \
    $(wildcard include/config/arch/supports/optimized/inlining.h) \
    $(wildcard include/config/optimize/inlining.h) \
  include/linux/compiler-gcc4.h \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/linkage.h \
    $(wildcard include/config/x86/32.h) \
    $(wildcard include/config/x86/64.h) \
    $(wildcard include/config/x86/alignment/16.h) \
  include/linux/stringify.h \
  include/linux/stddef.h \
  include/linux/types.h \
    $(wildcard include/config/uid16.h) \
    $(wildcard include/config/phys/addr/t/64bit.h) \
    $(wildcard include/config/64bit.h) \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/types.h \
    $(wildcard include/config/highmem64g.h) \
  include/asm-generic/types.h \
  include/asm-generic/int-ll64.h \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/bitsperlong.h \
  include/asm-generic/bitsperlong.h \
  include/linux/posix_types.h \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/posix_types.h \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/posix_types_64.h \
  include/linux/bitops.h \
    $(wildcard include/config/generic/find/first/bit.h) \
    $(wildcard include/config/generic/find/last/bit.h) \
    $(wildcard include/config/generic/find/next/bit.h) \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/bitops.h \
    $(wildcard include/config/x86/cmov.h) \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/alternative.h \
    $(wildcard include/config/smp.h) \
    $(wildcard include/config/paravirt.h) \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/asm.h \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/cpufeature.h \
    $(wildcard include/config/x86/invlpg.h) \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/required-features.h \
    $(wildcard include/config/x86/minimum/cpu/family.h) \
    $(wildcard include/config/math/emulation.h) \
    $(wildcard include/config/x86/pae.h) \
    $(wildcard include/config/x86/cmpxchg64.h) \
    $(wildcard include/config/x86/use/3dnow.h) \
    $(wildcard include/config/x86/p6/nop.h) \
  include/asm-generic/bitops/sched.h \
  include/asm-generic/bitops/hweight.h \
  include/asm-generic/bitops/fls64.h \
  include/asm-generic/bitops/ext2-non-atomic.h \
  include/asm-generic/bitops/le.h \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/byteorder.h \
  include/linux/byteorder/little_endian.h \
  include/linux/swab.h \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/swab.h \
    $(wildcard include/config/x86/bswap.h) \
  include/linux/byteorder/generic.h \
  include/asm-generic/bitops/minix.h \
  include/linux/log2.h \
    $(wildcard include/config/arch/has/ilog2/u32.h) \
    $(wildcard include/config/arch/has/ilog2/u64.h) \
  include/linux/typecheck.h \
  include/linux/ratelimit.h \
  include/linux/param.h \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/param.h \
  include/asm-generic/param.h \
    $(wildcard include/config/hz.h) \
  include/linux/dynamic_debug.h \
  include/linux/init.h \
    $(wildcard include/config/modules.h) \
    $(wildcard include/config/hotplug.h) \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/bug.h \
    $(wildcard include/config/bug.h) \
    $(wildcard include/config/debug/bugverbose.h) \
  include/asm-generic/bug.h \
    $(wildcard include/config/generic/bug.h) \
    $(wildcard include/config/generic/bug/relative/pointers.h) \
  include/linux/string.h \
    $(wildcard include/config/binary/printf.h) \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/string.h \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/string_64.h \
    $(wildcard include/config/kmemcheck.h) \
  /home/ron/work/mu2ePrj/PCIe_module/k7_pcie_dma_ddr3_base/linux_driver/include/xbasic_types.h \
  /home/ron/work/mu2ePrj/PCIe_module/k7_pcie_dma_ddr3_base/linux_driver/include/xdebug.h \
  /home/ron/work/mu2ePrj/PCIe_module/k7_pcie_dma_ddr3_base/linux_driver/include/xstatus.h \
    $(wildcard include/config/error.h) \
  /home/ron/work/mu2ePrj/PCIe_module/k7_pcie_dma_ddr3_base/linux_driver/include/xbasic_types.h \
  /home/ron/work/mu2ePrj/PCIe_module/k7_pcie_dma_ddr3_base/linux_driver/include/xio.h \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/io.h \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/page.h \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/page_types.h \
  include/linux/const.h \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/page_64_types.h \
    $(wildcard include/config/physical/start.h) \
    $(wildcard include/config/physical/align.h) \
    $(wildcard include/config/flatmem.h) \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/page_64.h \
  include/asm-generic/memory_model.h \
    $(wildcard include/config/discontigmem.h) \
    $(wildcard include/config/sparsemem/vmemmap.h) \
    $(wildcard include/config/sparsemem.h) \
  include/asm-generic/getorder.h \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/io_64.h \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/paravirt.h \
    $(wildcard include/config/transparent/hugepage.h) \
    $(wildcard include/config/paravirt/spinlocks.h) \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/pgtable_types.h \
    $(wildcard include/config/compat/vdso.h) \
    $(wildcard include/config/proc/fs.h) \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/pgtable_64_types.h \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/paravirt_types.h \
    $(wildcard include/config/x86/local/apic.h) \
    $(wildcard include/config/paravirt/debug.h) \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/desc_defs.h \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/kmap_types.h \
    $(wildcard include/config/debug/highmem.h) \
  include/asm-generic/kmap_types.h \
  include/linux/cpumask.h \
    $(wildcard include/config/cpumask/offstack.h) \
    $(wildcard include/config/hotplug/cpu.h) \
    $(wildcard include/config/debug/per/cpu/maps.h) \
    $(wildcard include/config/disable/obsolete/cpumask/functions.h) \
  include/linux/threads.h \
    $(wildcard include/config/nr/cpus.h) \
    $(wildcard include/config/base/small.h) \
  include/linux/bitmap.h \
  include/linux/vmalloc.h \
    $(wildcard include/config/mmu.h) \
    $(wildcard include/config/have/legacy/per/cpu/area.h) \
  include/linux/spinlock.h \
    $(wildcard include/config/debug/spinlock.h) \
    $(wildcard include/config/generic/lockbreak.h) \
    $(wildcard include/config/preempt.h) \
    $(wildcard include/config/debug/lock/alloc.h) \
  include/linux/preempt.h \
    $(wildcard include/config/debug/preempt.h) \
    $(wildcard include/config/preempt/tracer.h) \
    $(wildcard include/config/preempt/notifiers.h) \
  include/linux/thread_info.h \
    $(wildcard include/config/compat.h) \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/thread_info.h \
    $(wildcard include/config/debug/stack/usage.h) \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/processor.h \
    $(wildcard include/config/x86/vsmp.h) \
    $(wildcard include/config/cc/stackprotector.h) \
    $(wildcard include/config/m386.h) \
    $(wildcard include/config/m486.h) \
    $(wildcard include/config/x86/debugctlmsr.h) \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/processor-flags.h \
    $(wildcard include/config/vm86.h) \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/vm86.h \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/ptrace.h \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/ptrace-abi.h \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/segment.h \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/cache.h \
    $(wildcard include/config/x86/l1/cache/shift.h) \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/math_emu.h \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/sigcontext.h \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/current.h \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/percpu.h \
    $(wildcard include/config/x86/64/smp.h) \
  include/asm-generic/percpu.h \
    $(wildcard include/config/have/setup/per/cpu/area.h) \
  include/linux/percpu-defs.h \
    $(wildcard include/config/debug/force/weak/per/cpu.h) \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/system.h \
    $(wildcard include/config/ia32/emulation.h) \
    $(wildcard include/config/x86/32/lazy/gs.h) \
    $(wildcard include/config/x86/ppro/fence.h) \
    $(wildcard include/config/x86/oostore.h) \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/cmpxchg.h \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/cmpxchg_64.h \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/nops.h \
    $(wildcard include/config/mk7.h) \
  include/linux/irqflags.h \
    $(wildcard include/config/trace/irqflags.h) \
    $(wildcard include/config/irqsoff/tracer.h) \
    $(wildcard include/config/trace/irqflags/support.h) \
    $(wildcard include/config/x86.h) \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/irqflags.h \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/msr.h \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/msr-index.h \
  include/linux/ioctl.h \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/ioctl.h \
  include/asm-generic/ioctl.h \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/errno.h \
  include/asm-generic/errno.h \
  include/asm-generic/errno-base.h \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/cpumask.h \
  include/linux/personality.h \
  include/linux/cache.h \
    $(wildcard include/config/arch/has/cache/line/size.h) \
  include/linux/math64.h \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/div64.h \
  include/asm-generic/div64.h \
  include/linux/err.h \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/ftrace.h \
    $(wildcard include/config/function/tracer.h) \
    $(wildcard include/config/dynamic/ftrace.h) \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/atomic.h \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/atomic_64.h \
  include/asm-generic/atomic-long.h \
  include/linux/list.h \
    $(wildcard include/config/debug/list.h) \
  include/linux/poison.h \
    $(wildcard include/config/illegal/pointer/value.h) \
  include/linux/prefetch.h \
  include/linux/bottom_half.h \
  include/linux/spinlock_types.h \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/spinlock_types.h \
  include/linux/lockdep.h \
    $(wildcard include/config/lockdep.h) \
    $(wildcard include/config/lock/stat.h) \
    $(wildcard include/config/generic/hardirqs.h) \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/spinlock.h \
  /usr/src/kernels/2.6.32-279.el6.x86_64/arch/x86/include/asm/rwlock.h \
  include/linux/spinlock_api_smp.h \
  include/asm-generic/iomap.h \
  /home/ron/work/mu2ePrj/PCIe_module/k7_pcie_dma_ddr3_base/linux_driver/xdma/xdma_bdring.h \
  /home/ron/work/mu2ePrj/PCIe_module/k7_pcie_dma_ddr3_base/linux_driver/xdma/xdma_bd.h \
  /home/ron/work/mu2ePrj/PCIe_module/k7_pcie_dma_ddr3_base/linux_driver/xdma/xdma_hw.h \

/home/ron/work/mu2ePrj/PCIe_module/k7_pcie_dma_ddr3_base/linux_driver/xdma/xdma_bdring.o: $(deps_/home/ron/work/mu2ePrj/PCIe_module/k7_pcie_dma_ddr3_base/linux_driver/xdma/xdma_bdring.o)

$(deps_/home/ron/work/mu2ePrj/PCIe_module/k7_pcie_dma_ddr3_base/linux_driver/xdma/xdma_bdring.o):
