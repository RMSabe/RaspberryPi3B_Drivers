#ifndef BCM2837_I2C_REGISTER_MAPPING_H
#define BCM2837_I2C_REGISTER_MAPPING_H

#define I2C0_BASE_ADDR 0x3F205000
#define I2C1_BASE_ADDR 0x3F804000
#define I2C2_BASE_ADDR 0x3F805000

#define I2C_MAPPING_SIZE_BYTES 32
#define I2C_MAPPING_SIZE_UINT 8

#define I2C_CTRL_UINTP_POS 0
#define I2C_STATUS_UINTP_POS 1
#define I2C_DATALENGTH_UINTP_POS 2
#define I2C_SLAVEADDR_UINTP_POS 3
#define I2C_DATAFIFO_UINTP_POS 4
#define I2C_CLKDIV_UINTP_POS 5
#define I2C_DATADELAY_UINTP_POS 6
#define I2C_CLKTIMEOUT_UINTP_POS 7

#endif
