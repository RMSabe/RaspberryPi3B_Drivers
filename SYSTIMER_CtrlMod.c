//BCM2837 System Timer Driver

#include "BCM2837_SYSTIMER_RegisterMapping.h"
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <asm/io.h>

/*
 * SYSTIMER Command Structure (6 BYTES):
 *
 * BYTE0: CMD
 * BYTE1: TIMER NUM
 * BYTES 2-5 (1 UINT): ARG
 */

#define SYSTIMER_DATAIO_SIZE_BYTES 6

#define SYSTIMER_CMD_GET_TIMER_MATCH_OCCURRED 0
#define SYSTIMER_CMD_GET_COUNTER_VALUE_L32 1
#define SYSTIMER_CMD_GET_COUNTER_VALUE_H32 2
#define SYSTIMER_CMD_SET_TIMER_MATCH_VALUE 3
#define SYSTIMER_CMD_GET_TIMER_MATCH_VALUE 4

#define SYSTIMER_CMD_KERNEL_RESPONSE 0xFF

static struct proc_dir_entry *systimer_proc = NULL;
static unsigned int *systimer_mapping = NULL;
static void *systimer_data_io = NULL;

unsigned int systimer_is_reg_bit_active(unsigned int register_value, unsigned int reference_bit)
{
	unsigned int bit_value = (register_value & reference_bit);
	if(bit_value == reference_bit) return 1;
	return 0;
}

unsigned int systimer_timer_match_occurred(unsigned int timer_num)
{
	timer_num &= 0x00000003;

	if(systimer_is_reg_bit_active(systimer_mapping[SYSTIMER_CTRL_STATUS_UINTP_POS], (1 << timer_num)))
	{
		systimer_mapping[SYSTIMER_CTRL_STATUS_UINTP_POS] |= (1 << timer_num);
		return 1;
	}

	return 0;
}

unsigned int systimer_get_counter_value_l32(void)
{
	return systimer_mapping[SYSTIMER_COUNTER_L32_UINTP_POS];
}

unsigned int systimer_get_counter_value_h32(void)
{
	return systimer_mapping[SYSTIMER_COUNTER_H32_UINTP_POS];
}

void systimer_set_timer_compare_match_value(unsigned int timer_num, unsigned int value)
{
	timer_num &= 0x00000003;
	unsigned int mapping_pos = 0;
	
	switch(timer_num)
	{
		case 0:
			mapping_pos = SYSTIMER_COMP0_UINTP_POS;
			break;

		case 1:
			mapping_pos = SYSTIMER_COMP1_UINTP_POS;
			break;

		case 2:
			mapping_pos = SYSTIMER_COMP2_UINTP_POS;
			break;

		case 3:
			mapping_pos = SYSTIMER_COMP3_UINTP_POS;
			break;
	}

	systimer_mapping[mapping_pos] = value;
	return;
}

unsigned int systimer_get_timer_compare_match_value(unsigned int timer_num)
{
	timer_num &= 0x00000003;
	unsigned int mapping_pos = 0;

	switch(timer_num)
	{
		case 0:
			mapping_pos = SYSTIMER_COMP0_UINTP_POS;
			break;

		case 1:
			mapping_pos = SYSTIMER_COMP1_UINTP_POS;
			break;

		case 2:
			mapping_pos = SYSTIMER_COMP2_UINTP_POS;
			break;

		case 3:
			mapping_pos = SYSTIMER_COMP3_UINTP_POS;
			break;
	}

	return systimer_mapping[mapping_pos];
}

ssize_t systimer_mod_usrread(struct file *file, char __user *user, size_t size, loff_t *offset)
{
	copy_to_user(user, systimer_data_io, SYSTIMER_DATAIO_SIZE_BYTES);
	return size;
}

ssize_t systimer_mod_usrwrite(struct file *file, const char __user *user, size_t size, loff_t *offset)
{
	copy_from_user(systimer_data_io, user, SYSTIMER_DATAIO_SIZE_BYTES);

	unsigned char *pbyte = (unsigned char*) systimer_data_io;
	unsigned int *puint = (unsigned int*) &pbyte[2];

	switch(pbyte[0])
	{
		case SYSTIMER_CMD_GET_TIMER_MATCH_OCCURRED:
			puint[0] = systimer_timer_match_occurred(pbyte[1]);
			break;

		case SYSTIMER_CMD_GET_COUNTER_VALUE_L32:
			puint[0] = systimer_get_counter_value_l32();
			break;

		case SYSTIMER_CMD_GET_COUNTER_VALUE_H32:
			puint[0] = systimer_get_counter_value_h32();
			break;

		case SYSTIMER_CMD_SET_TIMER_MATCH_VALUE:
			systimer_set_timer_compare_match_value(pbyte[1], puint[0]);
			break;

		case SYSTIMER_CMD_GET_TIMER_MATCH_VALUE:
			puint[0] = systimer_get_timer_compare_match_value(pbyte[1]);
			break;
	}

	pbyte[0] = SYSTIMER_CMD_KERNEL_RESPONSE;
	return size;
}

static const struct proc_ops systimer_proc_ops = {
	.proc_read = systimer_mod_usrread,
	.proc_write = systimer_mod_usrwrite
};

static int __init driver_enable(void)
{
	systimer_mapping = (unsigned int*) ioremap(SYSTIMER_BASE_ADDR, SYSTIMER_MAPPING_SIZE_BYTES);
	if(systimer_mapping == NULL)
	{
		printk("SYSTIMER: Error mapping SYSTIMER addr\n");
		return -1;
	}

	systimer_proc = proc_create("SYSTIMER_Ctrl", 0x1B6, NULL, &systimer_proc_ops);
	if(systimer_proc == NULL)
	{
		printk("SYSTIMER: Error creating proc file\n");
		return -1;
	}

	systimer_data_io = vmalloc(SYSTIMER_DATAIO_SIZE_BYTES);
	printk("SYSTIMER Control Driver Enabled\n");
	return 0;
}

static void __exit driver_disable(void)
{
	iounmap(systimer_mapping);
	proc_remove(systimer_proc);
	vfree(systimer_data_io);
	printk("SYSTIMER Control Driver Disabled\n");
	return;
}

module_init(driver_enable);
module_exit(driver_disable);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rafael Sabe");
MODULE_DESCRIPTION("Driver for BCM2837 System Timer Control");
