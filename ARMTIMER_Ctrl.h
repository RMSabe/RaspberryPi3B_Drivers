#ifndef ARMTIMER_CTRL_H
#define ARMTIMER_CTRL_H

#include <stdbool.h>
#include <stdint.h>

#define ARMTIMER_TIMER_PRESCALE_NONE 0
#define ARMTIMER_TIMER_PRESCALE_CLKDIV16 1
#define ARMTIMER_TIMER_PRESCALE_CLKDIV256 2

#define ARMTIMER_COUNTSIZE_16BITS 0
#define ARMTIMER_COUNTSIZE_32BITS 1

bool armtimer_is_active(void);
bool armtimer_init(void);
void armtimer_set_load_value(uint32_t value);
uint32_t armtimer_get_load_value(void);
uint32_t armtimer_get_countdown_value(void);
void armtimer_set_freerun_counter_prescale(uint32_t prescale);
uint32_t armtimer_get_freerun_counter_prescale(void);
void armtimer_enable_freerun_counter(bool enable);
bool armtimer_freerun_counter_is_enabled(void);
void armtimer_enable_debug_halt_timer(bool enable);
bool armtimer_debug_halt_timer_is_enabled(void);
void armtimer_enable_timer(bool enable);
bool armtimer_timer_is_enabled(void);
void armtimer_enable_intr(bool enable);
bool armtimer_intr_is_enabled(void);
void armtimer_set_timer_prescale(uint32_t prescale);
uint32_t armtimer_get_timer_prescale(void);
void armtimer_set_countsize_bits(bool countsize);
bool armtimer_get_countsize_bits(void);
void armtimer_clear_intr_flags(void);
bool armtimer_get_raw_intr_status(void);
bool armtimer_get_masked_intr_status(void);
void armtimer_set_reload_value(uint32_t value);
uint32_t armtimer_get_reload_value(void);
void armtimer_set_predivider_value(uint32_t value);
uint32_t armtimer_get_predivider_value(void);
uint32_t armtimer_get_freerun_counter_value(void);

#endif
