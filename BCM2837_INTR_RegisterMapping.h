#ifndef BCM2837_INTR_REGISTER_MAPPING_H
#define BCM2837_INTR_REGISTER_MAPPING_H

//#define INTR_BASE_ADDR 0x3F00B000
#define INTR_BASE_ADDR 0x3F00B200

#define INTR_MAPPING_SIZE_BYTES 40
#define INTR_MAPPING_SIZE_UINT 10

#define INTR_BASIC_PENDING_UINTP_POS 0
#define INTR_GPU_PENDING0_UINTP_POS 1
#define INTR_GPU_PENDING1_UINTP_POS 2
#define INTR_FIQ_CTRL_UINTP_POS 3
#define INTR_GPU_ENABLE0_UINTP_POS 4
#define INTR_GPU_ENABLE1_UINTP_POS 5
#define INTR_BASIC_ENABLE_UINTP_POS 6
#define INTR_GPU_DISABLE0_UINTP_POS 7
#define INTR_GPU_DISABLE1_UINTP_POS 8
#define INTR_BASIC_DISABLE_UINTP_POS 9

#endif