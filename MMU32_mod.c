//Memory Mapping Utility Module (AARCH32)

#ifndef CONFIG_ARM
#error System is incompatible.
#endif

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <asm/io.h>

/*
 * MMU Command Structure (5 BYTES):
 *
 * BYTE0: CMD
 * BYTES 1-4 (1 UINT): 32BIT ADDR
 */

#define MMU_DATAIO_SIZE_BYTES 5

#define MMU_CMD_GET_PHYSICAL_ADDR 1
#define MMU_CMD_GET_VIRTUAL_ADDR 2

#define MMU_CMD_KERNEL_RESPONSE 0xFF

static struct proc_dir_entry *mmu_proc = NULL;
static void *mmu_data_io = NULL;

unsigned int mmu_get_physical_addr(unsigned int virtual_addr)
{
	return virt_to_phys((void*) virtual_addr);
}

unsigned int mmu_get_virtual_addr(unsigned int physical_addr)
{
	return (unsigned int) phys_to_virt(physical_addr);
}

ssize_t mmu_mod_usrread(struct file *file, char __user *user, size_t size, loff_t *offset)
{
	copy_to_user(user, mmu_data_io, MMU_DATAIO_SIZE_BYTES);
	return size;
}

ssize_t mmu_mod_usrwrite(struct file *file, const char __user *user, size_t size, loff_t *offset)
{
	copy_from_user(mmu_data_io, user, MMU_DATAIO_SIZE_BYTES);

	unsigned char *pbyte = (unsigned char*) mmu_data_io;
	unsigned int *puint = (unsigned int*) &pbyte[1];

	switch(pbyte[0])
	{
		case MMU_CMD_GET_PHYSICAL_ADDR:
			puint[0] = mmu_get_physical_addr(puint[0]);
			break;

		case MMU_CMD_GET_VIRTUAL_ADDR:
			puint[0] = mmu_get_virtual_addr(puint[0]);
			break;
	}

	pbyte[0] = MMU_CMD_KERNEL_RESPONSE;
	return size;
}

static const struct proc_ops mmu_proc_ops = {
	.proc_read = mmu_mod_usrread,
	.proc_write = mmu_mod_usrwrite
};

static int __init driver_enable(void)
{
	mmu_proc = proc_create("MMU32", 0x1B6, NULL, &mmu_proc_ops);
	if(mmu_proc == NULL)
	{
		printk("MMU: Error creating proc file\n");
		return -1;
	}

	mmu_data_io = vmalloc(MMU_DATAIO_SIZE_BYTES);
	printk("MMU Tool Enabled\n");
	return 0;
}

static void __exit driver_disable(void)
{
	proc_remove(mmu_proc);
	vfree(mmu_data_io);
	printk("MMU Tool Disabled\n");
	return;
}

module_init(driver_enable);
module_exit(driver_disable);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rafael Sabe");
MODULE_DESCRIPTION("Memory Mapping Utility Tool for GNU-Linux Systems.");
