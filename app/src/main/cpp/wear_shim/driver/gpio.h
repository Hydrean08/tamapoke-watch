#pragma once

typedef int gpio_num_t;
typedef int gpio_int_type_t;

#define GPIO_INTR_NEGEDGE 2
#define GPIO_INTR_LOW_LEVEL 4
#define GPIO_INTR_HIGH_LEVEL 5

static inline void gpio_wakeup_enable(gpio_num_t, gpio_int_type_t) {}
static inline void gpio_wakeup_disable(gpio_num_t) {}
static inline void gpio_set_intr_type(gpio_num_t, gpio_int_type_t) {}
