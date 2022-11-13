#include "DMA_Ctrl.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "MMU32_usr.h" //Use this for aarch32 GNU-Linux
//#include "MMU64_usr.h" //Use this for aarch64 GNU-Linux

/*
"DMA_CTRL_WAIT_KERNEL_RESPONSE"
If defined, application will call kernel and wait for response before proceeding.
Else, application will wait a specific time (defined in DMA_CTRL_WAIT_TIME_US) is microseconds before proceeding.
*/
#define DMA_CTRL_WAIT_KERNEL_RESPONSE

#define DMA_CTRL_PROC_FILE_DIR "/proc/DMA_Ctrl"
#define DMA_CTRL_WAIT_TIME_US 1

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

int dma_proc_fd = -1;
void *dma_data_io = NULL;

void dma_ctrl_wait(void)
{
	clock_t start_time = clock();
	while(clock() < (start_time + DMA_CTRL_WAIT_TIME_US));
	return;
}

bool dma_is_active(void)
{
	return (dma_proc_fd >= 0);
}

bool dma_init(void)
{
	if(dma_is_active()) return true;

	if(!mmu_is_active()) if(!mmu_init()) return false;

	dma_proc_fd = open(DMA_CTRL_PROC_FILE_DIR, O_RDWR);
	if(dma_proc_fd < 0) return false;

	dma_data_io = malloc(DMA_DATAIO_SIZE_BYTES);
	return true;
}

#ifdef DMA_CTRL_WAIT_KERNEL_RESPONSE
void dma_call_kernel(void)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	write(dma_proc_fd, dma_data_io, DMA_DATAIO_SIZE_BYTES);

	do{
		read(dma_proc_fd, dma_data_io, DMA_DATAIO_SIZE_BYTES);
	}while(pbyte[0] != DMA_CMD_KERNEL_RESPONSE);

	return;
}
#else
void dma_call_kernel(void)
{
	write(dma_proc_fd, dma_data_io, DMA_DATAIO_SIZE_BYTES);
	dma_ctrl_wait();
	read(dma_proc_fd, dma_data_io, DMA_DATAIO_SIZE_BYTES);
	return;
}
#endif

bool dma_get_type(uint8_t dma_ctrl)
{
	if(dma_ctrl > DMA_LITE_CH7) return false;
	return (dma_ctrl > DMA_STD_CH6);
}

void dma_reset_ctrlblock(dma_ctrlblock_t *p_ctrlblock)
{
	memset(p_ctrlblock, 0, sizeof(dma_ctrlblock_t));
	return;
}

void dma_enable_ctrl(uint8_t dma_ctrl, bool enable)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_SET_ENABLE_CTRL;
	pbyte[1] = dma_ctrl;
	puint[0] = enable;

	dma_call_kernel();
	return;
}

bool dma_ctrl_is_enabled(uint8_t dma_ctrl)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_GET_ENABLE_CTRL;
	pbyte[1] = dma_ctrl;

	dma_call_kernel();
	return (puint[0] & 0x00000001);
}

bool dma_get_channel_intr_status(uint8_t dma_ctrl)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_GET_CHANNEL_INTR_STATUS;
	pbyte[1] = dma_ctrl;

	dma_call_kernel();
	return (puint[0] & 0x00000001);
}

uint32_t dma_get_full_intr_status(void)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_GET_FULL_INTR_STATUS;

	dma_call_kernel();
	return puint[0];
}

void dma_set_ctrlblock_addr_phys(uint8_t dma_ctrl, uint32_t addr)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_SET_CTRLBLOCK_ADDR;
	pbyte[1] = dma_ctrl;
	puint[0] = addr;

	dma_call_kernel();
	return;
}

uint32_t dma_get_ctrlblock_addr_phys(uint8_t dma_ctrl)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_GET_CTRLBLOCK_ADDR;
	pbyte[1] = dma_ctrl;

	dma_call_kernel();
	return puint[0];
}

void dma_set_ctrlblock_addr_virt(uint8_t dma_ctrl, dma_ctrlblock_t *p_ctrlblock)
{
	dma_set_ctrlblock_addr_phys(dma_ctrl, (uint32_t) mmu_get_phys_from_virt(p_ctrlblock));
	return;
}

void dma_set_transfer_active(uint8_t dma_ctrl, bool active)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_SET_TRANSFER_ACTIVE;
	pbyte[1] = dma_ctrl;
	puint[0] = active;

	dma_call_kernel();
	return;
}

bool dma_get_transfer_active(uint8_t dma_ctrl)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_GET_TRANSFER_ACTIVE;
	pbyte[1] = dma_ctrl;

	dma_call_kernel();
	return (puint[0] & 0x00000001);
}

bool dma_transfer_done(uint8_t dma_ctrl)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_GET_TRANSFER_DONE;
	pbyte[1] = dma_ctrl;

	dma_call_kernel();
	return (puint[0] & 0x00000001);
}

bool dma_get_intr_status(uint8_t dma_ctrl)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_GET_INTR_STATUS;
	pbyte[1] = dma_ctrl;

	dma_call_kernel();
	return (puint[0] & 0x00000001);
}

bool dma_is_requesting_data(uint8_t dma_ctrl)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_GET_IS_REQUESTING_DATA;
	pbyte[1] = dma_ctrl;

	dma_call_kernel();
	return (puint[0] & 0x00000001);
}

bool dma_is_paused(uint8_t dma_ctrl)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_GET_IS_PAUSED;
	pbyte[1] = dma_ctrl;

	dma_call_kernel();
	return (puint[0] & 0x00000001);
}

bool dma_is_paused_by_inactive_dreq(uint8_t dma_ctrl)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_GET_IS_PAUSED_BY_INACTIVE_DREQ;
	pbyte[1] = dma_ctrl;

	dma_call_kernel();
	return (puint[0] & 0x00000001);
}

bool dma_is_waiting_ostd_writes(uint8_t dma_ctrl)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_GET_IS_WAITING_OSTD_WRITES;
	pbyte[1] = dma_ctrl;

	dma_call_kernel();
	return (puint[0] & 0x00000001);
}

bool dma_error_occurred(uint8_t dma_ctrl)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_GET_ERROR_OCCURRED;
	pbyte[1] = dma_ctrl;

	dma_call_kernel();
	return (puint[0] & 0x00000001);
}

void dma_set_priority(uint8_t dma_ctrl, uint32_t priority)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_SET_PRIORITY;
	pbyte[1] = dma_ctrl;
	puint[0] = priority;

	dma_call_kernel();
	return;
}

uint32_t dma_get_priority(uint8_t dma_ctrl)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_GET_PRIORITY;
	pbyte[1] = dma_ctrl;

	dma_call_kernel();
	return puint[0];
}

void dma_set_panic_priority(uint8_t dma_ctrl, uint32_t priority)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_SET_PANIC_PRIORITY;
	pbyte[1] = dma_ctrl;
	puint[0] = priority;

	dma_call_kernel();
	return;
}

uint32_t dma_get_panic_priority(uint8_t dma_ctrl)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_GET_PANIC_PRIORITY;
	pbyte[1] = dma_ctrl;

	dma_call_kernel();
	return puint[0];
}

void dma_enable_wait_ostd_writes(uint8_t dma_ctrl, bool enable)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_SET_ENABLE_WAIT_OSTD_WRITES;
	pbyte[1] = dma_ctrl;
	puint[0] = enable;

	dma_call_kernel();
	return;
}

bool dma_wait_ostd_writes_is_enabled(uint8_t dma_ctrl)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_GET_ENABLE_WAIT_OSTD_WRITES;
	pbyte[1] = dma_ctrl;

	dma_call_kernel();
	return (puint[0] & 0x00000001);
}

void dma_disable_debug_pause(uint8_t dma_ctrl, bool disable)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_SET_DISABLE_DEBUG_PAUSE;
	pbyte[1] = dma_ctrl;
	puint[0] = disable;

	dma_call_kernel();
	return;
}

bool dma_debug_pause_is_disabled(uint8_t dma_ctrl)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_GET_DISABLE_DEBUG_PAUSE;
	pbyte[1] = dma_ctrl;

	dma_call_kernel();
	return (puint[0] & 0x00000001);
}

void dma_abort(uint8_t dma_ctrl)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	pbyte[0] = DMA_CMD_ABORT;
	pbyte[1] = dma_ctrl;

	dma_call_kernel();
	return;
}

void dma_reset(uint8_t dma_ctrl)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	pbyte[0] = DMA_CMD_RESET;
	pbyte[1] = dma_ctrl;

	dma_call_kernel();
	return;
}

bool dma_wide_bursts_is_disabled(uint8_t dma_ctrl)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_GET_DISABLE_WIDE_BURSTS;
	pbyte[1] = dma_ctrl;
	
	dma_call_kernel();
	return (puint[0] & 0x00000001);
}

uint32_t dma_get_wait_cycles(uint8_t dma_ctrl)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_GET_WAIT_CYCLES;
	pbyte[1] = dma_ctrl;

	dma_call_kernel();
	return puint[0];
}

uint32_t dma_get_permap(uint8_t dma_ctrl)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_GET_PERMAP;
	pbyte[1] = dma_ctrl;

	dma_call_kernel();
	return puint[0];
}

uint32_t dma_get_burst_length(uint8_t dma_ctrl)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_GET_BURST_LENGTH;
	pbyte[1] = dma_ctrl;

	dma_call_kernel();
	return puint[0];
}

bool dma_ignore_src_reads_is_enabled(uint8_t dma_ctrl)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_GET_ENABLE_IGNORE_SRC_READS;
	pbyte[1] = dma_ctrl;

	dma_call_kernel();
	return (puint[0] & 0x00000001);
}

bool dma_get_dreq_calls_src_reads(uint8_t dma_ctrl)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_GET_DREQ_CALLS_SRC_READS;
	pbyte[1] = dma_ctrl;

	dma_call_kernel();
	return (puint[0] & 0x00000001);
}

bool dma_src_read_128bit_width_is_enabled(uint8_t dma_ctrl)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_GET_ENABLE_SRC_READ_128BIT_WIDTH;
	pbyte[1] = dma_ctrl;

	dma_call_kernel();
	return (puint[0] & 0x00000001);
}

bool dma_src_addr_inc_is_enabled(uint8_t dma_ctrl)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_GET_ENABLE_SRC_ADDR_INC;
	pbyte[1] = dma_ctrl;

	dma_call_kernel();
	return (puint[0] & 0x00000001);
}

bool dma_ignore_dst_writes_is_enabled(uint8_t dma_ctrl)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_GET_ENABLE_IGNORE_DST_WRITES;
	pbyte[1] = dma_ctrl;

	dma_call_kernel();
	return (puint[0] & 0x00000001);
}

bool dma_get_dreq_calls_dst_writes(uint8_t dma_ctrl)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_GET_DREQ_CALLS_DST_WRITES;
	pbyte[1] = dma_ctrl;

	dma_call_kernel();
	return (puint[0] & 0x00000001);
}

bool dma_dst_write_128bit_width_is_enabled(uint8_t dma_ctrl)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_GET_ENABLE_DST_WRITE_128BIT_WIDTH;
	pbyte[1] = dma_ctrl;

	dma_call_kernel();
	return (puint[0] & 0x00000001);
}

bool dma_dst_addr_inc_is_enabled(uint8_t dma_ctrl)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_GET_ENABLE_DST_ADDR_INC;
	pbyte[1] = dma_ctrl;

	dma_call_kernel();
	return (puint[0] & 0x00000001);
}

bool dma_wait_write_response_is_enabled(uint8_t dma_ctrl)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_GET_ENABLE_WAIT_WRITE_RESPONSE;
	pbyte[1] = dma_ctrl;

	dma_call_kernel();
	return (puint[0] & 0x00000001);
}

bool dma_tdmode_is_enabled(uint8_t dma_ctrl)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_GET_ENABLE_TDMODE;
	pbyte[1] = dma_ctrl;

	dma_call_kernel();
	return (puint[0] & 0x00000001);
}

bool dma_intr_is_enabled(uint8_t dma_ctrl)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_GET_ENABLE_INTR;
	pbyte[1] = dma_ctrl;

	dma_call_kernel();
	return (puint[0] & 0x00000001);
}

uint32_t dma_get_src_addr_phys(uint8_t dma_ctrl)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_GET_SRC_ADDR;
	pbyte[1] = dma_ctrl;

	dma_call_kernel();
	return puint[0];
}

uint32_t dma_get_dst_addr_phys(uint8_t dma_ctrl)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_GET_DST_ADDR;
	pbyte[1] = dma_ctrl;

	dma_call_kernel();
	return puint[0];
}

uint32_t dma_get_transfer_length_bytes(uint8_t dma_ctrl)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_GET_TRANSFER_LENGTH_BYTES;
	pbyte[1] = dma_ctrl;

	dma_call_kernel();
	return puint[0];
}

uint32_t dma_get_transfer_length_ext(uint8_t dma_ctrl)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_GET_TRANSFER_LENGTH_EXT;
	pbyte[1] = dma_ctrl;

	dma_call_kernel();
	return puint[0];
}

uint32_t dma_get_src_stride(uint8_t dma_ctrl)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_GET_SRC_STRIDE;
	pbyte[1] = dma_ctrl;

	dma_call_kernel();
	return puint[0];
}

uint32_t dma_get_dst_stride(uint8_t dma_ctrl)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_GET_DST_STRIDE;
	pbyte[1] = dma_ctrl;

	dma_call_kernel();
	return puint[0];
}

uint32_t dma_get_next_ctrlblock_addr_phys(uint8_t dma_ctrl)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_GET_NEXTCB_ADDR;
	pbyte[1] = dma_ctrl;

	dma_call_kernel();
	return puint[0];
}

bool dma_debug_is_type_lite(uint8_t dma_ctrl)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_DEBUG_GET_IS_TYPE_LITE;
	pbyte[1] = dma_ctrl;

	dma_call_kernel();
	return (puint[0] & 0x00000001);
}

uint32_t dma_debug_get_version(uint8_t dma_ctrl)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_DEBUG_GET_VERSION;
	pbyte[1] = dma_ctrl;

	dma_call_kernel();
	return puint[0];
}

uint32_t dma_debug_get_state(uint8_t dma_ctrl)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_DEBUG_GET_STATE;
	pbyte[1] = dma_ctrl;

	dma_call_kernel();
	return puint[0];
}

uint32_t dma_debug_get_id(uint8_t dma_ctrl)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_DEBUG_GET_ID;
	pbyte[1] = dma_ctrl;

	dma_call_kernel();
	return puint[0];
}

uint32_t dma_debug_get_ostd_writes_counter(uint8_t dma_ctrl)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_DEBUG_GET_OSTD_WRITES_COUNTER;
	pbyte[1] = dma_ctrl;

	dma_call_kernel();
	return puint[0];
}

bool dma_debug_get_read_error(uint8_t dma_ctrl)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_DEBUG_GET_READ_ERROR;
	pbyte[1] = dma_ctrl;

	dma_call_kernel();
	return (puint[0] & 0x00000001);
}

bool dma_debug_get_fifo_error(uint8_t dma_ctrl)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_DEBUG_GET_FIFO_ERROR;
	pbyte[1] = dma_ctrl;

	dma_call_kernel();
	return (puint[0] & 0x00000001);
}

bool dma_debug_readlast_not_set_error(uint8_t dma_ctrl)
{
	uint8_t *pbyte = (uint8_t*) dma_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = DMA_CMD_DEBUG_GET_READLASTNOTSET_ERROR;
	pbyte[1] = dma_ctrl;

	dma_call_kernel();
	return (puint[0] & 0x00000001);
}

void dma_disable_wide_bursts(dma_ctrlblock_t *p_ctrlblock, bool disable)
{
	p_ctrlblock->transfer_info &= ~(1 << 26);
	p_ctrlblock->transfer_info |= (disable << 26);
	return;
}

void dma_set_wait_cycles(dma_ctrlblock_t *p_ctrlblock, uint8_t wait_cycles)
{
	wait_cycles &= 0x1F;
	p_ctrlblock->transfer_info &= ~(0x1F << 21);
	p_ctrlblock->transfer_info |= (wait_cycles << 21);
	return;
}

void dma_set_permap(dma_ctrlblock_t *p_ctrlblock, uint8_t permap)
{
	permap &= 0x1F;
	p_ctrlblock->transfer_info &= ~(0x1F << 16);
	p_ctrlblock->transfer_info |= (permap << 16);
	return;
}

void dma_set_burst_length(dma_ctrlblock_t *p_ctrlblock, uint8_t burst_length)
{
	burst_length &= 0x0F;
	p_ctrlblock->transfer_info &= ~(0xF << 12);
	p_ctrlblock->transfer_info |= (burst_length << 12);
	return;
}

void dma_enable_ignore_src_reads(dma_ctrlblock_t *p_ctrlblock, bool enable)
{
	p_ctrlblock->transfer_info &= ~(1 << 11);
	p_ctrlblock->transfer_info |= (enable << 11);
	return;
}

void dma_set_dreq_calls_src_reads(dma_ctrlblock_t *p_ctrlblock, bool enable)
{
	p_ctrlblock->transfer_info &= ~(1 << 10);
	p_ctrlblock->transfer_info |= (enable << 10);
	return;
}

void dma_enable_src_read_128bit_width(dma_ctrlblock_t *p_ctrlblock, bool enable)
{
	p_ctrlblock->transfer_info &= ~(1 << 9);
	p_ctrlblock->transfer_info |= (enable << 9);
	return;
}

void dma_enable_src_addr_inc(dma_ctrlblock_t *p_ctrlblock, bool enable)
{
	p_ctrlblock->transfer_info &= ~(1 << 8);
	p_ctrlblock->transfer_info |= (enable << 8);
	return;
}

void dma_enable_ignore_dst_writes(dma_ctrlblock_t *p_ctrlblock, bool enable)
{
	p_ctrlblock->transfer_info &= ~(1 << 7);
	p_ctrlblock->transfer_info |= (enable << 7);
	return;
}

void dma_set_dreq_calls_dst_writes(dma_ctrlblock_t *p_ctrlblock, bool enable)
{
	p_ctrlblock->transfer_info &= ~(1 << 6);
	p_ctrlblock->transfer_info |= (enable << 6);
	return;
}

void dma_enable_dst_write_128bit_width(dma_ctrlblock_t *p_ctrlblock, bool enable)
{
	p_ctrlblock->transfer_info &= ~(1 << 5);
	p_ctrlblock->transfer_info |= (enable << 5);
	return;
}

void dma_enable_dst_addr_inc(dma_ctrlblock_t *p_ctrlblock, bool enable)
{
	p_ctrlblock->transfer_info &= ~(1 << 4);
	p_ctrlblock->transfer_info |= (enable << 4);
	return;
}

void dma_enable_wait_write_response(dma_ctrlblock_t *p_ctrlblock, bool enable)
{
	p_ctrlblock->transfer_info &= ~(1 << 3);
	p_ctrlblock->transfer_info |= (enable << 3);
	return;
}

void dma_enable_tdmode(dma_ctrlblock_t *p_ctrlblock, bool enable)
{
	p_ctrlblock->transfer_info &= ~(1 << 1);
	p_ctrlblock->transfer_info |= (enable << 1);
	return;
}

void dma_enable_intr(dma_ctrlblock_t *p_ctrlblock, bool enable)
{
	p_ctrlblock->transfer_info &= ~(1);
	p_ctrlblock->transfer_info |= (enable);
	return;
}

void dma_set_src_addr_phys(dma_ctrlblock_t *p_ctrlblock, uint32_t addr)
{
	p_ctrlblock->src_addr = addr;
	return;
}

void dma_set_src_addr_virt(dma_ctrlblock_t *p_ctrlblock, void *p)
{
	p_ctrlblock->src_addr = (uint32_t) mmu_get_phys_from_virt(p);
	return;
}

void dma_set_dst_addr_phys(dma_ctrlblock_t *p_ctrlblock, uint32_t addr)
{
	p_ctrlblock->dst_addr = addr;
	return;
}

void dma_set_dst_addr_virt(dma_ctrlblock_t *p_ctrlblock, void *p)
{
	p_ctrlblock->dst_addr = (uint32_t) mmu_get_phys_from_virt(p);
	return;
}

void dma_set_transfer_length_bytes(dma_ctrlblock_t *p_ctrlblock, uint16_t length)
{
	p_ctrlblock->transfer_length &= ~(0xFFFF);
	p_ctrlblock->transfer_length |= (length);
	return;
}

void dma_set_transfer_length_ext(dma_ctrlblock_t *p_ctrlblock, uint16_t length)
{
	length &= 0x3FFF;
	p_ctrlblock->transfer_length &= ~(0x3FFF << 16);
	p_ctrlblock->transfer_length |= (length << 16);
	return;
}

void dma_set_src_stride(dma_ctrlblock_t *p_ctrlblock, uint16_t stride)
{
	p_ctrlblock->stride &= ~(0xFFFF);
	p_ctrlblock->stride |= (stride);
	return;
}

void dma_set_dst_stride(dma_ctrlblock_t *p_ctrlblock, uint16_t stride)
{
	p_ctrlblock->stride &= ~(0xFFFF << 16);
	p_ctrlblock->stride |= (stride << 16);
	return;
}

void dma_set_next_ctrlblock_addr_phys(dma_ctrlblock_t *p_ctrlblock, uint32_t addr)
{
	p_ctrlblock->next_ctrlblock_addr = addr;
	return;
}

void dma_set_next_ctrlblock_addr_virt(dma_ctrlblock_t *p_ctrlblock, void *p)
{
	p_ctrlblock->next_ctrlblock_addr = (uint32_t) mmu_get_phys_from_virt(p);
	return;
}
