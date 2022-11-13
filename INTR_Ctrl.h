#ifndef INTR_CTRL_H
#define INTR_CTRL_H

#include <stdbool.h>
#include <stdint.h>

#define INTR_IRQ_ID_SYSTIMER_MATCH1 1
#define INTR_IRQ_ID_SYSTIMER_MATCH3 3
#define INTR_IRQ_ID_USB_CTRL 9
#define INTR_IRQ_ID_AUX_INTR 29
#define INTR_IRQ_ID_BSCSPI_SLAVE 43
#define INTR_IRQ_ID_PWM0 45
#define INTR_IRQ_ID_PWM1 46
#define INTR_IRQ_ID_SMI 48
#define INTR_IRQ_ID_GPIO_INTR0 49
#define INTR_IRQ_ID_GPIO_INTR1 50
#define INTR_IRQ_ID_GPIO_INTR2 51
#define INTR_IRQ_ID_GPIO_INTR3 52
#define INTR_IRQ_ID_I2C 53
#define INTR_IRQ_ID_SPI 54
#define INTR_IRQ_ID_PCM 55
#define INTR_IRQ_ID_UART 57
#define INTR_IRQ_ID_ARMTIMER 64
#define INTR_IRQ_ID_ARM_MAILBOX 65
#define INTR_IRQ_ID_ARM_DOORBELL0 66
#define INTR_IRQ_ID_ARM_DOORBELL1 67
#define INTR_IRQ_ID_ARM_GPU0_HALT 68
#define INTR_IRQ_ID_ARM_GPU1_HALT 69
#define INTR_IRQ_ID_ARM_ILLEGAL_ACCESS_TYPE1 70
#define INTR_IRQ_ID_ARM_ILLEGAL_ACCESS_TYPE0 71

//Returns true if "intr_init()" has already been called.
bool intr_is_active(void);
//Initializes INTR procedure.
//This function must be called before calling any other functions in this header.
//Returns true if initialization is successful.
bool intr_init(void);

bool intr_basic_irq_occurred(uint8_t irq_id);
bool intr_gpu_irq_occurred(uint8_t irq_id);
void intr_enable_fiq(bool enable);
bool intr_fiq_is_enabled(void);
void intr_set_fiq_src(uint8_t irq_id);
uint8_t intr_get_fiq_src(void);
void intr_enable_gpu_irq(uint8_t irq_id, bool enable);
void intr_enable_basic_irq(uint8_t irq_id, bool enable);

#endif
