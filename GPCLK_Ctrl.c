#include "GPCLK_Ctrl.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>

#include "GPIO_Ctrl.h"

/*
"GPCLK_CTRL_WAIT_KERNEL_RESPONSE"
If defined, application will call kernel and wait for response before proceeding.
Else, application will wait a specific time (defined in GPCLK_CTRL_WAIT_TIME_US) is microseconds before proceeding.
*/
#define GPCLK_CTRL_WAIT_KERNEL_RESPONSE

#define GPCLK_CTRL_PROC_FILE_DIR "/proc/GPCLK_Ctrl"
#define GPCLK_CTRL_WAIT_TIME_US 1

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

int gpclk_proc_fd = -1;
void *gpclk_data_io = NULL;

void gpclk_ctrl_wait(void)
{
	clock_t start_time = clock();
	while(clock() < (start_time + GPCLK_CTRL_WAIT_TIME_US));
	return;
}

bool gpclk_is_active(void)
{
	return (gpclk_proc_fd >= 0);
}

bool gpclk_init(void)
{
	if(gpclk_is_active()) return true;

	if(!gpio_is_active()) if(!gpio_init()) return false;

	gpclk_proc_fd = open(GPCLK_CTRL_PROC_FILE_DIR, O_RDWR);
	if(gpclk_proc_fd < 0) return false;

	gpclk_data_io = malloc(GPCLK_DATAIO_SIZE_BYTES);
	return true;
}

void gpclk_endpoint_map_to_gpio_pinmode(uint8_t gpclk, uint8_t endpoint, uint8_t *p_gpio, uint8_t *p_pinmode)
{
	switch(gpclk)
	{
		case GPCLK0:
			switch(endpoint)
			{
				case GPCLK_ENDPOINT0:
					if(p_gpio != NULL) *p_gpio = 4;
					if(p_pinmode != NULL) *p_pinmode = GPIO_PINMODE_ALTFUNC0;
					break;

				case GPCLK_ENDPOINT1:
					if(p_gpio != NULL) *p_gpio = 20;
					if(p_pinmode != NULL) *p_pinmode = GPIO_PINMODE_ALTFUNC5;
					break;

				case GPCLK_ENDPOINT2:
					if(p_gpio != NULL) *p_gpio = 32;
					if(p_pinmode != NULL) *p_pinmode = GPIO_PINMODE_ALTFUNC0;
					break;

				case GPCLK_ENDPOINT3:
					if(p_gpio != NULL) *p_gpio = 34;
					if(p_pinmode != NULL) *p_pinmode = GPIO_PINMODE_ALTFUNC0;
					break;
			}
			break;

		case GPCLK1:
			switch(endpoint)
			{
				case GPCLK_ENDPOINT0:
					if(p_gpio != NULL) *p_gpio = 5;
					if(p_pinmode != NULL) *p_pinmode = GPIO_PINMODE_ALTFUNC0;
					break;

				case GPCLK_ENDPOINT1:
					if(p_gpio != NULL) *p_gpio = 21;
					if(p_pinmode != NULL) *p_pinmode = GPIO_PINMODE_ALTFUNC5;
					break;

				case GPCLK_ENDPOINT2:
					if(p_gpio != NULL) *p_gpio = 42;
					if(p_pinmode != NULL) *p_pinmode = GPIO_PINMODE_ALTFUNC0;
					break;

				case GPCLK_ENDPOINT3:
					if(p_gpio != NULL) *p_gpio = 44;
					if(p_pinmode != NULL) *p_pinmode = GPIO_PINMODE_ALTFUNC0;
					break;
			}
			break;

		case GPCLK2:
			if(p_pinmode != NULL) *p_pinmode = GPIO_PINMODE_ALTFUNC0;
			switch(endpoint)
			{
				case GPCLK_ENDPOINT0:
					if(p_gpio != NULL) *p_gpio = 6;
					break;

				case GPCLK_ENDPOINT1:
					if(p_gpio != NULL) *p_gpio = 43;
					break;
			}
			break;
	}

	return;
}

void gpclk_init_gpio(uint8_t gpclk, uint8_t endpoint)
{
	if((gpclk == GPCLK2) && (endpoint > 1)) return;

	uint8_t gpio = 0;
	uint8_t pinmode = 0;
	gpclk_endpoint_map_to_gpio_pinmode(gpclk, endpoint, &gpio, &pinmode);

	gpio_reset_pin(gpio);
	gpio_set_pinmode(gpio, pinmode);
	return;
}

void gpclk_deinit_gpio(uint8_t gpclk, uint8_t endpoint)
{
	if((gpclk == GPCLK2) && (endpoint > 1)) return;

	uint8_t gpio = 0;
	gpclk_endpoint_map_to_gpio_pinmode(gpclk, endpoint, &gpio, NULL);

	gpio_reset_pin(gpio);
	return;
}

#ifdef GPCLK_CTRL_WAIT_KERNEL_RESPONSE
void gpclk_call_kernel(void)
{
	uint8_t *pbyte = (uint8_t*) gpclk_data_io;
	write(gpclk_proc_fd, gpclk_data_io, GPCLK_DATAIO_SIZE_BYTES);

	do{
		read(gpclk_proc_fd, gpclk_data_io, GPCLK_DATAIO_SIZE_BYTES);
	}while(pbyte[0] != GPCLK_CMD_KERNEL_RESPONSE);

	return;
}
#else
void gpclk_call_kernel(void)
{
	write(gpclk_proc_fd, gpclk_data_io, GPCLK_DATAIO_SIZE_BYTES);
	gpclk_ctrl_wait();
	read(gpclk_proc_fd, gpclk_data_io, GPCLK_DATAIO_SIZE_BYTES);
	return;
}
#endif

void gpclk_enable(uint8_t gpclk, bool enable)
{
	uint8_t *pbyte = (uint8_t*) gpclk_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = GPCLK_CMD_SET_ENABLE;
	pbyte[1] = gpclk;
	pushort[0] = enable;

	gpclk_call_kernel();
	return;
}

bool gpclk_is_enabled(uint8_t gpclk)
{
	uint8_t *pbyte = (uint8_t*) gpclk_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = GPCLK_CMD_GET_ENABLE;
	pbyte[1] = gpclk;

	gpclk_call_kernel();
	return (pushort[0] & 0x0001);
}

void gpclk_set_mash_level(uint8_t gpclk, uint16_t mash_level)
{
	uint8_t *pbyte = (uint8_t*) gpclk_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = GPCLK_CMD_SET_MASH_LEVEL;
	pbyte[1] = gpclk;
	pushort[0] = mash_level;

	gpclk_call_kernel();
	return;
}

uint16_t gpclk_get_mash_level(uint8_t gpclk)
{
	uint8_t *pbyte = (uint8_t*) gpclk_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = GPCLK_CMD_GET_MASH_LEVEL;
	pbyte[1] = gpclk;
	
	gpclk_call_kernel();
	return pushort[0];
}

void gpclk_invert_output(uint8_t gpclk, bool invert)
{
	uint8_t *pbyte = (uint8_t*) gpclk_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = GPCLK_CMD_SET_INVERT_OUTPUT;
	pbyte[1] = gpclk;
	pushort[0] = invert;

	gpclk_call_kernel();
	return;
}

bool gpclk_output_is_inverted(uint8_t gpclk)
{
	uint8_t *pbyte = (uint8_t*) gpclk_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = GPCLK_CMD_GET_INVERT_OUTPUT;
	pbyte[1] = gpclk;

	gpclk_call_kernel();
	return (pushort[0] & 0x0001);
}

void gpclk_set_clk_src(uint8_t gpclk, uint16_t clksrc)
{
	uint8_t *pbyte = (uint8_t*) gpclk_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = GPCLK_CMD_SET_CLKSRC;
	pbyte[1] = gpclk;
	pushort[0] = clksrc;

	gpclk_call_kernel();
	return;
}

uint16_t gpclk_get_clk_src(uint8_t gpclk)
{
	uint8_t *pbyte = (uint8_t*) gpclk_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = GPCLK_CMD_GET_CLKSRC;
	pbyte[1] = gpclk;

	gpclk_call_kernel();
	return pushort[0];
}

void gpclk_set_integer_divider(uint8_t gpclk, uint16_t divider)
{
	uint8_t *pbyte = (uint8_t*) gpclk_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = GPCLK_CMD_SET_IDIVIDER;
	pbyte[1] = gpclk;
	pushort[0] = divider;

	gpclk_call_kernel();
	return;
}

uint16_t gpclk_get_integer_divider(uint8_t gpclk)
{
	uint8_t *pbyte = (uint8_t*) gpclk_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = GPCLK_CMD_GET_IDIVIDER;
	pbyte[1] = gpclk;

	gpclk_call_kernel();
	return pushort[0];
}

void gpclk_set_fractional_divider(uint8_t gpclk, uint16_t divider)
{
	uint8_t *pbyte = (uint8_t*) gpclk_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = GPCLK_CMD_SET_FDIVIDER;
	pbyte[1] = gpclk;
	pushort[0] = divider;

	gpclk_call_kernel();
	return;
}

uint16_t gpclk_get_fractional_divider(uint8_t gpclk)
{
	uint8_t *pbyte = (uint8_t*) gpclk_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = GPCLK_CMD_GET_FDIVIDER;
	pbyte[1] = gpclk;
	
	gpclk_call_kernel();
	return pushort[0];
}

bool gpclk_is_busy(uint8_t gpclk)
{
	uint8_t *pbyte = (uint8_t*) gpclk_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = GPCLK_CMD_GET_IS_BUSY;
	pbyte[1] = gpclk;

	gpclk_call_kernel();
	return (pushort[0] & 0x0001);
}

void gpclk_set_kill_bit(uint8_t gpclk, bool bit_value)
{
	uint8_t *pbyte = (uint8_t*) gpclk_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = GPCLK_CMD_SET_KILL_BIT;
	pbyte[1] = gpclk;
	pushort[0] = bit_value;

	gpclk_call_kernel();
	return;
}

bool gpclk_get_kill_bit(uint8_t gpclk)
{
	uint8_t *pbyte = (uint8_t*) gpclk_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = GPCLK_CMD_GET_KILL_BIT;
	pbyte[1] = gpclk;

	gpclk_call_kernel();
	return (pushort[0] & 0x0001);
}

