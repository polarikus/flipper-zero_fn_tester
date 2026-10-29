#include "timer_state.h"

bool timer_state_add(TimerState* state, uint32_t seconds) {
    if(!state) return false;
    if(UINT32_MAX - state->total_seconds < seconds) return false;
    state->total_seconds += seconds;
    return true;
}

bool timer_state_tick(TimerState* state) {
    if(!state || !state->running) return false;
    if(state->total_seconds == 0) {
        state->running = false;
        return false;
    }
    state->total_seconds--;
    return true;
}
