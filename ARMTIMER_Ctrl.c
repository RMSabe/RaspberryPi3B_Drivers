#include "ARMTIMER_Ctrl.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>

/*
"ARMTIMER_CTRL_WAIT_KERNEL_RESPONSE"
If defined, application will call kernel and wait for response before proceeding.
Else, application will wait a specific time (defined in ARMTIMER_CTRL_WAIT_TIME_US) is microseconds before proceeding.
*/
#define ARMTIMER_CTRL_WAIT_KERNEL_RESPONSE

#define ARMTIMER_CTRL_PROC_FILE_DIR "/proc/ARMTIMER_Ctrl"
#define ARMTIMER_CTRL_WAIT_TIME_US 1

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

int armtimer_proc_fd = -1;
void *armtimer_data_io = NULL;

void armtimer_ctrl_wait(void)
{
	clock_t start_time = clock();
	while(clock() < (start_time + ARMTIMER_CTRL_WAIT_TIME_US));
	return;
}

bool armtimer_is_active(void)
{
	return (armtimer_proc_fd >= 0);
}

bool armtimer_init(void)
{
	if(armtimer_is_active()) return true;

	armtimer_proc_fd = open(ARMTIMER_CTRL_PROC_FILE_DIR, O_RDWR);
	if(armtimer_proc_fd < 0) return false;

	armtimer_data_io = malloc(ARMTIMER_DATAIO_SIZE_BYTES);
	return true;
}

#ifdef ARMTIMER_CTRL_WAIT_KERNEL_RESPONSE
void armtimer_call_kernel(void)
{
	uint8_t *pbyte = (uint8_t*) armtimer_data_io;
	write(armtimer_proc_fd, armtimer_data_io, ARMTIMER_DATAIO_SIZE_BYTES);

	do{
		read(armtimer_proc_fd, armtimer_data_io, ARMTIMER_DATAIO_SIZE_BYTES);
	}while(pbyte[0] != ARMTIMER_CMD_KERNEL_RESPONSE);

	return;
}
#else
void armtimer_call_kernel(void)
{
	write(armtimer_proc_fd, armtimer_data_io, ARMTIMER_DATAIO_SIZE_BYTES);
	armtimer_ctrl_wait();
	read(armtimer_proc_fd, armtimer_data_io, ARMTIMER_DATAIO_SIZE_BYTES);
	return;
}
#endif

void armtimer_set_load_value(uint32_t value)
{
	uint8_t *pbyte = (uint8_t*) armtimer_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[1];
	pbyte[0] = ARMTIMER_CMD_SET_LOAD_VALUE;
	puint[0] = value;

	armtimer_call_kernel();
	return;
}

uint32_t armtimer_get_load_value(void)
{
	uint8_t *pbyte = (uint8_t*) armtimer_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[1];
	pbyte[0] = ARMTIMER_CMD_GET_LOAD_VALUE;

	armtimer_call_kernel();
	return puint[0];
}

uint32_t armtimer_get_countdown_value(void)
{
	uint8_t *pbyte = (uint8_t*) armtimer_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[1];
	pbyte[0] = ARMTIMER_CMD_GET_COUNTDOWN_VALUE;

	armtimer_call_kernel();
	return puint[0];
}

void armtimer_set_freerun_counter_prescale(uint32_t prescale)
{
	uint8_t *pbyte = (uint8_t*) armtimer_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[1];
	pbyte[0] = ARMTIMER_CMD_SET_FREERUN_COUNTER_PRESCALE;
	puint[0] = prescale;

	armtimer_call_kernel();
	return;
}

uint32_t armtimer_get_freerun_counter_prescale(void)
{
	uint8_t *pbyte = (uint8_t*) armtimer_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[1];
	pbyte[0] = ARMTIMER_CMD_GET_FREERUN_COUNTER_PRESCALE;

	armtimer_call_kernel();
	return puint[0];
}

void armtimer_enable_freerun_counter(bool enable)
{
	uint8_t *pbyte = (uint8_t*) armtimer_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[1];
	pbyte[0] = ARMTIMER_CMD_SET_ENABLE_FREERUN_COUNTER;
	puint[0] = enable;

	armtimer_call_kernel();
	return;
}

bool armtimer_freerun_counter_is_enabled(void)
{
	uint8_t *pbyte = (uint8_t*) armtimer_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[1];
	pbyte[0] = ARMTIMER_CMD_GET_ENABLE_FREERUN_COUNTER;

	armtimer_call_kernel();
	return (puint[0] & 0x00000001);
}

void armtimer_enable_debug_halt_timer(bool enable)
{
	uint8_t *pbyte = (uint8_t*) armtimer_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[1];
	pbyte[0] = ARMTIMER_CMD_SET_ENABLE_DEBUG_HALT_TIMER;
	puint[0] = enable;

	armtimer_call_kernel();
	return;
}

bool armtimer_debug_halt_timer_is_enabled(void)
{
	uint8_t *pbyte = (uint8_t*) armtimer_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[1];
	pbyte[0] = ARMTIMER_CMD_GET_ENABLE_DEBUG_HALT_TIMER;

	armtimer_call_kernel();
	return (puint[0] & 0x00000001);
}

void armtimer_enable_timer(bool enable)
{
	uint8_t *pbyte = (uint8_t*) armtimer_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[1];
	pbyte[0] = ARMTIMER_CMD_SET_ENABLE_TIMER;
	puint[0] = enable;

	armtimer_call_kernel();
	return;
}

bool armtimer_timer_is_enabled(void)
{
	uint8_t *pbyte = (uint8_t*) armtimer_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[1];
	pbyte[0] = ARMTIMER_CMD_GET_ENABLE_TIMER;

	armtimer_call_kernel();
	return (puint[0] & 0x00000001);
}

void armtimer_enable_intr(bool enable)
{
	uint8_t *pbyte = (uint8_t*) armtimer_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[1];
	pbyte[0] = ARMTIMER_CMD_SET_ENABLE_INTR;
	puint[0] = enable;

	armtimer_call_kernel();
	return;
}

bool armtimer_intr_is_enabled(void)
{
	uint8_t *pbyte = (uint8_t*) armtimer_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[1];
	pbyte[0] = ARMTIMER_CMD_GET_ENABLE_INTR;

	armtimer_call_kernel();
	return (puint[0] & 0x00000001);
}

void armtimer_set_timer_prescale(uint32_t prescale)
{
	uint8_t *pbyte = (uint8_t*) armtimer_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[1];
	pbyte[0] = ARMTIMER_CMD_SET_TIMER_PRESCALE;
	puint[0] = prescale;

	armtimer_call_kernel();
	return;
}

uint32_t armtimer_get_timer_prescale(void)
{
	uint8_t *pbyte = (uint8_t*) armtimer_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[1];
	pbyte[0] = ARMTIMER_CMD_GET_TIMER_PRESCALE;

	armtimer_call_kernel();
	if(puint[0] == 3) return 0;
	return puint[0];
}

void armtimer_set_countsize_bits(bool countsize)
{
	uint8_t *pbyte = (uint8_t*) armtimer_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[1];
	pbyte[0] = ARMTIMER_CMD_SET_COUNTSIZE_BITS;
	puint[0] = countsize;

	armtimer_call_kernel();
	return;
}

bool armtimer_get_countsize_bits(void)
{
	uint8_t *pbyte = (uint8_t*) armtimer_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[1];
	pbyte[0] = ARMTIMER_CMD_GET_COUNTSIZE_BITS;

	armtimer_call_kernel();
	return (puint[0] & 0x00000001);
}

void armtimer_clear_intr_flags(void)
{
	uint8_t *pbyte = (uint8_t*) armtimer_data_io;
	pbyte[0] = ARMTIMER_CMD_CLEAR_INTR_FLAGS;
	
	armtimer_call_kernel();
	return;
}

bool armtimer_get_raw_intr_status(void)
{
	uint8_t *pbyte = (uint8_t*) armtimer_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[1];
	pbyte[0] = ARMTIMER_CMD_GET_RAW_INTR_STATUS;

	armtimer_call_kernel();
	return (puint[0] & 0x00000001);
}

bool armtimer_get_masked_intr_status(void)
{
	uint8_t *pbyte = (uint8_t*) armtimer_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[1];
	pbyte[0] = ARMTIMER_CMD_GET_MASKED_INTR_STATUS;

	armtimer_call_kernel();
	return (puint[0] & 0x00000001);
}

void armtimer_set_reload_value(uint32_t value)
{
	uint8_t *pbyte = (uint8_t*) armtimer_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[1];
	pbyte[0] = ARMTIMER_CMD_SET_RELOAD_VALUE;
	puint[0] = value;

	armtimer_call_kernel();
	return;
}

uint32_t armtimer_get_reload_value(void)
{
	uint8_t *pbyte = (uint8_t*) armtimer_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[1];
	pbyte[0] = ARMTIMER_CMD_GET_RELOAD_VALUE;

	armtimer_call_kernel();
	return puint[0];
}

void armtimer_set_predivider_value(uint32_t value)
{
	uint8_t *pbyte = (uint8_t*) armtimer_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[1];
	pbyte[0] = ARMTIMER_CMD_SET_PREDIV_VALUE;
	puint[0] = value;

	armtimer_call_kernel();
	return;
}

uint32_t armtimer_get_predivider_value(void)
{
	uint8_t *pbyte = (uint8_t*) armtimer_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[1];
	pbyte[0] = ARMTIMER_CMD_GET_PREDIV_VALUE;

	armtimer_call_kernel();
	return puint[0];
}

uint32_t armtimer_get_freerun_counter_value(void)
{
	uint8_t *pbyte = (uint8_t*) armtimer_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[1];
	pbyte[0] = ARMTIMER_CMD_GET_FREERUN_COUNTER_VALUE;

	armtimer_call_kernel();
	return puint[0];
}

