#include "SYSTIMER_Ctrl.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>

#define SYSTIMER_CTRL_WAIT_KERNEL_RESPONSE

#define SYSTIMER_CTRL_PROC_FILE_DIR "/proc/SYSTIMER_Ctrl"
#define SYSTIMER_CTRL_WAIT_TIME_US 1

#define SYSTIMER_DATAIO_SIZE_BYTES 6

#define SYSTIMER_CMD_GET_TIMER_MATCH_OCCURRED 0
#define SYSTIMER_CMD_GET_COUNTER_VALUE_L32 1
#define SYSTIMER_CMD_GET_COUNTER_VALUE_H32 2
#define SYSTIMER_CMD_SET_TIMER_MATCH_VALUE 3
#define SYSTIMER_CMD_GET_TIMER_MATCH_VALUE 4

#define SYSTIMER_CMD_KERNEL_RESPONSE 0xFF

int systimer_proc_fd = -1;
void *systimer_data_io = NULL;

void systimer_ctrl_wait(void)
{
	clock_t start_time = clock();
	while(clock() < (start_time + SYSTIMER_CTRL_WAIT_TIME_US));
	return;
}

bool systimer_is_active(void)
{
	return (systimer_proc_fd >= 0);
}

bool systimer_init(void)
{
	if(systimer_is_active()) return true;

	systimer_proc_fd = open(SYSTIMER_CTRL_PROC_FILE_DIR, O_RDWR);
	if(systimer_proc_fd < 0) return false;

	systimer_data_io = malloc(SYSTIMER_DATAIO_SIZE_BYTES);
	return true;
}

#ifdef SYSTIMER_CTRL_WAIT_KERNEL_RESPONSE
void systimer_call_kernel(void)
{
	uint8_t *pbyte = (uint8_t*) systimer_data_io;
	write(systimer_proc_fd, systimer_data_io, SYSTIMER_DATAIO_SIZE_BYTES);

	do{
		read(systimer_proc_fd, systimer_data_io, SYSTIMER_DATAIO_SIZE_BYTES);
	}while(pbyte[0] != SYSTIMER_CMD_KERNEL_RESPONSE);

	return;
}
#else
void systimer_call_kernel(void)
{
	write(systimer_proc_fd, systimer_data_io, SYSTIMER_DATAIO_SIZE_BYTES);
	systimer_ctrl_wait();
	read(systimer_proc_fd, systimer_data_io, SYSTIMER_DATAIO_SIZE_BYTES);
	return;
}
#endif

bool systimer_timer_match_occurred(uint8_t timer_num)
{
	uint8_t *pbyte = (uint8_t*) systimer_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = SYSTIMER_CMD_GET_TIMER_MATCH_OCCURRED;
	pbyte[1] = timer_num;

	systimer_call_kernel();
	return (puint[0] & 0x00000001);
}

uint32_t systimer_get_counter_value_l32(void)
{
	uint8_t *pbyte = (uint8_t*) systimer_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = SYSTIMER_CMD_GET_COUNTER_VALUE_L32;
	
	systimer_call_kernel();
	return puint[0];
}

uint32_t systimer_get_counter_value_h32(void)
{
	uint8_t *pbyte = (uint8_t*) systimer_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = SYSTIMER_CMD_GET_COUNTER_VALUE_H32;

	systimer_call_kernel();
	return puint[0];
}

uint64_t systimer_get_counter_value_full(void)
{
	uint32_t l32 = systimer_get_counter_value_l32();
	uint32_t h32 = systimer_get_counter_value_h32();
	uint64_t ulong = 0;

	uint32_t *p = (uint32_t*) &ulong;
	p[0] = l32;
	p[1] = h32;
	return ulong;
}

void systimer_set_timer_compare_match_value(uint8_t timer_num, uint32_t value)
{
	uint8_t *pbyte = (uint8_t*) systimer_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = SYSTIMER_CMD_SET_TIMER_MATCH_VALUE;
	pbyte[1] = timer_num;
	puint[0] = value;

	systimer_call_kernel();
	return;
}

uint32_t systimer_get_timer_compare_match_value(uint8_t timer_num)
{
	uint8_t *pbyte = (uint8_t*) systimer_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[2];
	pbyte[0] = SYSTIMER_CMD_GET_TIMER_MATCH_VALUE;
	pbyte[1] = timer_num;

	systimer_call_kernel();
	return puint[0];
}

