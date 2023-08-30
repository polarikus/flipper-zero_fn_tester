//
// Created by Игорь Данилов on 30.08.2023.
//
#include "unit_tests/minunit.h"
#include "fn_worker_i.h"
#include "fn_objects/fn_info/fn_i.h"
#include "unit_tests/fn_mgm_test_params.h"

FNWorker* fn_worker;

bool is_worker_cmplt = false;

static void test_setup(void) {
    fn_worker = fn_worker_alloc();
}

static void test_teardown(void) {
    fn_worker_free(fn_worker);
}

static bool await_cb()
{
    uint32_t timeout = 5000;

    while (is_worker_cmplt != true || timeout == 0){
        furi_delay_ms(10);
        timeout = timeout - 10;
    }
    is_worker_cmplt = false;

    return timeout;
}

static void fn_worker_test_cb(void* context, FNCustomEventWorker event, FNError fn_error) {
    is_worker_cmplt = true;
    UNUSED(context);
    UNUSED(event);
    UNUSED(fn_error);
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
    mu_assert(furi_thread_get_state(fn_worker->thread) == FuriThreadStateStarting, "FN WORKER not started!");
    fn_worker_stop_thread(fn_worker);
    mu_assert(furi_thread_get_state(fn_worker->thread) == FuriThreadStateStopped, "FN WORKER not stopped!");
}

MU_TEST(fn_worker_fn_detect_start_test)
{
    FNInfo* fn_info = malloc(sizeof(FNInfo));
    fn_info->fw_version.fw_mode = 1;
    fn_info->fn_state = 0;
    fn_worker_start_thread(fn_worker);

    fn_worker_fn_detect_start(fn_info, fn_worker, fn_worker_test_cb, NULL);
    mu_assert(await_cb(), "await_cb timeout!!!");
    mu_assert_string_eq(MGM_SN, fn_get_sn(fn_info));
    mu_assert_string_eq(MGM_FW, fn_get_fw_version(fn_info));
    mu_assert_int_between((int)FNStage1, (int)FNStage4, (int)fn_get_fn_state_enum(fn_info));
    mu_assert_int_between((int)FFD_1_05, (int)FFD_1_2, (int)fn_get_max_ffd_enum(fn_info));
    mu_assert_int_between((int)FFD_NONE, (int)FFD_1_2, (int)fn_get_ffd_enum(fn_info));

    mu_assert(fn_is_mgm(fn_info), "FN not MGM!!");

    fn_worker_stop_thread(fn_worker);
    free(fn_info);
}

//TODO LifeInfo тест. Просто нужно сначала починить lifeInfo. А то сейчас если МГМ чистый получаем таймаут
//TODO Добавить тест flashMGM, просто нужно еще написать много сервисов для реги ФН и подключать для тестов чистый МГМ. И флеш делать в конце всех тестов

MU_TEST_SUITE(fn_worker_suite)
{
    MU_SUITE_CONFIGURE(test_setup, test_teardown);

    MU_RUN_TEST(fn_worker_alloc_test);
    MU_RUN_TEST(fn_worker_start_and_stop_thread_test);
    MU_RUN_TEST(fn_worker_fn_detect_start_test);
}

int run_minunit_test_fn_worker() {
    MU_RUN_SUITE(fn_worker_suite);
    return MU_EXIT_CODE;
}
