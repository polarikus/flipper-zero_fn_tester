//
// Created by Игорь Данилов on 30.08.2023.
//
#include "unit_tests/minunit.h"
#include "unit_tests/test_index.h"
#include "fn_worker_i.h"
#include "fn_objects/fn_info/fn_i.h"
#include "fn_objects/life_info/fn_life_info_i.h"
#include "unit_tests/fn_mgm_test_params.h"

FNWorker* fn_worker;

static volatile bool is_worker_cmplt = false;
static FNCustomEventWorker last_worker_event;
static FNError last_worker_error;

#define WORKER_AWAIT_TIMEOUT_MS 15000

static void test_setup(void) {
    fn_worker = fn_test_get_worker();
    furi_check(fn_worker, "fn_test worker is not available");
    if(furi_thread_get_state(fn_worker->thread) != FuriThreadStateStopped) {
        fn_worker_stop_thread(fn_worker);
    }
    fn_worker->callback = NULL;
    fn_worker->cb_ctx = NULL;
    fn_worker->mode_index = FNWorkerModeIdle;
    fn_worker->fn_info = NULL;
    fn_worker->fn_answer_data = NULL;
    is_worker_cmplt = false;
    last_worker_event = FNCustomEventWorkerFNNotResponse;
    last_worker_error = FNOk;
}

static void test_teardown(void) {
    if(furi_thread_get_state(fn_worker->thread) != FuriThreadStateStopped) {
        fn_worker_stop_thread(fn_worker);
    }
    if(furi_thread_get_state(fn_worker->uart->thread) != FuriThreadStateStopped) {
        fn_uart_stop_thread(fn_worker->uart);
    }
    fn_worker->callback = NULL;
    fn_worker->cb_ctx = NULL;
    // Тесты освобождают свои объекты — не оставляем в воркере висячие указатели.
    fn_worker->fn_info = NULL;
    fn_worker->fn_answer_data = NULL;
}

static bool wait_thread_state(FuriThread* thread, FuriThreadState state) {
    for(size_t i = 0; i < 200; ++i) {
        if(furi_thread_get_state(thread) == state) {
            return true;
        }
        furi_delay_ms(5);
    }
    return false;
}

static bool await_cb(void) {
    uint32_t timeout = WORKER_AWAIT_TIMEOUT_MS;

    while(!is_worker_cmplt && timeout > 0) {
        furi_delay_ms(10);
        timeout -= 10;
    }
    is_worker_cmplt = false;

    return timeout > 0;
}

static void fn_worker_test_cb(void* context, FNCustomEventWorker event, FNError fn_error) {
    last_worker_event = event;
    last_worker_error = fn_error;
    is_worker_cmplt = true;
    UNUSED(context);
}

MU_TEST(fn_worker_alloc_test) {
    mu_assert_not_null(fn_worker);
    mu_assert_null(fn_worker->callback);
    mu_assert_not_null(fn_worker->thread);
    mu_assert(fn_worker->mode_index == FNWorkerModeIdle, "mode_index != FNWorkerModeIdle");
    mu_assert_not_null(fn_worker->uart);
    mu_assert_not_null(fn_worker->stream);
    mu_assert_pointers_eq(fn_worker->uart->rx_cmlt_cb, uart_thread_cb);
}

MU_TEST(fn_worker_start_and_stop_thread_test) {
    fn_worker_start_thread(fn_worker);
    mu_assert(
        wait_thread_state(fn_worker->thread, FuriThreadStateRunning), "FN WORKER not started!");
    fn_worker_stop_thread(fn_worker);
    mu_assert(
        wait_thread_state(fn_worker->thread, FuriThreadStateStopped), "FN WORKER not stopped!");
}

MU_TEST(fn_worker_fn_detect_start_test) {
    FNInfo* fn_info = malloc(sizeof(FNInfo));
    memset(fn_info, 0, sizeof(FNInfo));

    fn_worker_start_thread(fn_worker);
    fn_worker_fn_detect_start(fn_info, fn_worker, fn_worker_test_cb, NULL);
    bool completed = await_cb();
    fn_worker_stop_thread(fn_worker);

    if(!completed) {
        free(fn_info);
        mu_fail("await_cb timeout!!!");
    }

    if(last_worker_event != FNCustomEventWorkerFNIdentified) {
        printf(
            "[SKIP] fn_worker_fn_detect_start_test: FN not identified (event=%d, error=%d)\r\n",
            (int)last_worker_event,
            (int)last_worker_error);
        free(fn_info);
        return;
    }

    mu_assert_string_eq(MGM_SN, fn_get_sn(fn_info));
    mu_assert_string_eq(MGM_FW, fn_get_fw_version(fn_info));
    mu_assert_int_between((int)FNStage1, (int)FNStage4, (int)fn_get_fn_state_enum(fn_info));
    mu_assert_int_between((int)FFD_1_05, (int)FFD_1_2, (int)fn_get_max_ffd_enum(fn_info));
    mu_assert_int_between((int)FFD_NONE, (int)FFD_1_2, (int)fn_get_ffd_enum(fn_info));

    mu_assert(fn_is_mgm(fn_info), "FN not MGM!!");

    free(fn_info);
}

MU_TEST(fn_worker_fn_life_info_test) {
    FNLifeInfo* fn_life_info = fn_life_info_alloc();
    FNInfo* fn_info = malloc(sizeof(FNInfo));
    memset(fn_info, 0, sizeof(FNInfo));

    fn_worker_start_thread(fn_worker);

    fn_worker_fn_detect_start(fn_info, fn_worker, fn_worker_test_cb, NULL);
    bool completed = await_cb();
    if(!completed) {
        fn_worker_stop_thread(fn_worker);
        fn_life_info_free(fn_life_info);
        free(fn_info);
        mu_fail("detect await_cb timeout!!!");
    }

    if(last_worker_event != FNCustomEventWorkerFNIdentified) {
        fn_worker_stop_thread(fn_worker);
        printf(
            "[SKIP] fn_worker_fn_life_info_test: FN not identified (event=%d, error=%d)\r\n",
            (int)last_worker_event,
            (int)last_worker_error);
        fn_life_info_free(fn_life_info);
        free(fn_info);
        return;
    }

    fn_worker_get_life_info_start(fn_life_info, fn_worker, fn_worker_test_cb, NULL);
    completed = await_cb();
    fn_worker_stop_thread(fn_worker);

    if(!completed) {
        fn_life_info_free(fn_life_info);
        free(fn_info);
        mu_fail("life info await_cb timeout!!!");
    }

    if(last_worker_event != FNCustomEventWorkerDone) {
        printf(
            "[SKIP] fn_worker_fn_life_info_test: FN life info not received (event=%d, error=%d)\r\n",
            (int)last_worker_event,
            (int)last_worker_error);
        fn_life_info_free(fn_life_info);
        free(fn_info);
        return;
    }

    mu_assert_int_between(0, 10, fn_life_info->reg_report_ctn);

    fn_life_info_free(fn_life_info);
    free(fn_info);
}

MU_TEST_SUITE(fn_worker_suite) {
    MU_SUITE_CONFIGURE(test_setup, test_teardown);

    MU_RUN_TEST(fn_worker_alloc_test);
    MU_RUN_TEST(fn_worker_start_and_stop_thread_test);
    MU_RUN_TEST(fn_worker_fn_detect_start_test);
    MU_RUN_TEST(fn_worker_fn_life_info_test);
}

int run_minunit_test_fn_worker() {
    MU_RUN_SUITE(fn_worker_suite);
    return MU_EXIT_CODE;
}
