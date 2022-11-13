#include "INTR_Ctrl.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>

#define INTR_CTRL_WAIT_KERNEL_RESPONSE

#define INTR_CTRL_PROC_FILE_DIR "/proc/INTR_Ctrl"
#define INTR_CTRL_WAIT_TIME_US 1

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

int intr_proc_fd = -1;
void *intr_data_io = NULL;

void intr_ctrl_wait(void)
{
	clock_t start_time = clock();
	while(clock() < (start_time + INTR_CTRL_WAIT_TIME_US));
	return;
}

bool intr_is_active(void)
{
	return (intr_proc_fd >= 0);
}

bool intr_init(void)
{
	if(intr_is_active()) return true;

	intr_proc_fd = open(INTR_CTRL_PROC_FILE_DIR, O_RDWR);
	if(intr_proc_fd < 0) return false;

	intr_data_io = malloc(INTR_DATAIO_SIZE_BYTES);
	return true;
}

#ifdef INTR_CTRL_WAIT_KERNEL_RESPONSE
void intr_call_kernel(void)
{
	uint8_t *pbyte = (uint8_t*) intr_data_io;
	write(intr_proc_fd, intr_data_io, INTR_DATAIO_SIZE_BYTES);

	do{
		read(intr_proc_fd, intr_data_io, INTR_DATAIO_SIZE_BYTES);
	}while(pbyte[0] != INTR_CMD_KERNEL_RESPONSE);

	return;
}
#else
void intr_call_kernel(void)
{
	write(intr_proc_fd, intr_data_io, INTR_DATAIO_SIZE_BYTES);
	intr_ctrl_wait();
	read(intr_proc_fd, intr_data_io, INTR_DATAIO_SIZE_BYTES);
	return;
}
#endif

bool intr_basic_irq_occurred(uint8_t irq_id)
{
	uint8_t *pbyte = (uint8_t*) intr_data_io;
	pbyte[0] = INTR_CMD_GET_BASIC_IRQ_OCCURRED;
	pbyte[1] = irq_id;

	intr_call_kernel();
	return (pbyte[1] & 0x01);
}

bool intr_gpu_irq_occurred(uint8_t irq_id)
{
	uint8_t *pbyte = (uint8_t*) intr_data_io;
	pbyte[0] = INTR_CMD_GET_GPU_IRQ_OCCURRED;
	pbyte[1] = irq_id;

	intr_call_kernel();
	return (pbyte[1] & 0x01);
}

void intr_enable_fiq(bool enable)
{
	uint8_t *pbyte = (uint8_t*) intr_data_io;
	pbyte[0] = INTR_CMD_SET_ENABLE_FIQ;
	pbyte[1] = enable;

	intr_call_kernel();
	return;
}

bool intr_fiq_is_enabled(void)
{
	uint8_t *pbyte = (uint8_t*) intr_data_io;
	pbyte[0] = INTR_CMD_GET_ENABLE_FIQ;

	intr_call_kernel();
	return (pbyte[1] & 0x01);
}

void intr_set_fiq_src(uint8_t irq_id)
{
	uint8_t *pbyte = (uint8_t*) intr_data_io;
	pbyte[0] = INTR_CMD_SET_FIQ_SRC;
	pbyte[1] = irq_id;

	intr_call_kernel();
	return;
}

uint8_t intr_get_fiq_src(void)
{
	uint8_t *pbyte = (uint8_t*) intr_data_io;
	pbyte[0] = INTR_CMD_GET_FIQ_SRC;

	intr_call_kernel();
	return pbyte[1];
}

void intr_enable_gpu_irq(uint8_t irq_id, bool enable)
{
	uint8_t *pbyte = (uint8_t*) intr_data_io;
	pbyte[0] = INTR_CMD_SET_ENABLE_GPU_IRQ;
	pbyte[1] = irq_id;
	pbyte[2] = enable;

	intr_call_kernel();
	return;
}

void intr_enable_basic_irq(uint8_t irq_id, bool enable)
{
	uint8_t *pbyte = (uint8_t*) intr_data_io;
	pbyte[0] = INTR_CMD_SET_ENABLE_BASIC_IRQ;
	pbyte[1] = irq_id;
	pbyte[2] = enable;

	intr_call_kernel();
	return;
}

