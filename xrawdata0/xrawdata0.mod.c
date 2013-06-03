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
	{ 0xf9a482f9, "msleep" },
	{ 0x6a9f26c9, "init_timer_key" },
	{ 0x7d11c268, "jiffies" },
	{ 0xe83fea1, "del_timer_sync" },
	{ 0xea147363, "printk" },
	{ 0xb4390f9a, "mcount" },
	{ 0x46085e4f, "add_timer" },
	{ 0x93fca811, "__get_free_pages" },
	{ 0x4302d0eb, "free_pages" },
	{ 0x3aa1dbcf, "_spin_unlock_bh" },
	{ 0x7ae627e6, "DmaRegister" },
	{ 0x93cbd1ec, "_spin_lock_bh" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=xdma_k7";


MODULE_INFO(srcversion, "057158CF6D7B972AAF2F824");

static const struct rheldata _rheldata __used
__attribute__((section(".rheldata"))) = {
	.rhel_major = 6,
	.rhel_minor = 3,
};
