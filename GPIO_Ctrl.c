#include "GPIO_Ctrl.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#define GPIO_CTRL_WAIT_KERNEL_RESPONSE

#define GPIO_CTRL_PROC_FILE_DIR "/proc/GPIO_Ctrl"
#define GPIO_CTRL_WAIT_TIME_US 1

#define GPIO_DATAIO_SIZE_BYTES 3

#define GPIO_CMD_RESET_PIN 0
#define GPIO_CMD_SET_LEVEL 1
#define GPIO_CMD_GET_LEVEL 2
#define GPIO_CMD_SET_PINMODE 3
#define GPIO_CMD_GET_PINMODE 4
#define GPIO_CMD_SET_PUDCTRL 5
#define GPIO_CMD_GET_EVENTDETECTED 6
#define GPIO_CMD_SET_ENABLE_REDGEDETECT 7
#define GPIO_CMD_GET_ENABLE_REDGEDETECT 8
#define GPIO_CMD_SET_ENABLE_FEDGEDETECT 9
#define GPIO_CMD_GET_ENABLE_FEDGEDETECT 10
#define GPIO_CMD_SET_ENABLE_ASYNC_REDGEDETECT 11
#define GPIO_CMD_GET_ENABLE_ASYNC_REDGEDETECT 12
#define GPIO_CMD_SET_ENABLE_ASYNC_FEDGEDETECT 13
#define GPIO_CMD_GET_ENABLE_ASYNC_FEDGEDETECT 14
#define GPIO_CMD_SET_ENABLE_HIGHDETECT 15
#define GPIO_CMD_GET_ENABLE_HIGHDETECT 16
#define GPIO_CMD_SET_ENABLE_LOWDETECT 17
#define GPIO_CMD_GET_ENABLE_LOWDETECT 18

#define GPIO_CMD_KERNEL_RESPONSE 0xFF

int gpio_proc_fd = -1;
void *gpio_data_io = NULL;

void gpio_ctrl_wait(void)
{
	clock_t start_time = clock();
	while(clock() < (start_time + GPIO_CTRL_WAIT_TIME_US));
	return;
}

bool gpio_is_active(void)
{
	return (gpio_proc_fd >= 0);
}

bool gpio_init(void)
{
	if(gpio_is_active()) return true;

	gpio_proc_fd = open(GPIO_CTRL_PROC_FILE_DIR, O_RDWR);
	if(gpio_proc_fd < 0) return false;

	gpio_data_io = malloc(GPIO_DATAIO_SIZE_BYTES);
	return true;
}

#ifdef GPIO_CTRL_WAIT_KERNEL_RESPONSE
void gpio_call_kernel(void)
{
	uint8_t *pbyte = (uint8_t*) gpio_data_io;
	write(gpio_proc_fd, gpio_data_io, GPIO_DATAIO_SIZE_BYTES);

	do{
		read(gpio_proc_fd, gpio_data_io, GPIO_DATAIO_SIZE_BYTES);
	}while(pbyte[0] != GPIO_CMD_KERNEL_RESPONSE);

	return;
}
#else
void gpio_call_kernel(void)
{
	write(gpio_proc_fd, gpio_data_io, GPIO_DATAIO_SIZE_BYTES);
	gpio_ctrl_wait();
	read(gpio_proc_fd, gpio_data_io, GPIO_DATAIO_SIZE_BYTES);
	return;
}
#endif

void gpio_reset_pin(uint8_t pin_number)
{
	uint8_t *pbyte = (uint8_t*) gpio_data_io;
	pbyte[0] = GPIO_CMD_RESET_PIN;
	pbyte[1] = pin_number;

	gpio_call_kernel();
	return;
}

void gpio_set_pinmode(uint8_t pin_number, uint8_t pinmode)
{
	uint8_t *pbyte = (uint8_t*) gpio_data_io;
	pbyte[0] = GPIO_CMD_SET_PINMODE;
	pbyte[1] = pin_number;
	pbyte[2] = pinmode;

	gpio_call_kernel();
	return;
}

uint8_t gpio_get_pinmode(uint8_t pin_number)
{
	uint8_t *pbyte = (uint8_t*) gpio_data_io;
	pbyte[0] = GPIO_CMD_GET_PINMODE;
	pbyte[1] = pin_number;

	gpio_call_kernel();
	return pbyte[2];
}

void gpio_set_pudctrl(uint8_t pin_number, uint8_t pudctrl)
{
	uint8_t *pbyte = (uint8_t*) gpio_data_io;
	pbyte[0] = GPIO_CMD_SET_PUDCTRL;
	pbyte[1] = pin_number;
	pbyte[2] = pudctrl;

	gpio_call_kernel();
	return;
}

void gpio_set_level(uint8_t pin_number, bool level)
{
	uint8_t *pbyte = (uint8_t*) gpio_data_io;
	pbyte[0] = GPIO_CMD_SET_LEVEL;
	pbyte[1] = pin_number;
	pbyte[2] = level;

	gpio_call_kernel();
	return;
}

bool gpio_get_level(uint8_t pin_number)
{
	uint8_t *pbyte = (uint8_t*) gpio_data_io;
	pbyte[0] = GPIO_CMD_GET_LEVEL;
	pbyte[1] = pin_number;

	gpio_call_kernel();
	return (pbyte[2] & 0x01);
}

bool gpio_event_detected(uint8_t pin_number)
{
	uint8_t *pbyte = (uint8_t*) gpio_data_io;
	pbyte[0] = GPIO_CMD_GET_EVENTDETECTED;
	pbyte[1] = pin_number;

	gpio_call_kernel();
	return (pbyte[2] & 0x01);
}

void gpio_enable_risingedge_detect(uint8_t pin_number, bool enable)
{
	uint8_t *pbyte = (uint8_t*) gpio_data_io;
	pbyte[0] = GPIO_CMD_SET_ENABLE_REDGEDETECT;
	pbyte[1] = pin_number;
	pbyte[2] = enable;

	gpio_call_kernel();
	return;
}

bool gpio_risingedge_detect_is_enabled(uint8_t pin_number)
{
	uint8_t *pbyte = (uint8_t*) gpio_data_io;
	pbyte[0] = GPIO_CMD_GET_ENABLE_REDGEDETECT;
	pbyte[1] = pin_number;

	gpio_call_kernel();
	return (pbyte[2] & 0x01);
}

void gpio_enable_fallingedge_detect(uint8_t pin_number, bool enable)
{
	uint8_t *pbyte = (uint8_t*) gpio_data_io;
	pbyte[0] = GPIO_CMD_SET_ENABLE_FEDGEDETECT;
	pbyte[1] = pin_number;
	pbyte[2] = enable;
	
	gpio_call_kernel();
	return;
}

bool gpio_fallingedge_detect_is_enabled(uint8_t pin_number)
{
	uint8_t *pbyte = (uint8_t*) gpio_data_io;
	pbyte[0] = GPIO_CMD_GET_ENABLE_FEDGEDETECT;
	pbyte[1] = pin_number;

	gpio_call_kernel();
	return (pbyte[2] & 0x01);
}

void gpio_enable_async_risingedge_detect(uint8_t pin_number, bool enable)
{
	uint8_t *pbyte = (uint8_t*) gpio_data_io;
	pbyte[0] = GPIO_CMD_SET_ENABLE_ASYNC_REDGEDETECT;
	pbyte[1] = pin_number;
	pbyte[2] = enable;

	gpio_call_kernel();
	return;
}

bool gpio_async_risingedge_detect_is_enabled(uint8_t pin_number)
{
	uint8_t *pbyte = (uint8_t*) gpio_data_io;
	pbyte[0] = GPIO_CMD_GET_ENABLE_ASYNC_REDGEDETECT;
	pbyte[1] = pin_number;
	
	gpio_call_kernel();
	return (pbyte[2] & 0x01);
}

void gpio_enable_async_fallingedge_detect(uint8_t pin_number, bool enable)
{
	uint8_t *pbyte = (uint8_t*) gpio_data_io;
	pbyte[0] = GPIO_CMD_SET_ENABLE_ASYNC_FEDGEDETECT;
	pbyte[1] = pin_number;
	pbyte[2] = enable;

	gpio_call_kernel();
	return;
}

bool gpio_async_fallingedge_detect_is_enabled(uint8_t pin_number)
{
	uint8_t *pbyte = (uint8_t*) gpio_data_io;
	pbyte[0] = GPIO_CMD_GET_ENABLE_ASYNC_FEDGEDETECT;
	pbyte[1] = pin_number;

	gpio_call_kernel();
	return (pbyte[2] & 0x01);
}

void gpio_enable_high_detect(uint8_t pin_number, bool enable)
{
	uint8_t *pbyte = (uint8_t*) gpio_data_io;
	pbyte[0] = GPIO_CMD_SET_ENABLE_HIGHDETECT;
	pbyte[1] = pin_number;
	pbyte[2] = enable;

	gpio_call_kernel();
	return;
}

bool gpio_high_detect_is_enabled(uint8_t pin_number)
{
	uint8_t *pbyte = (uint8_t*) gpio_data_io;
	pbyte[0] = GPIO_CMD_GET_ENABLE_HIGHDETECT;
	pbyte[1] = pin_number;
	
	gpio_call_kernel();
	return (pbyte[2] & 0x01);
}

void gpio_enable_low_detect(uint8_t pin_number, bool enable)
{
	uint8_t *pbyte = (uint8_t*) gpio_data_io;
	pbyte[0] = GPIO_CMD_SET_ENABLE_LOWDETECT;
	pbyte[1] = pin_number;
	pbyte[2] = enable;

	gpio_call_kernel();
	return;
}

bool gpio_low_detect_is_enabled(uint8_t pin_number)
{
	uint8_t *pbyte = (uint8_t*) gpio_data_io;
	pbyte[0] = GPIO_CMD_GET_ENABLE_LOWDETECT;
	pbyte[1] = pin_number;

	gpio_call_kernel();
	return (pbyte[2] & 0x01);
}

