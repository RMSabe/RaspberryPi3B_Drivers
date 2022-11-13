//BCM2837 GPCLK Driver

#include "BCM2837_GPCLK_RegisterMapping.h"
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <asm/io.h>

#define GPCLK_KEY 0x5A

#define GPCLK0 0
#define GPCLK1 1
#define GPCLK2 2

#define GPCLK_CLKSRC_NULL 0
#define GPCLK_CLKSRC_OSC 1
#define GPCLK_CLKSRC_TESTDEBUG0 2
#define GPCLK_CLKSRC_TESTDEBUG1 3
#define GPCLK_CLKSRC_PLLA 4
#define GPCLK_CLKSRC_PLLC 5
#define GPCLK_CLKSRC_PLLD 6
#define GPCLK_CLKSRC_HDMIAUX 7

/*
 * GPCLK Command Structure (4 BYTES):
 * BYTE0: CMD
 * BYTE1: GPCLK
 * BYTES 2-3 (1 USHORT): ARG
 */

#define GPCLK_DATAIO_SIZE_BYTES 4

#define GPCLK_CMD_SET_ENABLE 0
#define GPCLK_CMD_GET_ENABLE 1
#define GPCLK_CMD_SET_MASH_LEVEL 2
#define GPCLK_CMD_GET_MASH_LEVEL 3
#define GPCLK_CMD_SET_INVERT_OUTPUT 4
#define GPCLK_CMD_GET_INVERT_OUTPUT 5
#define GPCLK_CMD_SET_CLKSRC 6
#define GPCLK_CMD_GET_CLKSRC 7
#define GPCLK_CMD_SET_IDIVIDER 8
#define GPCLK_CMD_GET_IDIVIDER 9
#define GPCLK_CMD_SET_FDIVIDER 10
#define GPCLK_CMD_GET_FDIVIDER 11
#define GPCLK_CMD_GET_IS_BUSY 12
#define GPCLK_CMD_SET_KILL_BIT 13
#define GPCLK_CMD_GET_KILL_BIT 14

#define GPCLK_CMD_KERNEL_RESPONSE 0xFF

static struct proc_dir_entry *gpclk_proc = NULL;
static unsigned int *gpclk_mapping = NULL;
static void *gpclk_data_io = NULL;

unsigned int gpclk_is_reg_bit_active(unsigned int register_value, unsigned int reference_bit)
{
	unsigned int bit_value = (register_value & reference_bit);
	if(bit_value == reference_bit) return 1;
	return 0;
}

void gpclk_get_mapping_pos(unsigned int gpclk, unsigned int clkdiv, unsigned int *p_mappingpos)
{
	if(p_mappingpos == NULL) return;

	clkdiv &= 0x00000001;
	switch(gpclk)
	{
		case GPCLK0:
			if(clkdiv) *p_mappingpos = GPCLK0_DIV_UINTP_POS;
			else *p_mappingpos = GPCLK0_CTRL_UINTP_POS;
			break;

		case GPCLK1:
			if(clkdiv) *p_mappingpos = GPCLK1_DIV_UINTP_POS;
			else *p_mappingpos = GPCLK1_CTRL_UINTP_POS;
			break;

		case GPCLK2:
			if(clkdiv) *p_mappingpos = GPCLK2_DIV_UINTP_POS;
			else *p_mappingpos = GPCLK2_CTRL_UINTP_POS;
			break;
	}

	return;
}

void gpclk_set_mash_level(unsigned int gpclk, unsigned int mash_level)
{
	mash_level &= 0x00000003;
	unsigned int mapping_pos = 0;
	gpclk_get_mapping_pos(gpclk, 0, &mapping_pos);

	gpclk_mapping[mapping_pos] = ((gpclk_mapping[mapping_pos] | (GPCLK_KEY << 24)) & ~(0x3 << 9));
	gpclk_mapping[mapping_pos] = (gpclk_mapping[mapping_pos] | (GPCLK_KEY << 24) | (mash_level << 9));
	return;
}

unsigned int gpclk_get_mash_level(unsigned int gpclk)
{
	unsigned int mash_level = 0;
	unsigned int mapping_pos = 0;
	gpclk_get_mapping_pos(gpclk, 0, &mapping_pos);

	mash_level = gpclk_mapping[mapping_pos];
	mash_level &= 0x00000600;
	return (mash_level >> 9);
}

void gpclk_invert_output(unsigned int gpclk, unsigned int invert)
{
	invert &= 0x00000001;
	unsigned int mapping_pos = 0;
	gpclk_get_mapping_pos(gpclk, 0, &mapping_pos);

	gpclk_mapping[mapping_pos] = ((gpclk_mapping[mapping_pos] | (GPCLK_KEY << 24)) & ~(1 << 8));
	gpclk_mapping[mapping_pos] = (gpclk_mapping[mapping_pos] | (GPCLK_KEY << 24) | (invert << 8));
	return;
}

unsigned int gpclk_output_is_inverted(unsigned int gpclk)
{
	unsigned int mapping_pos = 0;
	gpclk_get_mapping_pos(gpclk, 0, &mapping_pos);

	return gpclk_is_reg_bit_active(gpclk_mapping[mapping_pos], (1 << 8));
}

unsigned int gpclk_is_busy(unsigned int gpclk)
{
	unsigned int mapping_pos = 0;
	gpclk_get_mapping_pos(gpclk, 0, &mapping_pos);

	return gpclk_is_reg_bit_active(gpclk_mapping[mapping_pos], (1 << 7));
}

void gpclk_set_kill_bit(unsigned int gpclk, unsigned int bit_value)
{
	bit_value &= 0x00000001;
	unsigned int mapping_pos = 0;
	gpclk_get_mapping_pos(gpclk, 0, &mapping_pos);

	gpclk_mapping[mapping_pos] = ((gpclk_mapping[mapping_pos] | (GPCLK_KEY << 24)) & ~(1 << 5));
	gpclk_mapping[mapping_pos] = (gpclk_mapping[mapping_pos] | (GPCLK_KEY << 24) | (bit_value << 5));
	return;
}

unsigned int gpclk_get_kill_bit(unsigned int gpclk)
{
	unsigned int mapping_pos = 0;
	gpclk_get_mapping_pos(gpclk, 0, &mapping_pos);

	return gpclk_is_reg_bit_active(gpclk_mapping[mapping_pos], (1 << 5));
}

void gpclk_enable(unsigned int gpclk, unsigned int enable)
{
	enable &= 0x00000001;
	unsigned int mapping_pos = 0;
	gpclk_get_mapping_pos(gpclk, 0, &mapping_pos);

	gpclk_mapping[mapping_pos] = ((gpclk_mapping[mapping_pos] | (GPCLK_KEY << 24)) & ~(1 << 4));
	gpclk_mapping[mapping_pos] = (gpclk_mapping[mapping_pos] | (GPCLK_KEY << 24) | (enable << 4));
	return;
}

unsigned int gpclk_is_enabled(unsigned int gpclk)
{
	unsigned int mapping_pos = 0;
	gpclk_get_mapping_pos(gpclk, 0, &mapping_pos);

	return gpclk_is_reg_bit_active(gpclk_mapping[mapping_pos], (1 << 4));
}

void gpclk_set_clk_src(unsigned int gpclk, unsigned int clksrc)
{
	clksrc &= 0x00000007;
	unsigned int mapping_pos = 0;
	gpclk_get_mapping_pos(gpclk, 0, &mapping_pos);

	gpclk_mapping[mapping_pos] = ((gpclk_mapping[mapping_pos] | (GPCLK_KEY << 24)) & ~(0xF));
	gpclk_mapping[mapping_pos] = (gpclk_mapping[mapping_pos] | (GPCLK_KEY << 24) | (clksrc));
	return;
}

unsigned int gpclk_get_clk_src(unsigned int gpclk)
{
	unsigned int clksrc = 0;
	unsigned int mapping_pos = 0;
	gpclk_get_mapping_pos(gpclk, 0, &mapping_pos);

	clksrc = gpclk_mapping[mapping_pos];
	clksrc &= 0x00000007;
	return clksrc;
}

void gpclk_set_integer_divider(unsigned int gpclk, unsigned int divider)
{
	divider &= 0x00000FFF;
	unsigned int mapping_pos = 0;
	gpclk_get_mapping_pos(gpclk, 1, &mapping_pos);

	gpclk_mapping[mapping_pos] = ((gpclk_mapping[mapping_pos] | (GPCLK_KEY << 24)) & ~(0xFFF << 12));
	gpclk_mapping[mapping_pos] = (gpclk_mapping[mapping_pos] | (GPCLK_KEY << 24) | (divider << 12));
	return;
}

unsigned int gpclk_get_integer_divider(unsigned int gpclk)
{
	unsigned int divider = 0;
	unsigned int mapping_pos = 0;
	gpclk_get_mapping_pos(gpclk, 1, &mapping_pos);

	divider = gpclk_mapping[mapping_pos];
	divider &= 0x00FFF000;
	return (divider >> 12);
}

void gpclk_set_fractional_divider(unsigned int gpclk, unsigned int divider)
{
	divider &= 0x00000FFF;
	unsigned int mapping_pos = 0;
	gpclk_get_mapping_pos(gpclk, 1, &mapping_pos);

	gpclk_mapping[mapping_pos] = ((gpclk_mapping[mapping_pos] | (GPCLK_KEY << 24)) & ~(0xFFF));
	gpclk_mapping[mapping_pos] = (gpclk_mapping[mapping_pos] | (GPCLK_KEY << 24) | (divider));
	return;
}

unsigned int gpclk_get_fractional_divider(unsigned int gpclk)
{
	unsigned int divider = 0;
	unsigned int mapping_pos = 0;
	gpclk_get_mapping_pos(gpclk, 1, &mapping_pos);

	divider = gpclk_mapping[mapping_pos];
	divider &= 0x00000FFF;
	return divider;
}

ssize_t gpclk_mod_usrread(struct file *file, char __user *user, size_t size, loff_t *offset)
{
	copy_to_user(user, gpclk_data_io, GPCLK_DATAIO_SIZE_BYTES);
	return size;
}

ssize_t gpclk_mod_usrwrite(struct file *file, const char __user *user, size_t size, loff_t *offset)
{
	copy_from_user(gpclk_data_io, user, GPCLK_DATAIO_SIZE_BYTES);

	unsigned char *pbyte = (unsigned char*) gpclk_data_io;
	unsigned short *pushort = (unsigned short*) &pbyte[2];

	switch(pbyte[0])
	{
		case GPCLK_CMD_SET_ENABLE:
			gpclk_enable(pbyte[1], pushort[0]);
			break;

		case GPCLK_CMD_GET_ENABLE:
			pushort[0] = gpclk_is_enabled(pbyte[1]);
			break;

		case GPCLK_CMD_SET_MASH_LEVEL:
			gpclk_set_mash_level(pbyte[1], pushort[0]);
			break;

		case GPCLK_CMD_GET_MASH_LEVEL:
			pushort[0] = gpclk_get_mash_level(pbyte[1]);
			break;

		case GPCLK_CMD_SET_INVERT_OUTPUT:
			gpclk_invert_output(pbyte[1], pushort[0]);
			break;

		case GPCLK_CMD_GET_INVERT_OUTPUT:
			pushort[0] = gpclk_output_is_inverted(pbyte[1]);
			break;

		case GPCLK_CMD_SET_CLKSRC:
			gpclk_set_clk_src(pbyte[1], pushort[0]);
			break;

		case GPCLK_CMD_GET_CLKSRC:
			pushort[0] = gpclk_get_clk_src(pbyte[1]);
			break;

		case GPCLK_CMD_SET_IDIVIDER:
			gpclk_set_integer_divider(pbyte[1], pushort[0]);
			break;

		case GPCLK_CMD_GET_IDIVIDER:
			pushort[0] = gpclk_get_integer_divider(pbyte[1]);
			break;

		case GPCLK_CMD_SET_FDIVIDER:
			gpclk_set_fractional_divider(pbyte[1], pushort[0]);
			break;

		case GPCLK_CMD_GET_FDIVIDER:
			pushort[0] = gpclk_get_fractional_divider(pbyte[1]);
			break;

		case GPCLK_CMD_GET_IS_BUSY:
			pushort[0] = gpclk_is_busy(pbyte[1]);
			break;

		case GPCLK_CMD_SET_KILL_BIT:
			gpclk_set_kill_bit(pbyte[1], pushort[0]);
			break;

		case GPCLK_CMD_GET_KILL_BIT:
			pushort[0] = gpclk_get_kill_bit(pbyte[1]);
			break;
	}

	pbyte[0] = GPCLK_CMD_KERNEL_RESPONSE;
	return size;
}

static const struct proc_ops gpclk_proc_ops = {
	.proc_read = gpclk_mod_usrread,
	.proc_write = gpclk_mod_usrwrite
};

static int __init driver_enable(void)
{
	gpclk_mapping = (unsigned int*) ioremap(GPCLK_BASE_ADDR, GPCLK_MAPPING_SIZE_BYTES);
	if(gpclk_mapping == NULL)
	{
		printk("GPCLK: Error mapping GPCLK addr\n");
		return -1;
	}

	gpclk_proc = proc_create("GPCLK_Ctrl", 0x1B6, NULL, &gpclk_proc_ops);
	if(gpclk_proc == NULL)
	{
		printk("GPCLK: Error creating proc file\n");
		return -1;
	}

	gpclk_data_io = vmalloc(GPCLK_DATAIO_SIZE_BYTES);
	printk("GPCLK Control Driver Enabled\n");
	return 0;
}

static void __exit driver_disable(void)
{
	iounmap(gpclk_mapping);
	proc_remove(gpclk_proc);
	vfree(gpclk_data_io);
	printk("GPCLK Control Driver Disabled\n");
	return;
}

module_init(driver_enable);
module_exit(driver_disable);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rafael Sabe");
MODULE_DESCRIPTION("Driver for BCM2837 GPCLK Control");
