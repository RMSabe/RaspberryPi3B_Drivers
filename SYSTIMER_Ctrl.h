#ifndef SYSTIMER_CTRL_H
#define SYSTIMER_CTRL_H

#include <stdbool.h>
#include <stdint.h>

//Returns true if "systimer_init()" has already been called.
bool systimer_is_active(void);
//Initializes SYSTIMER procedure.
//This function must be called before calling any other functions in this header.
//Returns true if initialization is successful.
bool systimer_init(void);

bool systimer_timer_match_occurred(uint8_t timer_num);
uint32_t systimer_get_counter_value_l32(void);
uint32_t systimer_get_counter_value_h32(void);
uint64_t systimer_get_counter_value_full(void);
void systimer_set_timer_compare_match_value(uint8_t timer_num, uint32_t value);
uint32_t systimer_get_timer_compare_match_value(uint8_t timer_num);

#endif
