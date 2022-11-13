#include "I2C_Ctrl.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>

#include "GPIO_Ctrl.h"

#define I2C_CTRL_WAIT_KERNEL_RESPONSE

#define I2C_CTRL_PROC_FILE_DIR "/proc/I2C_Ctrl"
#define I2C_CTRL_WAIT_TIME_US 1

#define I2C_DATAIO_SIZE_BYTES 4

#define I2C_CMD_INIT_DEFAULT 0
#define I2C_CMD_DEINIT_DEFAULT 1
#define I2C_CMD_SET_ENABLE_CTRL 2
#define I2C_CMD_GET_ENABLE_CTRL 3
#define I2C_CMD_SET_ENABLE_INTR_ON_RX 4
#define I2C_CMD_GET_ENABLE_INTR_ON_RX 5
#define I2C_CMD_SET_ENABLE_INTR_ON_TX 6
#define I2C_CMD_GET_ENABLE_INTR_ON_TX 7
#define I2C_CMD_SET_ENABLE_INTR_ON_DONE 8
#define I2C_CMD_GET_ENABLE_INTR_ON_DONE 9
#define I2C_CMD_SET_RW_BIT 10
#define I2C_CMD_GET_RW_BIT 11
#define I2C_CMD_SET_TRANSFER_LENGTH 12
#define I2C_CMD_GET_TRANSFER_LENGTH 13
#define I2C_CMD_SET_SLAVE_ADDR 14
#define I2C_CMD_GET_SLAVE_ADDR 15
#define I2C_CMD_SET_FIFO_DATA 16
#define I2C_CMD_GET_FIFO_DATA 17
#define I2C_CMD_SET_CLKDIV 18
#define I2C_CMD_GET_CLKDIV 19
#define I2C_CMD_SET_FEDGE_DELAY 20
#define I2C_CMD_GET_FEDGE_DELAY 21
#define I2C_CMD_SET_REDGE_DELAY 22
#define I2C_CMD_GET_REDGE_DELAY 23
#define I2C_CMD_SET_TIMEOUT 24
#define I2C_CMD_GET_TIMEOUT 25
#define I2C_CMD_START_TRANSFER 26
#define I2C_CMD_CLEAR_FIFO 27
#define I2C_CMD_GET_TIMEOUT_OCCURRED 28
#define I2C_CMD_GET_ACK_ERR_OCCURRED 29
#define I2C_CMD_GET_FIFO_FULL 30
#define I2C_CMD_GET_FIFO_EMPTY 31
#define I2C_CMD_GET_FIFO_HAS_DATA 32
#define I2C_CMD_GET_FIFO_FITS_DATA 33
#define I2C_CMD_GET_FIFO_ALMOST_FULL 34
#define I2C_CMD_GET_FIFO_ALMOST_EMPTY 35
#define I2C_CMD_GET_TRANSFER_DONE 36
#define I2C_CMD_GET_TRANSFER_ACTIVE 37
#define I2C_CMD_SET_STD_CLKDIV 38
#define I2C_CMD_SET_STD_DATADELAY 39

#define I2C_CMD_KERNEL_RESPONSE 0xFF

int i2c_proc_fd = -1;
void *i2c_data_io = NULL;

void i2c_ctrl_wait(void)
{
	clock_t start_time = clock();
	while(clock() < (start_time + I2C_CTRL_WAIT_TIME_US));
	return;
}

bool i2c_is_active(void)
{
	return (i2c_proc_fd >= 0);
}

bool i2c_init(void)
{
	if(i2c_is_active()) return true;

	if(!gpio_is_active()) if(!gpio_init()) return false;

	i2c_proc_fd = open(I2C_CTRL_PROC_FILE_DIR, O_RDWR);
	if(i2c_proc_fd < 0) return false;

	i2c_data_io = malloc(I2C_DATAIO_SIZE_BYTES);
	return true;
}

void i2c_ctrl_endpoint_map_to_gpio_pinmode(uint8_t i2c_ctrl, uint8_t endpoint, uint8_t *p_sda_gpio, uint8_t *p_scl_gpio, uint8_t *p_pinmode)
{
	switch(i2c_ctrl)
	{
		case I2C_CTRL0:
			switch(endpoint)
			{
				case I2C_ENDPOINT0:
					if(p_sda_gpio != NULL) *p_sda_gpio = 0;
					if(p_scl_gpio != NULL) *p_scl_gpio = 1;
					if(p_pinmode != NULL) *p_pinmode = GPIO_PINMODE_ALTFUNC0;
					break;

				case I2C_ENDPOINT1:
					if(p_sda_gpio != NULL) *p_sda_gpio = 28;
					if(p_scl_gpio != NULL) *p_scl_gpio = 29;
					if(p_pinmode != NULL) *p_pinmode = GPIO_PINMODE_ALTFUNC0;
					break;

				case I2C_ENDPOINT2:
					if(p_sda_gpio != NULL) *p_sda_gpio = 44;
					if(p_scl_gpio != NULL) *p_scl_gpio = 45;
					if(p_pinmode != NULL) *p_pinmode = GPIO_PINMODE_ALTFUNC1;
					break;
			}
			break;

		case I2C_CTRL1:
			switch(endpoint)
			{
				case I2C_ENDPOINT0:
					if(p_sda_gpio != NULL) *p_sda_gpio = 2;
					if(p_scl_gpio != NULL) *p_scl_gpio = 3;
					if(p_pinmode != NULL) *p_pinmode = GPIO_PINMODE_ALTFUNC0;
					break;

				case I2C_ENDPOINT1:
					if(p_sda_gpio != NULL) *p_sda_gpio = 44;
					if(p_scl_gpio != NULL) *p_scl_gpio = 45;
					if(p_pinmode != NULL) *p_pinmode = GPIO_PINMODE_ALTFUNC2;
					break;
			}
			break;

		case I2C_CTRL2:
			if(endpoint == I2C_ENDPOINT0)
			{
				if(p_sda_gpio != NULL) *p_sda_gpio = 18;
				if(p_scl_gpio != NULL) *p_scl_gpio = 19;
				if(p_pinmode != NULL) *p_pinmode = GPIO_PINMODE_ALTFUNC3;
			}
			break;
	}

	return;
}

#ifdef I2C_CTRL_WAIT_KERNEL_RESPONSE
void i2c_call_kernel(void)
{
	uint8_t *pbyte = (uint8_t*) i2c_data_io;
	write(i2c_proc_fd, i2c_data_io, I2C_DATAIO_SIZE_BYTES);

	do{
		read(i2c_proc_fd, i2c_data_io, I2C_DATAIO_SIZE_BYTES);
	}while(pbyte[0] != I2C_CMD_KERNEL_RESPONSE);

	return;
}
#else
void i2c_call_kernel(void)
{
	write(i2c_proc_fd, i2c_data_io, I2C_DATAIO_SIZE_BYTES);
	i2c_ctrl_wait();
	read(i2c_proc_fd, i2c_data_io, I2C_DATAIO_SIZE_BYTES);
	return;
}
#endif

void i2c_init_gpio_default(uint8_t i2c_ctrl, uint8_t endpoint, bool enable_pullup)
{
	if((i2c_ctrl == I2C_CTRL2) && (endpoint != I2C_ENDPOINT0)) return;
	if((i2c_ctrl == I2C_CTRL1) && (endpoint == I2C_ENDPOINT2)) return;

	uint8_t sda = 0;
	uint8_t scl = 0;
	uint8_t pinmode = 0;

	i2c_ctrl_endpoint_map_to_gpio_pinmode(i2c_ctrl, endpoint, &sda, &scl, &pinmode);

	gpio_reset_pin(sda);
	gpio_reset_pin(scl);

	gpio_set_pinmode(sda, pinmode);
	gpio_set_pinmode(scl, pinmode);

	if(enable_pullup)
	{
		gpio_set_pudctrl(sda, GPIO_PUDCTRL_PULLUP);
		gpio_set_pudctrl(scl, GPIO_PUDCTRL_PULLUP);
	}

	return;
}

void i2c_deinit_gpio_default(uint8_t i2c_ctrl, uint8_t endpoint)
{
	if((i2c_ctrl == I2C_CTRL2) && (endpoint != I2C_ENDPOINT0)) return;
	if((i2c_ctrl == I2C_CTRL1) && (endpoint == I2C_ENDPOINT2)) return;

	uint8_t sda = 0;
	uint8_t scl = 0;

	i2c_ctrl_endpoint_map_to_gpio_pinmode(i2c_ctrl, endpoint, &sda, &scl, NULL);

	gpio_reset_pin(sda);
	gpio_reset_pin(scl);
	return;
}

void i2c_init_core_default(uint8_t i2c_ctrl, bool use_400kbps)
{
	uint8_t *pbyte = (uint8_t*) i2c_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = I2C_CMD_INIT_DEFAULT;
	pbyte[1] = i2c_ctrl;
	pushort[0] = use_400kbps;

	i2c_call_kernel();
	return;
}

void i2c_deinit_core_default(uint8_t i2c_ctrl)
{
	uint8_t *pbyte = (uint8_t*) i2c_data_io;
	pbyte[0] = I2C_CMD_DEINIT_DEFAULT;
	pbyte[1] = i2c_ctrl;

	i2c_call_kernel();
	return;
}

void i2c_init_default(uint8_t i2c_ctrl, uint8_t endpoint, bool use_400kbps, bool enable_pin_pullup)
{
	i2c_init_gpio_default(i2c_ctrl, endpoint, enable_pin_pullup);
	i2c_init_core_default(i2c_ctrl, use_400kbps);
	return;
}

void i2c_deinit_default(uint8_t i2c_ctrl, uint8_t endpoint)
{
	i2c_deinit_core_default(i2c_ctrl);
	i2c_deinit_gpio_default(i2c_ctrl, endpoint);
	return;
}

void i2c_ctrl_enable(uint8_t i2c_ctrl, bool enable)
{
	uint8_t *pbyte = (uint8_t*) i2c_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = I2C_CMD_SET_ENABLE_CTRL;
	pbyte[1] = i2c_ctrl;
	pushort[0] = enable;

	i2c_call_kernel();
	return;
}

bool i2c_ctrl_is_enabled(uint8_t i2c_ctrl)
{
	uint8_t *pbyte = (uint8_t*) i2c_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = I2C_CMD_GET_ENABLE_CTRL;
	pbyte[1] = i2c_ctrl;

	i2c_call_kernel();
	return (pushort[0] & 0x0001);
}

void i2c_enable_intr_on_rx(uint8_t i2c_ctrl, bool enable)
{
	uint8_t *pbyte = (uint8_t*) i2c_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = I2C_CMD_SET_ENABLE_INTR_ON_RX;
	pbyte[1] = i2c_ctrl;
	pushort[0] = enable;

	i2c_call_kernel();
	return;
}

bool i2c_intr_on_rx_is_enabled(uint8_t i2c_ctrl)
{
	uint8_t *pbyte = (uint8_t*) i2c_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = I2C_CMD_GET_ENABLE_INTR_ON_RX;
	pbyte[1] = i2c_ctrl;
	
	i2c_call_kernel();
	return (pushort[0] & 0x0001);
}

void i2c_enable_intr_on_tx(uint8_t i2c_ctrl, bool enable)
{
	uint8_t *pbyte = (uint8_t*) i2c_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = I2C_CMD_SET_ENABLE_INTR_ON_TX;
	pbyte[1] = i2c_ctrl;
	pushort[0] = enable;

	i2c_call_kernel();
	return;
}

bool i2c_intr_on_tx_is_enabled(uint8_t i2c_ctrl)
{
	uint8_t *pbyte = (uint8_t*) i2c_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = I2C_CMD_GET_ENABLE_INTR_ON_TX;
	pbyte[1] = i2c_ctrl;

	i2c_call_kernel();
	return (pushort[0] & 0x0001);
}

void i2c_enable_intr_on_done(uint8_t i2c_ctrl, bool enable)
{
	uint8_t *pbyte = (uint8_t*) i2c_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = I2C_CMD_SET_ENABLE_INTR_ON_DONE;
	pbyte[1] = i2c_ctrl;
	pushort[0] = enable;

	i2c_call_kernel();
	return;
}

bool i2c_intr_on_done_is_enabled(uint8_t i2c_ctrl)
{
	uint8_t *pbyte = (uint8_t*) i2c_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = I2C_CMD_GET_ENABLE_INTR_ON_DONE;
	pbyte[1] = i2c_ctrl;
	
	i2c_call_kernel();
	return (pushort[0] & 0x0001);
}

void i2c_set_rw_bit(uint8_t i2c_ctrl, bool bit_value)
{
	uint8_t *pbyte = (uint8_t*) i2c_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = I2C_CMD_SET_RW_BIT;
	pbyte[1] = i2c_ctrl;
	pushort[0] = bit_value;

	i2c_call_kernel();
	return;
}

bool i2c_get_rw_bit(uint8_t i2c_ctrl)
{
	uint8_t *pbyte = (uint8_t*) i2c_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = I2C_CMD_GET_RW_BIT;
	pbyte[1] = i2c_ctrl;

	i2c_call_kernel();
	return (pushort[0] & 0x0001);
}

void i2c_set_transfer_length_bytes(uint8_t i2c_ctrl, uint16_t length)
{
	uint8_t *pbyte = (uint8_t*) i2c_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = I2C_CMD_SET_TRANSFER_LENGTH;
	pbyte[1] = i2c_ctrl;
	pushort[0] = length;

	i2c_call_kernel();
	return;
}

uint16_t i2c_get_transfer_length_bytes(uint8_t i2c_ctrl)
{
	uint8_t *pbyte = (uint8_t*) i2c_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = I2C_CMD_GET_TRANSFER_LENGTH;
	pbyte[1] = i2c_ctrl;

	i2c_call_kernel();
	return pushort[0];
}

void i2c_set_slave_addr(uint8_t i2c_ctrl, uint8_t slave_addr)
{
	uint8_t *pbyte = (uint8_t*) i2c_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = I2C_CMD_SET_SLAVE_ADDR;
	pbyte[1] = i2c_ctrl;
	pushort[0] = slave_addr;

	i2c_call_kernel();
	return;
}

uint8_t i2c_get_slave_addr(uint8_t i2c_ctrl)
{
	uint8_t *pbyte = (uint8_t*) i2c_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = I2C_CMD_GET_SLAVE_ADDR;
	pbyte[1] = i2c_ctrl;

	i2c_call_kernel();
	return (pushort[0] & 0x00FF);
}

void i2c_set_fifo_data(uint8_t i2c_ctrl, uint8_t data)
{
	uint8_t *pbyte = (uint8_t*) i2c_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = I2C_CMD_SET_FIFO_DATA;
	pbyte[1] = i2c_ctrl;
	pushort[0] = data;

	i2c_call_kernel();
	return;
}

uint8_t i2c_get_fifo_data(uint8_t i2c_ctrl)
{
	uint8_t *pbyte = (uint8_t*) i2c_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = I2C_CMD_GET_FIFO_DATA;
	pbyte[1] = i2c_ctrl;

	i2c_call_kernel();
	return (pushort[0] & 0x00FF);
}

void i2c_set_clkdiv(uint8_t i2c_ctrl, uint16_t clkdiv)
{
	uint8_t *pbyte = (uint8_t*) i2c_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = I2C_CMD_SET_CLKDIV;
	pbyte[1] = i2c_ctrl;
	pushort[0] = clkdiv;

	i2c_call_kernel();
	return;
}

uint16_t i2c_get_clkdiv(uint8_t i2c_ctrl)
{
	uint8_t *pbyte = (uint8_t*) i2c_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = I2C_CMD_GET_CLKDIV;
	pbyte[1] = i2c_ctrl;

	i2c_call_kernel();
	return pushort[0];
}

void i2c_set_fallingedge_delay(uint8_t i2c_ctrl, uint16_t delay)
{
	uint8_t *pbyte = (uint8_t*) i2c_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = I2C_CMD_SET_FEDGE_DELAY;
	pbyte[1] = i2c_ctrl;
	pushort[0] = delay;

	i2c_call_kernel();
	return;
}

uint16_t i2c_get_fallingedge_delay(uint8_t i2c_ctrl)
{
	uint8_t *pbyte = (uint8_t*) i2c_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = I2C_CMD_GET_FEDGE_DELAY;
	pbyte[1] = i2c_ctrl;

	i2c_call_kernel();
	return pushort[0];
}

void i2c_set_risingedge_delay(uint8_t i2c_ctrl, uint16_t delay)
{
	uint8_t *pbyte = (uint8_t*) i2c_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = I2C_CMD_SET_REDGE_DELAY;
	pbyte[1] = i2c_ctrl;
	pushort[0] = delay;

	i2c_call_kernel();
	return;
}

uint16_t i2c_get_risingedge_delay(uint8_t i2c_ctrl)
{
	uint8_t *pbyte = (uint8_t*) i2c_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = I2C_CMD_GET_REDGE_DELAY;
	pbyte[1] = i2c_ctrl;
	
	i2c_call_kernel();
	return pushort[0];
}

void i2c_set_timeout(uint8_t i2c_ctrl, uint16_t timeout)
{
	uint8_t *pbyte = (uint8_t*) i2c_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = I2C_CMD_SET_TIMEOUT;
	pbyte[1] = i2c_ctrl;
	pushort[0] = timeout;

	i2c_call_kernel();
	return;
}

uint16_t i2c_get_timeout(uint8_t i2c_ctrl)
{
	uint8_t *pbyte = (uint8_t*) i2c_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = I2C_CMD_GET_TIMEOUT;
	pbyte[1] = i2c_ctrl;

	i2c_call_kernel();
	return pushort[0];
}

void i2c_start_transfer(uint8_t i2c_ctrl)
{
	uint8_t *pbyte = (uint8_t*) i2c_data_io;
	pbyte[0] = I2C_CMD_START_TRANSFER;
	pbyte[1] = i2c_ctrl;

	i2c_call_kernel();
	return;
}

void i2c_clear_fifo(uint8_t i2c_ctrl)
{
	uint8_t *pbyte = (uint8_t*) i2c_data_io;
	pbyte[0] = I2C_CMD_CLEAR_FIFO;
	pbyte[1] = i2c_ctrl;

	i2c_call_kernel();
	return;
}

bool i2c_timeout_occurred(uint8_t i2c_ctrl)
{
	uint8_t *pbyte = (uint8_t*) i2c_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = I2C_CMD_GET_TIMEOUT_OCCURRED;
	pbyte[1] = i2c_ctrl;

	i2c_call_kernel();
	return (pushort[0] & 0x0001);
}

bool i2c_ack_err_occurred(uint8_t i2c_ctrl)
{
	uint8_t *pbyte = (uint8_t*) i2c_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = I2C_CMD_GET_ACK_ERR_OCCURRED;
	pbyte[1] = i2c_ctrl;

	i2c_call_kernel();
	return (pushort[0] & 0x0001);
}

bool i2c_fifo_is_full(uint8_t i2c_ctrl)
{
	uint8_t *pbyte = (uint8_t*) i2c_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = I2C_CMD_GET_FIFO_FULL;
	pbyte[1] = i2c_ctrl;

	i2c_call_kernel();
	return (pushort[0] & 0x0001);
}

bool i2c_fifo_is_empty(uint8_t i2c_ctrl)
{
	uint8_t *pbyte = (uint8_t*) i2c_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = I2C_CMD_GET_FIFO_EMPTY;
	pbyte[1] = i2c_ctrl;

	i2c_call_kernel();
	return (pushort[0] & 0x0001);
}

bool i2c_fifo_has_data(uint8_t i2c_ctrl)
{
	uint8_t *pbyte = (uint8_t*) i2c_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = I2C_CMD_GET_FIFO_HAS_DATA;
	pbyte[1] = i2c_ctrl;

	i2c_call_kernel();
	return (pushort[0] & 0x0001);
}

bool i2c_fifo_fits_data(uint8_t i2c_ctrl)
{
	uint8_t *pbyte = (uint8_t*) i2c_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = I2C_CMD_GET_FIFO_FITS_DATA;
	pbyte[1] = i2c_ctrl;

	i2c_call_kernel();
	return (pushort[0] & 0x0001);
}

bool i2c_fifo_is_almost_full(uint8_t i2c_ctrl)
{
	uint8_t *pbyte = (uint8_t*) i2c_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = I2C_CMD_GET_FIFO_ALMOST_FULL;
	pbyte[1] = i2c_ctrl;

	i2c_call_kernel();
	return (pushort[0] & 0x0001);
}

bool i2c_fifo_is_almost_empty(uint8_t i2c_ctrl)
{
	uint8_t *pbyte = (uint8_t*) i2c_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = I2C_CMD_GET_FIFO_ALMOST_EMPTY;
	pbyte[1] = i2c_ctrl;

	i2c_call_kernel();
	return (pushort[0] & 0x0001);
}

bool i2c_transfer_done(uint8_t i2c_ctrl)
{
	uint8_t *pbyte = (uint8_t*) i2c_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = I2C_CMD_GET_TRANSFER_DONE;
	pbyte[1] = i2c_ctrl;

	i2c_call_kernel();
	return (pushort[0] & 0x0001);
}

bool i2c_transfer_is_active(uint8_t i2c_ctrl)
{
	uint8_t *pbyte = (uint8_t*) i2c_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = I2C_CMD_GET_TRANSFER_ACTIVE;
	pbyte[1] = i2c_ctrl;

	i2c_call_kernel();
	return (pushort[0] & 0x0001);
}

void i2c_set_std_clkdiv(uint8_t i2c_ctrl, bool use_400kbps)
{
	uint8_t *pbyte = (uint8_t*) i2c_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = I2C_CMD_SET_STD_CLKDIV;
	pbyte[1] = i2c_ctrl;
	pushort[0] = use_400kbps;

	i2c_call_kernel();
	return;
}

void i2c_set_std_data_delay(uint8_t i2c_ctrl, bool use_400kbps)
{
	uint8_t *pbyte = (uint8_t*) i2c_data_io;
	uint16_t *pushort = (uint16_t*) &pbyte[2];
	pbyte[0] = I2C_CMD_SET_STD_DATADELAY;
	pbyte[1] = i2c_ctrl;
	pushort[0] = use_400kbps;

	i2c_call_kernel();
	return;
}


