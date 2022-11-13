//BCM2837 ARM Timer Control Driver

#include "BCM2837_ARMTIMER_RegisterMapping.h"
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <asm/io.h>

#define ARMTIMER_TIMER_PRESCALE_NONE 0
#define ARMTIMER_TIMER_PRESCALE_CLKDIV16 1
#define ARMTIMER_TIMER_PRESCALE_CLKDIV256 2

#define ARMTIMER_COUNTSIZE_16BITS 0
#define ARMTIMER_COUNTSIZE_32BITS 1

/*
 * ARMTIMER Command Structure (5 BYTES):
 * BYTE0: CMD
 * BYTES 1-4 (1 UINT): ARG
 */

#define ARMTIMER_DATAIO_SIZE_BYTES 5

#define ARMTIMER_CMD_SET_LOAD_VALUE 0
#define ARMTIMER_CMD_GET_LOAD_VALUE 1
#define ARMTIMER_CMD_GET_COUNTDOWN_VALUE 2
#define ARMTIMER_CMD_SET_FREERUN_COUNTER_PRESCALE 3
#define ARMTIMER_CMD_GET_FREERUN_COUNTER_PRESCALE 4
#define ARMTIMER_CMD_SET_ENABLE_FREERUN_COUNTER 5
#define ARMTIMER_CMD_GET_ENABLE_FREERUN_COUNTER 6
#define ARMTIMER_CMD_SET_ENABLE_DEBUG_HALT_TIMER 7
#define ARMTIMER_CMD_GET_ENABLE_DEBUG_HALT_TIMER 8
#define ARMTIMER_CMD_SET_ENABLE_TIMER 9
#define ARMTIMER_CMD_GET_ENABLE_TIMER 10
#define ARMTIMER_CMD_SET_ENABLE_INTR 11
#define ARMTIMER_CMD_GET_ENABLE_INTR 12
#define ARMTIMER_CMD_SET_TIMER_PRESCALE 13
#define ARMTIMER_CMD_GET_TIMER_PRESCALE 14
#define ARMTIMER_CMD_SET_COUNTSIZE_BITS 15
#define ARMTIMER_CMD_GET_COUNTSIZE_BITS 16
#define ARMTIMER_CMD_CLEAR_INTR_FLAGS 17
#define ARMTIMER_CMD_GET_RAW_INTR_STATUS 18
#define ARMTIMER_CMD_GET_MASKED_INTR_STATUS 19
#define ARMTIMER_CMD_SET_RELOAD_VALUE 20
#define ARMTIMER_CMD_GET_RELOAD_VALUE 21
#define ARMTIMER_CMD_SET_PREDIV_VALUE 22
#define ARMTIMER_CMD_GET_PREDIV_VALUE 23
#define ARMTIMER_CMD_GET_FREERUN_COUNTER_VALUE 24

#define ARMTIMER_CMD_KERNEL_RESPONSE 0xFF

static struct proc_dir_entry *armtimer_proc = NULL;
static unsigned int *armtimer_mapping = NULL;
static void *armtimer_data_io = NULL;

unsigned int armtimer_is_reg_bit_active(unsigned int register_value, unsigned int reference_bit)
{
	unsigned int bit_value = (register_value & reference_bit);
	if(bit_value == reference_bit) return 1;
	return 0;
}

void armtimer_set_load_value(unsigned int value)
{
	armtimer_mapping[ARMTIMER_LOAD_UINTP_POS] = value;
	return;
}

unsigned int armtimer_get_load_value(void)
{
	return armtimer_mapping[ARMTIMER_LOAD_UINTP_POS];
}

unsigned int armtimer_get_countdown_value(void)
{
	return armtimer_mapping[ARMTIMER_VALUE_UINTP_POS];
}

void armtimer_set_freerun_counter_prescale(unsigned int prescale)
{
	prescale &= 0x000000FF;
	armtimer_mapping[ARMTIMER_CTRL_UINTP_POS] &= ~(0xFF << 16);
	armtimer_mapping[ARMTIMER_CTRL_UINTP_POS] |= (prescale << 16);
	return;
}

unsigned int armtimer_get_freerun_counter_prescale(void)
{
	unsigned int prescale = armtimer_mapping[ARMTIMER_CTRL_UINTP_POS];
	prescale &= (0xFF << 16);
	return (prescale >> 16);
}

void armtimer_enable_freerun_counter(unsigned int enable)
{
	enable &= 0x00000001;
	armtimer_mapping[ARMTIMER_CTRL_UINTP_POS] &= ~(1 << 9);
	armtimer_mapping[ARMTIMER_CTRL_UINTP_POS] |= (enable << 9);
	return;
}

unsigned int armtimer_freerun_counter_is_enabled(void)
{
	return armtimer_is_reg_bit_active(armtimer_mapping[ARMTIMER_CTRL_UINTP_POS], (1 << 9));
}

void armtimer_enable_debug_halt_timer(unsigned int enable)
{
	enable &= 0x00000001;
	armtimer_mapping[ARMTIMER_CTRL_UINTP_POS] &= ~(1 << 8);
	armtimer_mapping[ARMTIMER_CTRL_UINTP_POS] |= (enable << 8);
	return;
}

unsigned int armtimer_debug_halt_timer_is_enabled(void)
{
	return armtimer_is_reg_bit_active(armtimer_mapping[ARMTIMER_CTRL_UINTP_POS], (1 << 8));
}

void armtimer_enable_timer(unsigned int enable)
{
	enable &= 0x00000001;
	armtimer_mapping[ARMTIMER_CTRL_UINTP_POS] &= ~(1 << 7);
	armtimer_mapping[ARMTIMER_CTRL_UINTP_POS] |= (enable << 7);
	return;
}

unsigned int armtimer_timer_is_enabled(void)
{
	return armtimer_is_reg_bit_active(armtimer_mapping[ARMTIMER_CTRL_UINTP_POS], (1 << 7));
}

void armtimer_enable_intr(unsigned int enable)
{
	enable &= 0x00000001;
	armtimer_mapping[ARMTIMER_CTRL_UINTP_POS] &= ~(1 << 5);
	armtimer_mapping[ARMTIMER_CTRL_UINTP_POS] |= (enable << 5);
	return;
}

unsigned int armtimer_intr_is_enabled(void)
{
	return armtimer_is_reg_bit_active(armtimer_mapping[ARMTIMER_CTRL_UINTP_POS], (1 << 5));
}

void armtimer_set_timer_prescale(unsigned int prescale)
{
	prescale &= 0x00000003;
	armtimer_mapping[ARMTIMER_CTRL_UINTP_POS] &= ~(0x3 << 2);
	armtimer_mapping[ARMTIMER_CTRL_UINTP_POS] |= (prescale << 2);
	return;
}

unsigned int armtimer_get_timer_prescale(void)
{
	unsigned int prescale = armtimer_mapping[ARMTIMER_CTRL_UINTP_POS];
	prescale &= (0x3 << 2);
	return (prescale >> 2);
}

void armtimer_set_count_size_bits(unsigned int countsize)
{
	countsize &= 0x00000001;
	armtimer_mapping[ARMTIMER_CTRL_UINTP_POS] &= ~(1 << 1);
	armtimer_mapping[ARMTIMER_CTRL_UINTP_POS] |= (countsize << 1);
	return;
}

unsigned int armtimer_get_count_size_bits(void)
{
	return armtimer_is_reg_bit_active(armtimer_mapping[ARMTIMER_CTRL_UINTP_POS], (1 << 1));
}

void armtimer_clear_intr_flags(void)
{
	armtimer_mapping[ARMTIMER_INTR_CLEAR_UINTP_POS] = (1);
	return;
}

unsigned int armtimer_get_raw_intr_status(void)
{
	return armtimer_is_reg_bit_active(armtimer_mapping[ARMTIMER_RAW_INTR_UINTP_POS], (1));
}

unsigned int armtimer_get_masked_intr_status(void)
{
	return armtimer_is_reg_bit_active(armtimer_mapping[ARMTIMER_MASKED_INTR_UINTP_POS], (1));
}

void armtimer_set_reload_value(unsigned int value)
{
	armtimer_mapping[ARMTIMER_RELOAD_UINTP_POS] = value;
	return;
}

unsigned int armtimer_get_reload_value(void)
{
	return armtimer_mapping[ARMTIMER_RELOAD_UINTP_POS];
}

void armtimer_set_predivider_value(unsigned int value)
{
	value &= 0x000003FF;
	armtimer_mapping[ARMTIMER_PREDIV_UINTP_POS] = value;
	return;
}

unsigned int armtimer_get_predivider_value(void)
{
	unsigned int value = armtimer_mapping[ARMTIMER_PREDIV_UINTP_POS];
	value &= (0x3FF);
	return value;
}

unsigned int armtimer_get_freerun_counter_value(void)
{
	return armtimer_mapping[ARMTIMER_COUNTER_UINTP_POS];
}

ssize_t armtimer_mod_usrread(struct file *file, char __user *user, size_t size, loff_t *offset)
{
	copy_to_user(user, armtimer_data_io, ARMTIMER_DATAIO_SIZE_BYTES);
	return size;
}

ssize_t armtimer_mod_usrwrite(struct file *file, const char __user *user, size_t size, loff_t *offset)
{
	copy_from_user(armtimer_data_io, user, ARMTIMER_DATAIO_SIZE_BYTES);

	unsigned char *pbyte = (unsigned char*) armtimer_data_io;
	unsigned int *puint = (unsigned int*) &pbyte[1];

	switch(pbyte[0])
	{
		case ARMTIMER_CMD_SET_LOAD_VALUE:
			armtimer_set_load_value(puint[0]);
			break;

		case ARMTIMER_CMD_GET_LOAD_VALUE:
			puint[0] = armtimer_get_load_value();
			break;

		case ARMTIMER_CMD_GET_COUNTDOWN_VALUE:
			puint[0] = armtimer_get_countdown_value();
			break;

		case ARMTIMER_CMD_SET_FREERUN_COUNTER_PRESCALE:
			armtimer_set_freerun_counter_prescale(puint[0]);
			break;

		case ARMTIMER_CMD_GET_FREERUN_COUNTER_PRESCALE:
			puint[0] = armtimer_get_freerun_counter_prescale();
			break;

		case ARMTIMER_CMD_SET_ENABLE_FREERUN_COUNTER:
			armtimer_enable_freerun_counter(puint[0]);
			break;

		case ARMTIMER_CMD_GET_ENABLE_FREERUN_COUNTER:
			puint[0] = armtimer_freerun_counter_is_enabled();
			break;

		case ARMTIMER_CMD_SET_ENABLE_DEBUG_HALT_TIMER:
			armtimer_enable_debug_halt_timer(puint[0]);
			break;

		case ARMTIMER_CMD_GET_ENABLE_DEBUG_HALT_TIMER:
			puint[0] = armtimer_debug_halt_timer_is_enabled();
			break;

		case ARMTIMER_CMD_SET_ENABLE_TIMER:
			armtimer_enable_timer(puint[0]);
			break;

		case ARMTIMER_CMD_GET_ENABLE_TIMER:
			puint[0] = armtimer_timer_is_enabled();
			break;

		case ARMTIMER_CMD_SET_ENABLE_INTR:
			armtimer_enable_intr(puint[0]);
			break;

		case ARMTIMER_CMD_GET_ENABLE_INTR:
			puint[0] = armtimer_intr_is_enabled();
			break;

		case ARMTIMER_CMD_SET_TIMER_PRESCALE:
			armtimer_set_timer_prescale(puint[0]);
			break;

		case ARMTIMER_CMD_GET_TIMER_PRESCALE:
			puint[0] = armtimer_get_timer_prescale();
			break;

		case ARMTIMER_CMD_SET_COUNTSIZE_BITS:
			armtimer_set_count_size_bits(puint[0]);
			break;

		case ARMTIMER_CMD_GET_COUNTSIZE_BITS:
			puint[0] = armtimer_get_count_size_bits();
			break;

		case ARMTIMER_CMD_CLEAR_INTR_FLAGS:
			armtimer_clear_intr_flags();
			break;

		case ARMTIMER_CMD_GET_RAW_INTR_STATUS:
			puint[0] = armtimer_get_raw_intr_status();
			break;

		case ARMTIMER_CMD_GET_MASKED_INTR_STATUS:
			puint[0] = armtimer_get_masked_intr_status();
			break;

		case ARMTIMER_CMD_SET_RELOAD_VALUE:
			armtimer_set_reload_value(puint[0]);
			break;

		case ARMTIMER_CMD_GET_RELOAD_VALUE:
			puint[0] = armtimer_get_reload_value();
			break;

		case ARMTIMER_CMD_SET_PREDIV_VALUE:
			armtimer_set_predivider_value(puint[0]);
			break;

		case ARMTIMER_CMD_GET_PREDIV_VALUE:
			puint[0] = armtimer_get_predivider_value();
			break;

		case ARMTIMER_CMD_GET_FREERUN_COUNTER_VALUE:
			puint[0] = armtimer_get_freerun_counter_value();
			break;
	}

	pbyte[0] = ARMTIMER_CMD_KERNEL_RESPONSE;
	return size;
}

static const struct proc_ops armtimer_proc_ops = {
	.proc_read = armtimer_mod_usrread,
	.proc_write = armtimer_mod_usrwrite
};

static int __init driver_enable(void)
{
	armtimer_mapping = (unsigned int*) ioremap(ARMTIMER_BASE_ADDR, ARMTIMER_MAPPING_SIZE_BYTES);
	if(armtimer_mapping == NULL)
	{
		printk("ARMTIMER: Error mapping ARMTIMER addr\n");
		return -1;
	}

	armtimer_proc = proc_create("ARMTIMER_Ctrl", 0x1B6, NULL, &armtimer_proc_ops);
	if(armtimer_proc == NULL)
	{
		printk("ARMTIMER: Error creating proc file\n");
		return -1;
	}

	armtimer_data_io = vmalloc(ARMTIMER_DATAIO_SIZE_BYTES);
	printk("ARMTIMER Control Driver Enabled\n");
	return 0;
}

static void __exit driver_disable(void)
{
	iounmap(armtimer_mapping);
	proc_remove(armtimer_proc);
	vfree(armtimer_data_io);
	printk("ARMTIMER Control Driver Disabled\n");
	return;
}

module_init(driver_enable);
module_exit(driver_disable);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rafael Sabe");
MODULE_DESCRIPTION("Driver for BCM2837 ARM Timer Control");
