//BCM2837 Interrupt Control Driver

#include "BCM2837_INTR_RegisterMapping.h"
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <asm/io.h>

#define INTR_IRQ_ID_SYSTIMER_MATCH1 1
#define INTR_IRQ_ID_SYSTIMER_MATCH3 3
#define INTR_IRQ_ID_USB_CTRL 9
#define INTR_IRQ_ID_AUX_INTR 29
#define INTR_IRQ_ID_BSCSPI_SLAVE 43
#define INTR_IRQ_ID_PWM0 45
#define INTR_IRQ_ID_PWM1 46
#define INTR_IRQ_ID_SMI 48
#define INTR_IRQ_ID_GPIO_INTR0 49
#define INTR_IRQ_ID_GPIO_INTR1 50
#define INTR_IRQ_ID_GPIO_INTR2 51
#define INTR_IRQ_ID_GPIO_INTR3 52
#define INTR_IRQ_ID_I2C 53
#define INTR_IRQ_ID_SPI 54
#define INTR_IRQ_ID_PCM 55
#define INTR_IRQ_ID_UART 57
#define INTR_IRQ_ID_ARMTIMER 64
#define INTR_IRQ_ID_ARM_MAILBOX 65
#define INTR_IRQ_ID_ARM_DOORBELL0 66
#define INTR_IRQ_ID_ARM_DOORBELL1 67
#define INTR_IRQ_ID_ARM_GPU0_HALT 68
#define INTR_IRQ_ID_ARM_GPU1_HALT 69
#define INTR_IRQ_ID_ARM_ILLEGAL_ACCESS_TYPE1 70
#define INTR_IRQ_ID_ARM_ILLEGAL_ACCESS_TYPE0 71

/*
 * INTR Command Structure (3 BYTES):
 * BYTE0: CMD
 * BYTE1: ARG0
 * BYTE2: ARG1
 */

#define INTR_DATAIO_SIZE_BYTES 3

#define INTR_CMD_GET_BASIC_IRQ_OCCURRED 0
#define INTR_CMD_GET_GPU_IRQ_OCCURRED 1
#define INTR_CMD_SET_ENABLE_FIQ 2
#define INTR_CMD_GET_ENABLE_FIQ 3
#define INTR_CMD_SET_FIQ_SRC 4
#define INTR_CMD_GET_FIQ_SRC 5
#define INTR_CMD_SET_ENABLE_GPU_IRQ 6
#define INTR_CMD_SET_ENABLE_BASIC_IRQ 7

#define INTR_CMD_KERNEL_RESPONSE 0xFF

static struct proc_dir_entry *intr_proc = NULL;
static unsigned int *intr_mapping = NULL;
static void *intr_data_io = NULL;

unsigned int intr_is_reg_bit_active(unsigned int register_value, unsigned int reference_bit)
{
	unsigned int bit_value = (register_value & reference_bit);
	if(bit_value == reference_bit) return 1;
	return 0;
}

//=======================================================================================================
//BASIC PENDING

unsigned int intr_basic_irq_occurred(unsigned int irq_id)
{
	if((irq_id < 64) || (irq_id > 71)) return 0;

	irq_id -= 64;
	return intr_is_reg_bit_active(intr_mapping[INTR_BASIC_PENDING_UINTP_POS], (1 << irq_id));
}

//BASIC PENDING
//=======================================================================================================
//GPU PENDING

unsigned int intr_gpu_irq_occurred(unsigned int irq_id)
{
	if(irq_id >= 64) return 0;

	unsigned int mapping_pos = 0;
	if(irq_id < 32) mapping_pos = INTR_GPU_PENDING0_UINTP_POS;
	else mapping_pos = INTR_GPU_PENDING1_UINTP_POS;

	irq_id %= 32;
	return intr_is_reg_bit_active(intr_mapping[mapping_pos], (1 << irq_id));
}

//GPU PENDING
//=======================================================================================================
//FIQ CTRL

void intr_enable_fiq(unsigned int enable)
{
	enable &= 0x00000001;
	intr_mapping[INTR_FIQ_CTRL_UINTP_POS] &= ~(1 << 7);
	intr_mapping[INTR_FIQ_CTRL_UINTP_POS] |= (enable << 7);
	return;
}

unsigned int intr_fiq_is_enabled(void)
{
	return intr_is_reg_bit_active(intr_mapping[INTR_FIQ_CTRL_UINTP_POS], (1 << 7));
}

void intr_set_fiq_src(unsigned int irq_id)
{
	if(irq_id > 71) return;

	intr_mapping[INTR_FIQ_CTRL_UINTP_POS] &= ~(0x7F);
	intr_mapping[INTR_FIQ_CTRL_UINTP_POS] |= (irq_id);
	return;
}

unsigned int intr_get_fiq_src(void)
{
	unsigned int irq_id = intr_mapping[INTR_FIQ_CTRL_UINTP_POS];
	irq_id &= (0x7F);
	return irq_id;
}

//FIQ CTRL
//=======================================================================================================
//INTR GPU ENABLE/DISABLE

void intr_enable_gpu_irq(unsigned int irq_id, unsigned int enable)
{
	if(irq_id >= 64) return;

	enable &= 0x00000001;
	unsigned int mapping_pos = 0;
	if(enable)
	{
		if(irq_id < 32) mapping_pos = INTR_GPU_ENABLE0_UINTP_POS;
		else mapping_pos = INTR_GPU_ENABLE1_UINTP_POS;
	}
	else
	{
		if(irq_id < 32) mapping_pos = INTR_GPU_DISABLE0_UINTP_POS;
		else mapping_pos = INTR_GPU_DISABLE1_UINTP_POS;
	}

	irq_id %= 32;
	intr_mapping[mapping_pos] = (1 << irq_id);
	return;
}

//INTR GPU ENABLE/DISABLE
//=======================================================================================================
//INTR BASIC ENABLE/DISABLE

void intr_enable_basic_irq(unsigned int irq_id, unsigned int enable)
{
	if((irq_id < 64) || (irq_id > 71)) return;

	enable &= 0x00000001;
	unsigned int mapping_pos = 0;
	if(enable) mapping_pos = INTR_BASIC_ENABLE_UINTP_POS;
	else mapping_pos = INTR_BASIC_DISABLE_UINTP_POS;

	irq_id -= 64;
	intr_mapping[mapping_pos] = (1 << irq_id);
	return;
}

//INTR BASIC ENABLE/DISABLE
//=======================================================================================================

ssize_t intr_mod_usrread(struct file *file, char __user *user, size_t size, loff_t *offset)
{
	copy_to_user(user, intr_data_io, INTR_DATAIO_SIZE_BYTES);
	return size;
}

ssize_t intr_mod_usrwrite(struct file *file, const char __user *user, size_t size, loff_t *offset)
{
	copy_from_user(intr_data_io, user, INTR_DATAIO_SIZE_BYTES);

	unsigned char *pbyte = (unsigned char*) intr_data_io;

	switch(pbyte[0])
	{
		case INTR_CMD_GET_BASIC_IRQ_OCCURRED:
			pbyte[1] = intr_basic_irq_occurred(pbyte[1]);
			break;

		case INTR_CMD_GET_GPU_IRQ_OCCURRED:
			pbyte[1] = intr_gpu_irq_occurred(pbyte[1]);
			break;

		case INTR_CMD_SET_ENABLE_FIQ:
			intr_enable_fiq(pbyte[1]);
			break;

		case INTR_CMD_GET_ENABLE_FIQ:
			pbyte[1] = intr_fiq_is_enabled();
			break;

		case INTR_CMD_SET_FIQ_SRC:
			intr_set_fiq_src(pbyte[1]);
			break;

		case INTR_CMD_GET_FIQ_SRC:
			pbyte[1] = intr_get_fiq_src();
			break;

		case INTR_CMD_SET_ENABLE_GPU_IRQ:
			intr_enable_gpu_irq(pbyte[1], pbyte[2]);
			break;

		case INTR_CMD_SET_ENABLE_BASIC_IRQ:
			intr_enable_basic_irq(pbyte[1], pbyte[2]);
			break;
	}

	pbyte[0] = INTR_CMD_KERNEL_RESPONSE;
	return size;
}

static const struct proc_ops intr_proc_ops = {
	.proc_read = intr_mod_usrread,
	.proc_write = intr_mod_usrwrite
};

static int __init driver_enable(void)
{
	intr_mapping = (unsigned int*) ioremap(INTR_BASE_ADDR, INTR_MAPPING_SIZE_BYTES);
	if(intr_mapping == NULL)
	{
		printk("INTR: Error mapping INTR addr\n");
		return -1;
	}

	intr_proc = proc_create("INTR_Ctrl", 0x1B6, NULL, &intr_proc_ops);
	if(intr_proc == NULL)
	{
		printk("INTR: Error creating proc file\n");
		return -1;
	}

	intr_data_io = vmalloc(INTR_DATAIO_SIZE_BYTES);
	printk("INTR Control Driver Enabled\n");
	return 0;
}

static void __exit driver_disable(void)
{
	iounmap(intr_mapping);
	proc_remove(intr_proc);
	vfree(intr_data_io);
	printk("INTR Control Driver Disabled\n");
	return;
}

module_init(driver_enable);
module_exit(driver_disable);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rafael Sabe");
MODULE_DESCRIPTION("Driver for BCM2837 Interrupt Control");
