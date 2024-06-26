#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

#ifdef CONFIG_UNWINDER_ORC
#include <asm/orc_header.h>
ORC_HEADER;
#endif

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x5d626f20, "kthread_create_on_node" },
	{ 0xbca77c8c, "wake_up_process" },
	{ 0x122c3a7e, "_printk" },
	{ 0x29abdefa, "kfree_skb_reason" },
	{ 0x6006375e, "__register_chrdev" },
	{ 0x1399bb1, "class_create" },
	{ 0xd3044a78, "device_create" },
	{ 0xa6f7a612, "cdev_init" },
	{ 0xf4407d6b, "cdev_add" },
	{ 0xc3920a5a, "init_net" },
	{ 0x7c9dac2e, "__netlink_kernel_create" },
	{ 0x80efbd79, "proc_create" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0x92ce99, "class_destroy" },
	{ 0xf7be671b, "device_destroy" },
	{ 0xa1f25452, "netlink_kernel_release" },
	{ 0x8f44466e, "cdev_del" },
	{ 0xce168946, "proc_remove" },
	{ 0xb6a5193f, "pcpu_hot" },
	{ 0x656e4a6e, "snprintf" },
	{ 0xa916b694, "strnlen" },
	{ 0xf1f012c6, "__alloc_skb" },
	{ 0x2aa4c83d, "__nlmsg_put" },
	{ 0x9166fada, "strncpy" },
	{ 0x2756c0b8, "netlink_unicast" },
	{ 0xcbd4898c, "fortify_panic" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x3c3ff9fd, "sprintf" },
	{ 0x619cb7dd, "simple_read_from_buffer" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0x2fa5cadd, "module_layout" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "884661A1AF1AD6B9755EC35");
