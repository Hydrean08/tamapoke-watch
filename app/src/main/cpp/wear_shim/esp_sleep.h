#pragma once

#include <cstdint>

static inline void esp_sleep_enable_timer_wakeup(uint64_t) {}
static inline void esp_sleep_enable_gpio_wakeup() {}
static inline void esp_light_sleep_start() {}
