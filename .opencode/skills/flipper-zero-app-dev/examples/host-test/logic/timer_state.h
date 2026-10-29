#pragma once

#include <stdint.h>
#include <stdbool.h>

/** Чистая логика: без furi.h, без HAL — тестируется на хосте. */
typedef struct {
    uint32_t total_seconds;
    bool running;
} TimerState;

bool timer_state_add(TimerState* state, uint32_t seconds);
bool timer_state_tick(TimerState* state);
