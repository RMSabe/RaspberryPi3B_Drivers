//BCM2837 DMA Control Driver

#include "BCM2837_DMA_RegisterMapping.h"
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <asm/io.h>

#define DMA_TYPE_STD 0
#define DMA_TYPE_LITE 1

#define DMA_STD_CH0 0
#define DMA_STD_CH1 1
#define DMA_STD_CH2 2
#define DMA_STD_CH3 3
#define DMA_STD_CH4 4
#define DMA_STD_CH5 5
#define DMA_STD_CH6 6

#define DMA_LITE_CH0 7
#define DMA_LITE_CH1 8
#define DMA_LITE_CH2 9
#define DMA_LITE_CH3 10
#define DMA_LITE_CH4 11
#define DMA_LITE_CH5 12
#define DMA_LITE_CH6 13
#define DMA_LITE_CH7 14

#define DMA_CTRLBLOCK_SIZE_BYTES 256

/*
 * DMA Command Structure (6 BYTES):
 *
 * BYTE0: CMD
 * BYTE1: DMA CTRL
 * BYTES 2-5 (1 UINT): ARG
 */

#define DMA_DATAIO_SIZE_BYTES 6

#define DMA_CMD_SET_ENABLE_CTRL 0
#define DMA_CMD_GET_ENABLE_CTRL 1
#define DMA_CMD_GET_CHANNEL_INTR_STATUS 2
#define DMA_CMD_GET_FULL_INTR_STATUS 3
#define DMA_CMD_SET_CTRLBLOCK_ADDR 4
#define DMA_CMD_GET_CTRLBLOCK_ADDR 5
#define DMA_CMD_SET_TRANSFER_ACTIVE 6
#define DMA_CMD_GET_TRANSFER_ACTIVE 7
#define DMA_CMD_GET_TRANSFER_DONE 8
#define DMA_CMD_GET_INTR_STATUS 9
#define DMA_CMD_GET_IS_REQUESTING_DATA 10
#define DMA_CMD_GET_IS_PAUSED 11
#define DMA_CMD_GET_IS_PAUSED_BY_INACTIVE_DREQ 12
#define DMA_CMD_GET_IS_WAITING_OSTD_WRITES 13
#define DMA_CMD_GET_ERROR_OCCURRED 14
#define DMA_CMD_SET_PRIORITY 15
#define DMA_CMD_GET_PRIORITY 16
#define DMA_CMD_SET_PANIC_PRIORITY 17
#define DMA_CMD_GET_PANIC_PRIORITY 18
#define DMA_CMD_SET_ENABLE_WAIT_OSTD_WRITES 19
#define DMA_CMD_GET_ENABLE_WAIT_OSTD_WRITES 20
#define DMA_CMD_SET_DISABLE_DEBUG_PAUSE 21
#define DMA_CMD_GET_DISABLE_DEBUG_PAUSE 22
#define DMA_CMD_ABORT 23
#define DMA_CMD_RESET 24
#define DMA_CMD_GET_DISABLE_WIDE_BURSTS 25
#define DMA_CMD_GET_WAIT_CYCLES 26
#define DMA_CMD_GET_PERMAP 27
#define DMA_CMD_GET_BURST_LENGTH 28
#define DMA_CMD_GET_ENABLE_IGNORE_SRC_READS 29
#define DMA_CMD_GET_DREQ_CALLS_SRC_READS 30
#define DMA_CMD_GET_ENABLE_SRC_READ_128BIT_WIDTH 31
#define DMA_CMD_GET_ENABLE_SRC_ADDR_INC 32
#define DMA_CMD_GET_ENABLE_IGNORE_DST_WRITES 33
#define DMA_CMD_GET_DREQ_CALLS_DST_WRITES 34
#define DMA_CMD_GET_ENABLE_DST_WRITE_128BIT_WIDTH 35
#define DMA_CMD_GET_ENABLE_DST_ADDR_INC 36
#define DMA_CMD_GET_ENABLE_WAIT_WRITE_RESPONSE 37
#define DMA_CMD_GET_ENABLE_TDMODE 38
#define DMA_CMD_GET_ENABLE_INTR 39
#define DMA_CMD_GET_SRC_ADDR 40
#define DMA_CMD_GET_DST_ADDR 41
#define DMA_CMD_GET_TRANSFER_LENGTH_BYTES 42
#define DMA_CMD_GET_TRANSFER_LENGTH_EXT 43
#define DMA_CMD_GET_SRC_STRIDE 44
#define DMA_CMD_GET_DST_STRIDE 45
#define DMA_CMD_GET_NEXTCB_ADDR 46
#define DMA_CMD_DEBUG_GET_IS_TYPE_LITE 47
#define DMA_CMD_DEBUG_GET_VERSION 48
#define DMA_CMD_DEBUG_GET_STATE 49
#define DMA_CMD_DEBUG_GET_ID 50
#define DMA_CMD_DEBUG_GET_OSTD_WRITES_COUNTER 51
#define DMA_CMD_DEBUG_GET_READ_ERROR 52
#define DMA_CMD_DEBUG_GET_FIFO_ERROR 53
#define DMA_CMD_DEBUG_GET_READLASTNOTSET_ERROR 54

#define DMA_CMD_KERNEL_RESPONSE 0xFF

static struct proc_dir_entry *dma_proc = NULL;
static unsigned int **dma_std_mapping_group = NULL;
static unsigned int **dma_lite_mapping_group = NULL;
static unsigned int *dma_intr_status_reg = NULL;
static unsigned int *dma_channel_enable_reg = NULL;
static void *dma_data_io = NULL;

//=====================================================================================================================

unsigned int dma_is_reg_bit_active(unsigned int register_value, unsigned int reference_bit)
{
	unsigned int bit_value = (register_value & reference_bit);

	if(bit_value == reference_bit) return 1;
	return 0;
}

void dma_ctrl_map_to_type_pointer(unsigned int dma_ctrl, unsigned int *p_type, unsigned int **p_pointer)
{
	switch(dma_ctrl)
	{
		case DMA_STD_CH0:
			if(p_type != NULL) *p_type = DMA_TYPE_STD;
			if(p_pointer != NULL) *p_pointer = dma_std_mapping_group[0];
			break;

		case DMA_STD_CH1:
			if(p_type != NULL) *p_type = DMA_TYPE_STD;
			if(p_pointer != NULL) *p_pointer = dma_std_mapping_group[1];
			break;

		case DMA_STD_CH2:
			if(p_type != NULL) *p_type = DMA_TYPE_STD;
			if(p_pointer != NULL) *p_pointer = dma_std_mapping_group[2];
			break;

		case DMA_STD_CH3:
			if(p_type != NULL) *p_type = DMA_TYPE_STD;
			if(p_pointer != NULL) *p_pointer = dma_std_mapping_group[3];
			break;

		case DMA_STD_CH4:
			if(p_type != NULL) *p_type = DMA_TYPE_STD;
			if(p_pointer != NULL) *p_pointer = dma_std_mapping_group[4];
			break;

		case DMA_STD_CH5:
			if(p_type != NULL) *p_type = DMA_TYPE_STD;
			if(p_pointer != NULL) *p_pointer = dma_std_mapping_group[5];
			break;

		case DMA_STD_CH6:
			if(p_type != NULL) *p_type = DMA_TYPE_STD;
			if(p_pointer != NULL) *p_pointer = dma_std_mapping_group[6];
			break;

		case DMA_LITE_CH0:
			if(p_type != NULL) *p_type = DMA_TYPE_LITE;
			if(p_pointer != NULL) *p_pointer = dma_lite_mapping_group[0];
			break;

		case DMA_LITE_CH1:
			if(p_type != NULL) *p_type = DMA_TYPE_LITE;
			if(p_pointer != NULL) *p_pointer = dma_lite_mapping_group[1];
			break;

		case DMA_LITE_CH2:
			if(p_type != NULL) *p_type = DMA_TYPE_LITE;
			if(p_pointer != NULL) *p_pointer = dma_lite_mapping_group[2];
			break;

		case DMA_LITE_CH3:
			if(p_type != NULL) *p_type = DMA_TYPE_LITE;
			if(p_pointer != NULL) *p_pointer = dma_lite_mapping_group[3];
			break;

		case DMA_LITE_CH4:
			if(p_type != NULL) *p_type = DMA_TYPE_LITE;
			if(p_pointer != NULL) *p_pointer = dma_lite_mapping_group[4];
			break;

		case DMA_LITE_CH5:
			if(p_type != NULL) *p_type = DMA_TYPE_LITE;
			if(p_pointer != NULL) *p_pointer = dma_lite_mapping_group[5];
			break;

		case DMA_LITE_CH6:
			if(p_type != NULL) *p_type = DMA_TYPE_LITE;
			if(p_pointer != NULL) *p_pointer = dma_lite_mapping_group[6];
			break;

		case DMA_LITE_CH7:
			if(p_type != NULL) *p_type = DMA_TYPE_LITE;
			if(p_pointer != NULL) *p_pointer = dma_lite_mapping_group[7];
			break;
	}

	return;
}

//=====================================================================================================================
//CTRL STATUS

void dma_reset(unsigned int dma_ctrl)
{
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, NULL, &dma_mapping);

	dma_mapping[DMA_CTRL_STATUS_UINTP_POS] |= (1 << 31);
	return;
}

void dma_abort(unsigned int dma_ctrl)
{
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, NULL, &dma_mapping);

	dma_mapping[DMA_CTRL_STATUS_UINTP_POS] |= (1 << 30);
	return;
}

void dma_disable_debug_pause(unsigned int dma_ctrl, unsigned int disable)
{
	disable &= 0x00000001;
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, NULL, &dma_mapping);

	dma_mapping[DMA_CTRL_STATUS_UINTP_POS] &= ~(1 << 29);
	dma_mapping[DMA_CTRL_STATUS_UINTP_POS] |= (disable << 29);
	return;
}

unsigned int dma_debug_pause_is_disabled(unsigned int dma_ctrl)
{
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, NULL, &dma_mapping);

	return dma_is_reg_bit_active(dma_mapping[DMA_CTRL_STATUS_UINTP_POS], (1 << 29));
}

void dma_enable_wait_ostd_writes(unsigned int dma_ctrl, unsigned int enable)
{
	enable &= 0x00000001;
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, NULL, &dma_mapping);

	dma_mapping[DMA_CTRL_STATUS_UINTP_POS] &= ~(1 << 28);
	dma_mapping[DMA_CTRL_STATUS_UINTP_POS] |= (enable << 28);
	return;
}

unsigned int dma_wait_ostd_writes_is_enabled(unsigned int dma_ctrl)
{
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, NULL, &dma_mapping);

	return dma_is_reg_bit_active(dma_mapping[DMA_CTRL_STATUS_UINTP_POS], (1 << 28));
}

void dma_set_panic_priority(unsigned int dma_ctrl, unsigned int priority)
{
	priority &= 0x0000000F;
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, NULL, &dma_mapping);

	dma_mapping[DMA_CTRL_STATUS_UINTP_POS] &= ~(0xF << 20);
	dma_mapping[DMA_CTRL_STATUS_UINTP_POS] |= (priority << 20);
	return;
}

unsigned int dma_get_panic_priority(unsigned int dma_ctrl)
{
	unsigned int priority = 0;
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, NULL, &dma_mapping);

	priority = dma_mapping[DMA_CTRL_STATUS_UINTP_POS];
	priority &= (0xF << 20);
	return (priority >> 20);
}

void dma_set_priority(unsigned int dma_ctrl, unsigned int priority)
{
	priority &= 0x0000000F;
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, NULL, &dma_mapping);

	dma_mapping[DMA_CTRL_STATUS_UINTP_POS] &= ~(0xF << 16);
	dma_mapping[DMA_CTRL_STATUS_UINTP_POS] |= (priority << 16);
	return;
}

unsigned int dma_get_priority(unsigned int dma_ctrl)
{
	unsigned int priority = 0;
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, NULL, &dma_mapping);

	priority = dma_mapping[DMA_CTRL_STATUS_UINTP_POS];
	priority &= (0xF << 16);
	return (priority >> 16);
}

unsigned int dma_error_occurred(unsigned int dma_ctrl)
{
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, NULL, &dma_mapping);

	return dma_is_reg_bit_active(dma_mapping[DMA_CTRL_STATUS_UINTP_POS], (1 << 8));
}

unsigned int dma_is_waiting_ostd_writes(unsigned int dma_ctrl)
{
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, NULL, &dma_mapping);

	return dma_is_reg_bit_active(dma_mapping[DMA_CTRL_STATUS_UINTP_POS], (1 << 6));
}

unsigned int dma_is_paused_by_inactive_dreq(unsigned int dma_ctrl)
{
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, NULL, &dma_mapping);

	return dma_is_reg_bit_active(dma_mapping[DMA_CTRL_STATUS_UINTP_POS], (1 << 5));
}

unsigned int dma_is_paused(unsigned int dma_ctrl)
{
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, NULL, &dma_mapping);

	return dma_is_reg_bit_active(dma_mapping[DMA_CTRL_STATUS_UINTP_POS], (1 << 4));
}

unsigned int dma_is_requesting_data(unsigned int dma_ctrl)
{
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, NULL, &dma_mapping);

	return dma_is_reg_bit_active(dma_mapping[DMA_CTRL_STATUS_UINTP_POS], (1 << 3));
}

unsigned int dma_get_intr_status(unsigned int dma_ctrl)
{
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, NULL, &dma_mapping);

	if(dma_is_reg_bit_active(dma_mapping[DMA_CTRL_STATUS_UINTP_POS], (1 << 2)))
	{
		dma_mapping[DMA_CTRL_STATUS_UINTP_POS] |= (1 << 2);
		return 1;
	}

	return 0;
}

unsigned int dma_transfer_done(unsigned int dma_ctrl)
{
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, NULL, &dma_mapping);

	if(dma_is_reg_bit_active(dma_mapping[DMA_CTRL_STATUS_UINTP_POS], (1 << 1)))
	{
		dma_mapping[DMA_CTRL_STATUS_UINTP_POS] |= (1 << 1);
		return 1;
	}

	return 0;
}

void dma_set_transfer_active(unsigned int dma_ctrl, unsigned int active)
{
	active &= 0x00000001;
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, NULL, &dma_mapping);

	dma_mapping[DMA_CTRL_STATUS_UINTP_POS] &= ~(1);
	dma_mapping[DMA_CTRL_STATUS_UINTP_POS] |= (active);
	return;
}

unsigned int dma_get_transfer_active(unsigned int dma_ctrl)
{
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, NULL, &dma_mapping);

	return dma_is_reg_bit_active(dma_mapping[DMA_CTRL_STATUS_UINTP_POS], (1));
}

//CTRL STATUS
//=====================================================================================================================
//CTRL BLOCK ADDR

void dma_set_ctrlblock_addr(unsigned int dma_ctrl, unsigned int addr)
{
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, NULL, &dma_mapping);

	dma_mapping[DMA_CTRLBLOCK_ADDR_UINTP_POS] = addr;
	return;
}

unsigned int dma_get_ctrlblock_addr(unsigned int dma_ctrl)
{
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, NULL, &dma_mapping);

	return dma_mapping[DMA_CTRLBLOCK_ADDR_UINTP_POS];
}

//CTRL BLOCK ADDR
//=====================================================================================================================
//TRANSFER INFO (READ ONLY)

unsigned int dma_wide_bursts_is_disabled(unsigned int dma_ctrl)
{
	unsigned int dma_type = 0;
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, &dma_type, &dma_mapping);

	if(dma_type == DMA_TYPE_LITE) return 0;
	return dma_is_reg_bit_active(dma_mapping[DMA_TRANSFER_INFO_UINTP_POS], (1 << 26));
}

unsigned int dma_get_wait_cycles(unsigned int dma_ctrl)
{
	unsigned int wait_cycles = 0;
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, NULL, &dma_mapping);

	wait_cycles = dma_mapping[DMA_TRANSFER_INFO_UINTP_POS];
	wait_cycles &= (0x1F << 21);
	return (wait_cycles >> 21);
}

unsigned int dma_get_permap(unsigned int dma_ctrl)
{
	unsigned int permap = 0;
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, NULL, &dma_mapping);

	permap = dma_mapping[DMA_TRANSFER_INFO_UINTP_POS];
	permap &= (0x1F << 16);
	return (permap >> 16);
}

unsigned int dma_get_burst_length(unsigned int dma_ctrl)
{
	unsigned int length = 0;
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, NULL, &dma_mapping);

	length = dma_mapping[DMA_TRANSFER_INFO_UINTP_POS];
	length &= (0xF << 12);
	return (length >> 12);
}

unsigned int dma_ignore_src_reads_is_enabled(unsigned int dma_ctrl)
{
	unsigned int dma_type = 0;
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, &dma_type, &dma_mapping);

	if(dma_type == DMA_TYPE_LITE) return 0;
	return dma_is_reg_bit_active(dma_mapping[DMA_TRANSFER_INFO_UINTP_POS], (1 << 11));
}

unsigned int dma_dreq_calls_src_reads(unsigned int dma_ctrl)
{
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, NULL, &dma_mapping);

	return dma_is_reg_bit_active(dma_mapping[DMA_TRANSFER_INFO_UINTP_POS], (1 << 10));
}

unsigned int dma_src_read_128bit_width_is_enabled(unsigned int dma_ctrl)
{
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, NULL, &dma_mapping);

	return dma_is_reg_bit_active(dma_mapping[DMA_TRANSFER_INFO_UINTP_POS], (1 << 9));
}

unsigned int dma_src_addr_inc_is_enabled(unsigned int dma_ctrl)
{
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, NULL, &dma_mapping);

	return dma_is_reg_bit_active(dma_mapping[DMA_TRANSFER_INFO_UINTP_POS], (1 << 8));
}

unsigned int dma_ignore_dst_writes_is_enabled(unsigned int dma_ctrl)
{
	unsigned int dma_type = 0;
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, &dma_type, &dma_mapping);

	if(dma_type == DMA_TYPE_LITE) return 0;
	return dma_is_reg_bit_active(dma_mapping[DMA_TRANSFER_INFO_UINTP_POS], (1 << 7));
}

unsigned int dma_dreq_calls_dst_writes(unsigned int dma_ctrl)
{
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, NULL, &dma_mapping);

	return dma_is_reg_bit_active(dma_mapping[DMA_TRANSFER_INFO_UINTP_POS], (1 << 6));
}

unsigned int dma_dst_write_128bit_width_is_enabled(unsigned int dma_ctrl)
{
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, NULL, &dma_mapping);

	return dma_is_reg_bit_active(dma_mapping[DMA_TRANSFER_INFO_UINTP_POS], (1 << 5));
}

unsigned int dma_dst_addr_inc_is_enabled(unsigned int dma_ctrl)
{
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, NULL, &dma_mapping);

	return dma_is_reg_bit_active(dma_mapping[DMA_TRANSFER_INFO_UINTP_POS], (1 << 4));
}

unsigned int dma_wait_write_response_is_enabled(unsigned int dma_ctrl)
{
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, NULL, &dma_mapping);

	return dma_is_reg_bit_active(dma_mapping[DMA_TRANSFER_INFO_UINTP_POS], (1 << 3));
}

unsigned int dma_tdmode_is_enabled(unsigned int dma_ctrl)
{
	unsigned int dma_type = 0;
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, &dma_type, &dma_mapping);

	if(dma_type == DMA_TYPE_LITE) return 0;
	return dma_is_reg_bit_active(dma_mapping[DMA_TRANSFER_INFO_UINTP_POS], (1 << 1));
}

unsigned int dma_intr_is_enabled(unsigned int dma_ctrl)
{
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, NULL, &dma_mapping);

	return dma_is_reg_bit_active(dma_mapping[DMA_TRANSFER_INFO_UINTP_POS], (1));
}

//TRANSFER INFO (READ ONLY)
//=====================================================================================================================
//SRC ADDR (READ ONLY)

unsigned int dma_get_src_addr(unsigned int dma_ctrl)
{
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, NULL, &dma_mapping);

	return dma_mapping[DMA_SRC_ADDR_UINTP_POS];
}

//SRC ADDR (READ ONLY)
//=====================================================================================================================
//DST ADDR (READ ONLY)

unsigned int dma_get_dst_addr(unsigned int dma_ctrl)
{
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, NULL, &dma_mapping);

	return dma_mapping[DMA_DST_ADDR_UINTP_POS];
}

//DST ADDR (READ ONLY)
//=====================================================================================================================
//TRANSFER LENGTH (READ ONLY)

unsigned int dma_get_transfer_length_bytes(unsigned int dma_ctrl)
{
	unsigned int length = 0;
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, NULL, &dma_mapping);

	length = dma_mapping[DMA_TRANSFER_LENGTH_UINTP_POS];
	length &= (0xFFFF);
	return length;
}

unsigned int dma_get_transfer_length_ext(unsigned int dma_ctrl)
{
	unsigned int length = 0;
	unsigned int dma_type = 0;
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, &dma_type, &dma_mapping);

	if(dma_type == DMA_TYPE_LITE) return 0;

	length = dma_mapping[DMA_TRANSFER_LENGTH_UINTP_POS];
	length &= (0x3FFF << 16);
	return (length >> 16);
}

//TRANSFER LENGTH (READ ONLY)
//=====================================================================================================================
//STRIDE (READ ONLY)

unsigned int dma_get_src_stride(unsigned int dma_ctrl)
{
	unsigned int stride = 0;
	unsigned int dma_type = 0;
	unsigned int *dma_mapping = 0;
	dma_ctrl_map_to_type_pointer(dma_ctrl, &dma_type, &dma_mapping);

	if(dma_type == DMA_TYPE_LITE) return 0;

	stride = dma_mapping[DMA_STRIDE_UINTP_POS];
	stride &= (0xFFFF);
	return stride;
}

unsigned int dma_get_dst_stride(unsigned int dma_ctrl)
{
	unsigned int stride = 0;
	unsigned int dma_type = 0;
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, &dma_type, &dma_mapping);

	if(dma_type == DMA_TYPE_LITE) return 0;

	stride = dma_mapping[DMA_STRIDE_UINTP_POS];
	stride &= (0xFFFF << 16);
	return (stride >> 16);
}

//STRIDE (READ ONLY)
//=====================================================================================================================
//NEXT CTRL BLOCK ADDR (READ ONLY)

unsigned int dma_get_next_ctrlblock_addr(unsigned int dma_ctrl)
{
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, NULL, &dma_mapping);

	return dma_mapping[DMA_NEXTCB_ADDR_UINTP_POS];
}

//NEXT CTRL BLOCK ADDR (READ ONLY)
//=====================================================================================================================
//DEBUG

unsigned int dma_debug_is_type_lite(unsigned int dma_ctrl)
{
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, NULL, &dma_mapping);

	return dma_is_reg_bit_active(dma_mapping[DMA_DEBUG_UINTP_POS], (1 << 28));
}

unsigned int dma_debug_get_version(unsigned int dma_ctrl)
{
	unsigned int version = 0;
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, NULL, &dma_mapping);

	version = dma_mapping[DMA_DEBUG_UINTP_POS];
	version &= (0x7 << 25);
	return (version >> 25);
}

unsigned int dma_debug_get_state(unsigned int dma_ctrl)
{
	unsigned int state = 0;
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, NULL, &dma_mapping);

	state = dma_mapping[DMA_DEBUG_UINTP_POS];
	state &= (0x1FF << 16);
	return (state >> 16);
}

unsigned int dma_debug_get_id(unsigned int dma_ctrl)
{
	unsigned int id = 0;
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, NULL, &dma_mapping);

	id = dma_mapping[DMA_DEBUG_UINTP_POS];
	id &= (0xFF << 8);
	return (id >> 8);
}

unsigned int dma_debug_get_ostd_writes_counter(unsigned int dma_ctrl)
{
	unsigned int counter = 0;
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, NULL, &dma_mapping);

	counter = dma_mapping[DMA_DEBUG_UINTP_POS];
	counter &= (0xF << 4);
	return (counter >> 4);
}

unsigned int dma_debug_get_read_error(unsigned int dma_ctrl)
{
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, NULL, &dma_mapping);

	if(dma_is_reg_bit_active(dma_mapping[DMA_DEBUG_UINTP_POS], (1 << 2)))
	{
		dma_mapping[DMA_DEBUG_UINTP_POS] |= (1 << 2);
		return 1;
	}

	return 0;
}

unsigned int dma_debug_get_fifo_error(unsigned int dma_ctrl)
{
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, NULL, &dma_mapping);

	if(dma_is_reg_bit_active(dma_mapping[DMA_DEBUG_UINTP_POS], (1 << 1)))
	{
		dma_mapping[DMA_DEBUG_UINTP_POS] |= (1 << 1);
		return 1;
	}

	return 0;
}

unsigned int dma_debug_get_read_last_not_set_error(unsigned int dma_ctrl)
{
	unsigned int *dma_mapping = NULL;
	dma_ctrl_map_to_type_pointer(dma_ctrl, NULL, &dma_mapping);

	if(dma_is_reg_bit_active(dma_mapping[DMA_DEBUG_UINTP_POS], (1)))
	{
		dma_mapping[DMA_DEBUG_UINTP_POS] |= (1);
		return 1;
	}

	return 0;
}

//DEBUG
//=====================================================================================================================
//INTR STATUS

unsigned int dma_get_full_intr_status(void)
{
	unsigned int full_status = *dma_intr_status_reg;
	full_status &= 0x00007FFF;
	return full_status;
}

unsigned int dma_get_channel_intr_status(unsigned int dma_ctrl)
{
	return dma_is_reg_bit_active(*dma_intr_status_reg, (1 << dma_ctrl));
}

//INTR STATUS
//=====================================================================================================================
//ENABLE CHANNEL

void dma_enable_ctrl(unsigned int dma_ctrl, unsigned int enable)
{
	if(dma_ctrl > 14) return;

	enable &= 0x00000001;
	*dma_channel_enable_reg &= ~(1 << dma_ctrl);
	*dma_channel_enable_reg |= (enable << dma_ctrl);
	return;
}

unsigned int dma_ctrl_is_enabled(unsigned int dma_ctrl)
{
	if(dma_ctrl > 14) return 0;

	return dma_is_reg_bit_active(*dma_channel_enable_reg, (1 << dma_ctrl));
}

//ENABLE CHANNEL
//=====================================================================================================================

ssize_t dma_mod_usrread(struct file *file, char __user *user, size_t size, loff_t *offset)
{
	copy_to_user(user, dma_data_io, DMA_DATAIO_SIZE_BYTES);
	return size;
}

ssize_t dma_mod_usrwrite(struct file *file, const char __user *user, size_t size, loff_t *offset)
{
	copy_from_user(dma_data_io, user, DMA_DATAIO_SIZE_BYTES);

	unsigned char *pbyte = (unsigned char*) dma_data_io;
	unsigned int *puint = (unsigned int*) &pbyte[2];

	switch(pbyte[0])
	{
		case DMA_CMD_SET_ENABLE_CTRL:
			dma_enable_ctrl(pbyte[1], puint[0]);
			break;

		case DMA_CMD_GET_ENABLE_CTRL:
			puint[0] = dma_ctrl_is_enabled(pbyte[1]);
			break;

		case DMA_CMD_GET_CHANNEL_INTR_STATUS:
			puint[0] = dma_get_channel_intr_status(pbyte[1]);
			break;

		case DMA_CMD_GET_FULL_INTR_STATUS:
			puint[0] = dma_get_full_intr_status();
			break;

		case DMA_CMD_SET_CTRLBLOCK_ADDR:
			dma_set_ctrlblock_addr(pbyte[1], puint[0]);
			break;

		case DMA_CMD_GET_CTRLBLOCK_ADDR:
			puint[0] = dma_get_ctrlblock_addr(pbyte[1]);
			break;

		case DMA_CMD_SET_TRANSFER_ACTIVE:
			dma_set_transfer_active(pbyte[1], puint[0]);
			break;

		case DMA_CMD_GET_TRANSFER_ACTIVE:
			puint[0] = dma_get_transfer_active(pbyte[1]);
			break;

		case DMA_CMD_GET_TRANSFER_DONE:
			puint[0] = dma_transfer_done(pbyte[1]);
			break;

		case DMA_CMD_GET_INTR_STATUS:
			puint[0] = dma_get_intr_status(pbyte[1]);
			break;

		case DMA_CMD_GET_IS_REQUESTING_DATA:
			puint[0] = dma_is_requesting_data(pbyte[1]);
			break;

		case DMA_CMD_GET_IS_PAUSED:
			puint[0] = dma_is_paused(pbyte[1]);
			break;

		case DMA_CMD_GET_IS_PAUSED_BY_INACTIVE_DREQ:
			puint[0] = dma_is_paused_by_inactive_dreq(pbyte[1]);
			break;

		case DMA_CMD_GET_IS_WAITING_OSTD_WRITES:
			puint[0] = dma_is_waiting_ostd_writes(pbyte[1]);
			break;

		case DMA_CMD_GET_ERROR_OCCURRED:
			puint[0] = dma_error_occurred(pbyte[1]);
			break;

		case DMA_CMD_SET_PRIORITY:
			dma_set_priority(pbyte[1], puint[0]);
			break;

		case DMA_CMD_GET_PRIORITY:
			puint[0] = dma_get_priority(pbyte[1]);
			break;

		case DMA_CMD_SET_PANIC_PRIORITY:
			dma_set_panic_priority(pbyte[1], puint[0]);
			break;

		case DMA_CMD_GET_PANIC_PRIORITY:
			puint[0] = dma_get_panic_priority(pbyte[1]);
			break;

		case DMA_CMD_SET_ENABLE_WAIT_OSTD_WRITES:
			dma_enable_wait_ostd_writes(pbyte[1], puint[0]);
			break;

		case DMA_CMD_GET_ENABLE_WAIT_OSTD_WRITES:
			puint[0] = dma_wait_ostd_writes_is_enabled(pbyte[1]);
			break;

		case DMA_CMD_SET_DISABLE_DEBUG_PAUSE:
			dma_disable_debug_pause(pbyte[1], puint[0]);
			break;

		case DMA_CMD_GET_DISABLE_DEBUG_PAUSE:
			puint[0] = dma_debug_pause_is_disabled(pbyte[1]);
			break;

		case DMA_CMD_ABORT:
			dma_abort(pbyte[1]);
			break;

		case DMA_CMD_RESET:
			dma_reset(pbyte[1]);
			break;

		case DMA_CMD_GET_DISABLE_WIDE_BURSTS:
			puint[0] = dma_wide_bursts_is_disabled(pbyte[1]);
			break;

		case DMA_CMD_GET_WAIT_CYCLES:
			puint[0] = dma_get_wait_cycles(pbyte[1]);
			break;

		case DMA_CMD_GET_PERMAP:
			puint[0] = dma_get_permap(pbyte[1]);
			break;

		case DMA_CMD_GET_BURST_LENGTH:
			puint[0] = dma_get_burst_length(pbyte[1]);
			break;

		case DMA_CMD_GET_ENABLE_IGNORE_SRC_READS:
			puint[0] = dma_ignore_src_reads_is_enabled(pbyte[1]);
			break;

		case DMA_CMD_GET_DREQ_CALLS_SRC_READS:
			puint[0] = dma_dreq_calls_src_reads(pbyte[1]);
			break;

		case DMA_CMD_GET_ENABLE_SRC_READ_128BIT_WIDTH:
			puint[0] = dma_src_read_128bit_width_is_enabled(pbyte[1]);
			break;

		case DMA_CMD_GET_ENABLE_SRC_ADDR_INC:
			puint[0] = dma_src_addr_inc_is_enabled(pbyte[1]);
			break;

		case DMA_CMD_GET_ENABLE_IGNORE_DST_WRITES:
			puint[0] = dma_ignore_dst_writes_is_enabled(pbyte[1]);
			break;

		case DMA_CMD_GET_DREQ_CALLS_DST_WRITES:
			puint[0] = dma_dreq_calls_dst_writes(pbyte[1]);
			break;

		case DMA_CMD_GET_ENABLE_DST_WRITE_128BIT_WIDTH:
			puint[0] = dma_dst_write_128bit_width_is_enabled(pbyte[1]);
			break;

		case DMA_CMD_GET_ENABLE_DST_ADDR_INC:
			puint[0] = dma_dst_addr_inc_is_enabled(pbyte[1]);
			break;

		case DMA_CMD_GET_ENABLE_WAIT_WRITE_RESPONSE:
			puint[0] = dma_wait_write_response_is_enabled(pbyte[1]);
			break;

		case DMA_CMD_GET_ENABLE_TDMODE:
			puint[0] = dma_tdmode_is_enabled(pbyte[1]);
			break;

		case DMA_CMD_GET_ENABLE_INTR:
			puint[0] = dma_intr_is_enabled(pbyte[1]);
			break;

		case DMA_CMD_GET_SRC_ADDR:
			puint[0] = dma_get_src_addr(pbyte[1]);
			break;

		case DMA_CMD_GET_DST_ADDR:
			puint[0] = dma_get_dst_addr(pbyte[1]);
			break;

		case DMA_CMD_GET_TRANSFER_LENGTH_BYTES:
			puint[0] = dma_get_transfer_length_bytes(pbyte[1]);
			break;

		case DMA_CMD_GET_TRANSFER_LENGTH_EXT:
			puint[0] = dma_get_transfer_length_ext(pbyte[1]);
			break;

		case DMA_CMD_GET_SRC_STRIDE:
			puint[0] = dma_get_src_stride(pbyte[1]);
			break;

		case DMA_CMD_GET_DST_STRIDE:
			puint[0] = dma_get_dst_stride(pbyte[1]);
			break;

		case DMA_CMD_GET_NEXTCB_ADDR:
			puint[0] = dma_get_next_ctrlblock_addr(pbyte[1]);
			break;

		case DMA_CMD_DEBUG_GET_IS_TYPE_LITE:
			puint[0] = dma_debug_is_type_lite(pbyte[1]);
			break;

		case DMA_CMD_DEBUG_GET_VERSION:
			puint[0] = dma_debug_get_version(pbyte[1]);
			break;

		case DMA_CMD_DEBUG_GET_STATE:
			puint[0] = dma_debug_get_state(pbyte[1]);
			break;

		case DMA_CMD_DEBUG_GET_ID:
			puint[0] = dma_debug_get_id(pbyte[1]);
			break;

		case DMA_CMD_DEBUG_GET_OSTD_WRITES_COUNTER:
			puint[0] = dma_debug_get_ostd_writes_counter(pbyte[1]);
			break;

		case DMA_CMD_DEBUG_GET_READ_ERROR:
			puint[0] = dma_debug_get_read_error(pbyte[1]);
			break;

		case DMA_CMD_DEBUG_GET_FIFO_ERROR:
			puint[0] = dma_debug_get_fifo_error(pbyte[1]);
			break;

		case DMA_CMD_DEBUG_GET_READLASTNOTSET_ERROR:
			puint[0] = dma_debug_get_read_last_not_set_error(pbyte[1]);
			break;
	}

	pbyte[0] = DMA_CMD_KERNEL_RESPONSE;
	return size;
}

static const struct proc_ops dma_proc_ops = {
	.proc_read = dma_mod_usrread,
	.proc_write = dma_mod_usrwrite
};

static int __init driver_enable(void)
{
	dma_std_mapping_group = (unsigned int**) vmalloc(7*sizeof(unsigned int*));
	dma_lite_mapping_group = (unsigned int**) vmalloc(8*sizeof(unsigned int*));

	dma_std_mapping_group[0] = (unsigned int*) ioremap(DMA_STD_CH0_BASE_ADDR, DMA_MAPPING_SIZE_BYTES);
	dma_std_mapping_group[1] = (unsigned int*) ioremap(DMA_STD_CH1_BASE_ADDR, DMA_MAPPING_SIZE_BYTES);
	dma_std_mapping_group[2] = (unsigned int*) ioremap(DMA_STD_CH2_BASE_ADDR, DMA_MAPPING_SIZE_BYTES);
	dma_std_mapping_group[3] = (unsigned int*) ioremap(DMA_STD_CH3_BASE_ADDR, DMA_MAPPING_SIZE_BYTES);
	dma_std_mapping_group[4] = (unsigned int*) ioremap(DMA_STD_CH4_BASE_ADDR, DMA_MAPPING_SIZE_BYTES);
	dma_std_mapping_group[5] = (unsigned int*) ioremap(DMA_STD_CH5_BASE_ADDR, DMA_MAPPING_SIZE_BYTES);
	dma_std_mapping_group[6] = (unsigned int*) ioremap(DMA_STD_CH6_BASE_ADDR, DMA_MAPPING_SIZE_BYTES);

	dma_lite_mapping_group[0] = (unsigned int*) ioremap(DMA_LITE_CH0_BASE_ADDR, DMA_MAPPING_SIZE_BYTES);
	dma_lite_mapping_group[1] = (unsigned int*) ioremap(DMA_LITE_CH1_BASE_ADDR, DMA_MAPPING_SIZE_BYTES);
	dma_lite_mapping_group[2] = (unsigned int*) ioremap(DMA_LITE_CH2_BASE_ADDR, DMA_MAPPING_SIZE_BYTES);
	dma_lite_mapping_group[3] = (unsigned int*) ioremap(DMA_LITE_CH3_BASE_ADDR, DMA_MAPPING_SIZE_BYTES);
	dma_lite_mapping_group[4] = (unsigned int*) ioremap(DMA_LITE_CH4_BASE_ADDR, DMA_MAPPING_SIZE_BYTES);
	dma_lite_mapping_group[5] = (unsigned int*) ioremap(DMA_LITE_CH5_BASE_ADDR, DMA_MAPPING_SIZE_BYTES);
	dma_lite_mapping_group[6] = (unsigned int*) ioremap(DMA_LITE_CH6_BASE_ADDR, DMA_MAPPING_SIZE_BYTES);
	dma_lite_mapping_group[7] = (unsigned int*) ioremap(DMA_LITE_CH7_BASE_ADDR, DMA_MAPPING_SIZE_BYTES);

	dma_intr_status_reg = (unsigned int*) ioremap(DMA_INTR_STATUS_REG_ADDR, sizeof(unsigned int));
	dma_channel_enable_reg = (unsigned int*) ioremap(DMA_ENABLE_CH_REG_ADDR, sizeof(unsigned int));

	if(dma_std_mapping_group[0] == NULL)
	{
		printk("DMA: Error mapping DMA STD CH0 addr\n");
		vfree(dma_std_mapping_group);
		vfree(dma_lite_mapping_group);
		return -1;
	}
	if(dma_std_mapping_group[1] == NULL)
	{
		printk("DMA: Error mapping DMA STD CH1 addr\n");
		vfree(dma_std_mapping_group);
		vfree(dma_lite_mapping_group);
		return -1;
	}
	if(dma_std_mapping_group[2] == NULL)
	{
		printk("DMA: Error mapping DMA STD CH2 addr\n");
		vfree(dma_std_mapping_group);
		vfree(dma_lite_mapping_group);
		return -1;
	}
	if(dma_std_mapping_group[3] == NULL)
	{
		printk("DMA: Error mapping DMA STD CH3 addr\n");
		vfree(dma_std_mapping_group);
		vfree(dma_lite_mapping_group);
		return -1;
	}
	if(dma_std_mapping_group[4] == NULL)
	{
		printk("DMA: Error mapping DMA STD CH4 addr\n");
		vfree(dma_std_mapping_group);
		vfree(dma_lite_mapping_group);
		return -1;
	}
	if(dma_std_mapping_group[5] == NULL)
	{
		printk("DMA: Error mapping DMA STD CH5 addr\n");
		vfree(dma_std_mapping_group);
		vfree(dma_lite_mapping_group);
		return -1;
	}
	if(dma_std_mapping_group[6] == NULL)
	{
		printk("DMA: Error mapping DMA STD CH6 addr\n");
		vfree(dma_std_mapping_group);
		vfree(dma_lite_mapping_group);
		return -1;
	}

	if(dma_lite_mapping_group[0] == NULL)
	{
		printk("DMA: Error mapping DMA LITE CH0 addr\n");
		vfree(dma_std_mapping_group);
		vfree(dma_lite_mapping_group);
		return -1;
	}
	if(dma_lite_mapping_group[1] == NULL)
	{
		printk("DMA: Error mapping DMA LITE CH1 addr\n");
		vfree(dma_std_mapping_group);
		vfree(dma_lite_mapping_group);
		return -1;
	}
	if(dma_lite_mapping_group[2] == NULL)
	{
		printk("DMA: Error mapping DMA LITE CH2 addr\n");
		vfree(dma_std_mapping_group);
		vfree(dma_lite_mapping_group);
		return -1;
	}
	if(dma_lite_mapping_group[3] == NULL)
	{
		printk("DMA: Error mapping DMA LITE CH3 addr\n");
		vfree(dma_std_mapping_group);
		vfree(dma_lite_mapping_group);
		return -1;
	}
	if(dma_lite_mapping_group[4] == NULL)
	{
		printk("DMA: Error mapping DMA LITE CH4 addr\n");
		vfree(dma_std_mapping_group);
		vfree(dma_lite_mapping_group);
		return -1;
	}
	if(dma_lite_mapping_group[5] == NULL)
	{
		printk("DMA: Error mapping DMA LITE CH5 addr\n");
		vfree(dma_std_mapping_group);
		vfree(dma_lite_mapping_group);
		return -1;
	}
	if(dma_lite_mapping_group[6] == NULL)
	{
		printk("DMA: Error mapping DMA LITE CH6 addr\n");
		vfree(dma_std_mapping_group);
		vfree(dma_lite_mapping_group);
		return -1;
	}
	if(dma_lite_mapping_group[7] == NULL)
	{
		printk("DMA: Error mapping DMA LITE CH7 addr\n");
		vfree(dma_std_mapping_group);
		vfree(dma_lite_mapping_group);
		return -1;
	}

	if(dma_intr_status_reg == NULL)
	{
		printk("DMA: Error mapping DMA INTR STATUS addr\n");
		vfree(dma_std_mapping_group);
		vfree(dma_lite_mapping_group);
		return -1;
	}
	if(dma_channel_enable_reg == NULL)
	{
		printk("DMA: Error mapping DMA CHANNEL ENABLE addr\n");
		vfree(dma_std_mapping_group);
		vfree(dma_lite_mapping_group);
		return -1;
	}

	dma_proc = proc_create("DMA_Ctrl", 0x1B6, NULL, &dma_proc_ops);
	if(dma_proc == NULL)
	{
		printk("DMA: Error creating proc file\n");
		vfree(dma_std_mapping_group);
		vfree(dma_lite_mapping_group);
		return -1;
	}

	dma_data_io = vmalloc(DMA_DATAIO_SIZE_BYTES);
	printk("DMA Control Driver Enabled\n");
	return 0;
}

static void __exit driver_disable(void)
{
	unsigned int n = 0;
	while(n < 7)
	{
		iounmap(dma_std_mapping_group[n]);
		n++;
	}

	n = 0;
	while(n < 8)
	{
		iounmap(dma_lite_mapping_group[n]);
		n++;
	}

	iounmap(dma_intr_status_reg);
	iounmap(dma_channel_enable_reg);

	vfree(dma_std_mapping_group);
	vfree(dma_lite_mapping_group);

	proc_remove(dma_proc);
	vfree(dma_data_io);

	printk("DMA Control Driver Disabled\n");
	return;
}

module_init(driver_enable);
module_exit(driver_disable);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rafael Sabe");
MODULE_DESCRIPTION("Driver for BCM2837 DMA Control");
