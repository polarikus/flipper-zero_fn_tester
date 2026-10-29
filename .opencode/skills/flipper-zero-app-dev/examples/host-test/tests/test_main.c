#include "test.h"
#include "../logic/timer_state.h"

static void test_add(void) {
    TimerState s = {0};
    CHECK(timer_state_add(&s, 30));
    CHECK_EQ_INT(30, s.total_seconds);
    CHECK(timer_state_add(&s, 90));
    CHECK_EQ_INT(120, s.total_seconds);
}

static void test_add_overflow(void) {
    TimerState s = {.total_seconds = UINT32_MAX - 5};
    CHECK(!timer_state_add(&s, 10));
    CHECK_EQ_INT(UINT32_MAX - 5, s.total_seconds);
}

static void test_add_null(void) {
    CHECK(!timer_state_add(NULL, 1));
}

static void test_tick(void) {
    TimerState s = {.total_seconds = 2, .running = true};
    CHECK(timer_state_tick(&s));
    CHECK_EQ_INT(1, s.total_seconds);
    CHECK(timer_state_tick(&s));
    CHECK_EQ_INT(0, s.total_seconds);
    CHECK(!timer_state_tick(&s));
    CHECK(!s.running);
}

static void test_tick_not_running(void) {
    TimerState s = {.total_seconds = 10, .running = false};
    CHECK(!timer_state_tick(&s));
    CHECK_EQ_INT(10, s.total_seconds);
}

int main(void) {
    test_add();
    test_add_overflow();
    test_add_null();
    test_tick();
    test_tick_not_running();
    TEST_REPORT();
}
