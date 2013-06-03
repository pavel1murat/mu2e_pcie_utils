#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x14522340, "module_layout" },
	{ 0xa0a833c5, "cdev_alloc" },
	{ 0x42e80c19, "cdev_del" },
	{ 0xc917223d, "pci_bus_read_config_byte" },
	{ 0xd2037915, "dev_set_drvdata" },
	{ 0xfa2e111f, "slab_buffer_size" },
	{ 0xd691cba2, "malloc_sizes" },
	{ 0xa30682, "pci_disable_device" },
	{ 0x973873ab, "_spin_lock" },
	{ 0x105e2727, "__tracepoint_kmalloc" },
	{ 0xd3364703, "x86_dma_fallback_dev" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0x102b9c3, "pci_release_regions" },
	{ 0x6a9f26c9, "init_timer_key" },
	{ 0x7485e15e, "unregister_chrdev_region" },
	{ 0x712aa29b, "_spin_lock_irqsave" },
	{ 0x7d11c268, "jiffies" },
	{ 0x9629486a, "per_cpu__cpu_number" },
	{ 0xaf559063, "pci_set_master" },
	{ 0xe83fea1, "del_timer_sync" },
	{ 0xde0bdcff, "memset" },
	{ 0x9f1019bd, "pci_set_dma_mask" },
	{ 0xf85ccdae, "kmem_cache_alloc_notrace" },
	{ 0xea147363, "printk" },
	{ 0x85f8a266, "copy_to_user" },
	{ 0xb4390f9a, "mcount" },
	{ 0x16305289, "warn_slowpath_null" },
	{ 0x6dcaeb88, "per_cpu__kernel_stack" },
	{ 0x4b07e779, "_spin_unlock_irqrestore" },
	{ 0x46085e4f, "add_timer" },
	{ 0x520ee4c8, "pci_find_capability" },
	{ 0xa6d1bdca, "cdev_add" },
	{ 0x78764f4e, "pv_irq_ops" },
	{ 0x42c8de35, "ioremap_nocache" },
	{ 0x84b453e6, "pci_bus_read_config_word" },
	{ 0xc5aa6d66, "pci_bus_read_config_dword" },
	{ 0x68f7c535, "pci_unregister_driver" },
	{ 0xe52947e7, "__phys_addr" },
	{ 0x3aa1dbcf, "_spin_unlock_bh" },
	{ 0x37a0cba, "kfree" },
	{ 0x6d090f30, "pci_request_regions" },
	{ 0xedc03953, "iounmap" },
	{ 0x5f07b9f3, "__pci_register_driver" },
	{ 0x93cbd1ec, "_spin_lock_bh" },
	{ 0xa12add91, "pci_enable_device" },
	{ 0x3302b500, "copy_from_user" },
	{ 0xa92a43c, "dev_get_drvdata" },
	{ 0x6e9681d2, "dma_ops" },
	{ 0x29537c9e, "alloc_chrdev_region" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("pci:v000010EEd00007042sv*sd*bc*sc*i*");

MODULE_INFO(srcversion, "16CB0FFDA468C3C0C1B6C1A");

static const struct rheldata _rheldata __used
__attribute__((section(".rheldata"))) = {
	.rhel_major = 6,
	.rhel_minor = 3,
};
