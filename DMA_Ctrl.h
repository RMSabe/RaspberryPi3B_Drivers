#ifndef DMA_CTRL_H
#define DMA_CTRL_H

#include <stdbool.h>
#include <stdint.h>

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

#define DMA_PERMAP_ALWAYS_ON 0
#define DMA_PERMAP_DSI 1
#define DMA_PERMAP_PCM_TX 2
#define DMA_PERMAP_PCM_RX 3
#define DMA_PERMAP_SMI 4
#define DMA_PERMAP_PWM 5
#define DMA_PERMAP_SPI_TX 6
#define DMA_PERMAP_SPI_RX 7
#define DMA_PERMAP_I2C_TX 8
#define DMA_PERMAP_I2C_RX 9
#define DMA_PERMAP_EMMC 11
#define DMA_PERMAP_UART_TX 12
#define DMA_PERMAP_SD_HOST 13
#define DMA_PERMAP_UART_RX 14
//#define DMA_PERMAP_DSI 15
#define DMA_PERMAP_SLIMBUS_MCTX 16
#define DMA_PERMAP_HDMI 17
#define DMA_PERMAP_SLIMBUS_MCRX 18
#define DMA_PERMAP_SLIMBUS_DC0 19
#define DMA_PERMAP_SLIMBUS_DC1 20
#define DMA_PERMAP_SLIMBUS_DC2 21
#define DMA_PERMAP_SLIMBUS_DC3 22
#define DMA_PERMAP_SLIMBUS_DC4 23
#define DMA_PERMAP_SCALERFIFO0 24
#define DMA_PERMAP_SCALERFIFO1 25
#define DMA_PERMAP_SCALERFIFO2 26
#define DMA_PERMAP_SLIMBUS_DC5 27
#define DMA_PERMAP_SLIMBUS_DC6 28
#define DMA_PERMAP_SLIMBUS_DC7 29
#define DMA_PERMAP_SLIMBUS_DC8 30
#define DMA_PERMAP_SLIMBUS_DC9 31

#define DMA_PERMAP_BSC_SPI_TX DMA_PERMAP_I2C_TX
#define DMA_PERMAP_BSC_SPI_RX DMA_PERMAP_I2C_RX

typedef struct {
	uint32_t transfer_info;
	uint32_t src_addr;
	uint32_t dst_addr;
	uint32_t transfer_length;
	uint32_t stride;
	uint32_t next_ctrlblock_addr;
	uint32_t unused_0;
	uint32_t unused_1;
} dma_ctrlblock_t;

bool dma_is_active(void);
bool dma_init(void);
void dma_reset_ctrlblock(dma_ctrlblock_t *p_ctrlblock);
void dma_enable_ctrl(uint8_t dma_ctrl, bool enable);
bool dma_ctrl_is_enabled(uint8_t dma_ctrl);
bool dma_get_channel_intr_status(uint8_t dma_ctrl);
uint32_t dma_get_full_intr_status(void);
void dma_set_ctrlblock_addr_phys(uint8_t dma_ctrl, uint32_t addr);
uint32_t dma_get_ctrlblock_addr_phys(uint8_t dma_ctrl);
void dma_set_ctrlblock_addr_virt(uint8_t dma_ctrl, dma_ctrlblock_t *p_ctrlblock);
void dma_set_transfer_active(uint8_t dma_ctrl, bool active);
bool dma_get_transfer_active(uint8_t dma_ctrl);
bool dma_transfer_done(uint8_t dma_ctrl);
bool dma_get_intr_status(uint8_t dma_ctrl);
bool dma_is_requesting_data(uint8_t dma_ctrl);
bool dma_is_paused(uint8_t dma_ctrl);
bool dma_is_paused_by_inactive_dreq(uint8_t dma_ctrl);
bool dma_is_waiting_ostd_writes(uint8_t dma_ctrl);
bool dma_error_occurred(uint8_t dma_ctrl);
void dma_set_priority(uint8_t dma_ctrl, uint32_t priority);
uint32_t dma_get_priority(uint8_t dma_ctrl);
void dma_set_panic_priority(uint8_t dma_ctrl, uint32_t priority);
uint32_t dma_get_panic_priority(uint8_t dma_ctrl);
void dma_enable_wait_ostd_writes(uint8_t dma_ctrl, bool enable);
bool dma_wait_ostd_writes_is_enabled(uint8_t dma_ctrl);
void dma_disable_debug_pause(uint8_t dma_ctrl, bool disable);
bool dma_debug_pause_is_disabled(uint8_t dma_ctrl);
void dma_abort(uint8_t dma_ctrl);
void dma_reset(uint8_t dma_ctrl);
bool dma_wide_bursts_is_disabled(uint8_t dma_ctrl);
uint32_t dma_get_wait_cycles(uint8_t dma_ctrl);
uint32_t dma_get_permap(uint8_t dma_ctrl);
uint32_t dma_get_burst_length(uint8_t dma_ctrl);
bool dma_ignore_src_reads_is_enabled(uint8_t dma_ctrl);
bool dma_get_dreq_calls_src_reads(uint8_t dma_ctrl);
bool dma_src_read_128bit_is_enabled(uint8_t dma_ctrl);
bool dma_src_addr_inc_is_enabled(uint8_t dma_ctrl);
bool dma_ignore_dst_writes_is_enabled(uint8_t dma_ctrl);
bool dma_get_dreq_calls_dst_writes(uint8_t dma_ctrl);
bool dma_dst_write_128bit_is_enabled(uint8_t dma_ctrl);
bool dma_dst_addr_inc_is_enabled(uint8_t dma_ctrl);
bool dma_wait_write_response_is_enabled(uint8_t dma_ctrl);
bool dma_tdmode_is_enabled(uint8_t dma_ctrl);
bool dma_intr_is_enabled(uint8_t dma_ctrl);
uint32_t dma_get_src_addr_phys(uint8_t dma_ctrl);
uint32_t dma_get_dst_addr_phys(uint8_t dma_ctrl);
uint32_t dma_get_transfer_length_bytes(uint8_t dma_ctrl);
uint32_t dma_get_transfer_length_ext(uint8_t dma_ctrl);
uint32_t dma_get_src_stride(uint8_t dma_ctrl);
uint32_t dma_get_dst_stride(uint8_t dma_ctrl);
uint32_t dma_get_next_ctrlblock_addr_phys(uint8_t dma_ctrl);
bool dma_debug_is_type_lite(uint8_t dma_ctrl);
uint32_t dma_debug_get_version(uint8_t dma_ctrl);
uint32_t dma_debug_get_state(uint8_t dma_ctrl);
uint32_t dma_debug_get_id(uint8_t dma_ctrl);
uint32_t dma_debug_get_ostd_writes_counter(uint8_t dma_ctrl);
bool dma_debug_get_read_error(uint8_t dma_ctrl);
bool dma_debug_get_fifo_error(uint8_t dma_ctrl);
bool dma_debug_readlast_not_set_error(uint8_t dma_ctrl);

void dma_disable_wide_bursts(dma_ctrlblock_t *p_ctrlblock, bool disable);
void dma_set_wait_cycles(dma_ctrlblock_t *p_ctrlblock, uint8_t wait_cycles);
void dma_set_permap(dma_ctrlblock_t *p_ctrlblock, uint8_t permap);
void dma_set_burst_length(dma_ctrlblock_t *p_ctrlblock, uint8_t burst_length);
void dma_enable_ignore_src_reads(dma_ctrlblock_t *p_ctrlblock, bool enable);
void dma_set_dreq_calls_src_reads(dma_ctrlblock_t *p_ctrlblock, bool enable);
void dma_enable_src_read_128bit_width(dma_ctrlblock_t *p_ctrlblock, bool enable);
void dma_enable_src_addr_inc(dma_ctrlblock_t *p_ctrlblock, bool enable);
void dma_enable_ignore_dst_writes(dma_ctrlblock_t *p_ctrlblock, bool enable);
void dma_set_dreq_calls_dst_writes(dma_ctrlblock_t *p_ctrlblock, bool enable);
void dma_enable_dst_write_128bit_width(dma_ctrlblock_t *p_ctrlblock, bool enable);
void dma_enable_dst_addr_inc(dma_ctrlblock_t *p_ctrlblock, bool enable);
void dma_enable_wait_write_response(dma_ctrlblock_t *p_ctrlblock, bool enable);
void dma_enable_tdmode(dma_ctrlblock_t *p_ctrlblock, bool enable);
void dma_enable_intr(dma_ctrlblock_t *p_ctrlblock, bool enable);
void dma_set_src_addr_phys(dma_ctrlblock_t *p_ctrlblock, uint32_t addr);
void dma_set_src_addr_virt(dma_ctrlblock_t *p_ctrlblock, void *p);
void dma_set_dst_addr_phys(dma_ctrlblock_t *p_ctrlblock, uint32_t addr);
void dma_set_dst_addr_virt(dma_ctrlblock_t *p_ctrlblock, void *p);
void dma_set_transfer_length_bytes(dma_ctrlblock_t *p_ctrlblock, uint16_t length);
void dma_set_transfer_length_ext(dma_ctrlblock_t *p_ctrlblock, uint16_t length);
void dma_set_src_stride(dma_ctrlblock_t *p_ctrlblock, uint16_t stride);
void dma_set_dst_stride(dma_ctrlblock_t *p_ctrlblock, uint16_t stride);
void dma_set_next_ctrlblock_addr_phys(dma_ctrlblock_t *p_ctrlblock, uint32_t addr);
void dma_set_next_ctrlblock_addr_virt(dma_ctrlblock_t *p_ctrlblock, void *p);

#endif
