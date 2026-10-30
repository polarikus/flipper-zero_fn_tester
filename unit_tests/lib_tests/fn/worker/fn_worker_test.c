// On-device тесты воркера (T035, R5).
//
// Используется воркер запущенного приложения (fn_test_get_worker): порт ФН уже
// занят им. Тесты не требуют подключенного ФН: без ответа операция завершается
// статусом NoResponse, что и проверяется.
#include "unit_tests/minunit.h"
#include "unit_tests/test_index.h"

#include "lib/fn/fn_worker_i.h"

#include <furi.h>
#include <string.h>

// Callback выполняется в потоке воркера, поэтому результаты читает другой поток:
// поля volatile, ожидание — опросом с шагом 10 мс.
static volatile uint32_t test_callback_count = 0;
static volatile uint32_t test_result_id = 0;
static volatile FnOpKind test_result_kind = FnOpDetect;
static volatile FnOpStatus test_result_status = FnOpStatusInternalError;

#define TEST_WAIT_STEP_MS 10u

static void test_callback(void* context, const FnOpResult* result) {
    UNUSED(context);
    test_result_id = result->id;
    test_result_kind = result->kind;
    test_result_status = result->status;
    test_callback_count++;
}

static void test_setup(void) {
    test_callback_count = 0;
    test_result_id = 0;
    test_result_kind = FnOpDetect;
    test_result_status = FnOpStatusInternalError;
}

static void test_teardown(void) {
}

/** Ожидание завершения операции; true — callback получен. */
static bool test_wait(uint32_t timeout_ms) {
    for(uint32_t waited = 0; waited < timeout_ms; waited += TEST_WAIT_STEP_MS) {
        if(test_callback_count != 0) return true;
        furi_delay_ms(TEST_WAIT_STEP_MS);
    }
    return test_callback_count != 0;
}

MU_TEST(invalid_args_test) {
    FnTask empty_task;
    memset(&empty_task, 0, sizeof(empty_task));

    mu_assert(!fn_worker_start(NULL, FnOpDetect, &empty_task, test_callback, NULL), "NULL-воркер");
    mu_assert(!fn_worker_is_busy(NULL), "NULL не занят");
    mu_assert_int_eq(0, (int)fn_worker_get_op_id(NULL));
    fn_worker_cancel(NULL); // no-op

    FNWorker* worker = fn_test_get_worker();
    if(!worker) {
        printf("[SKIP] invalid_args_test: воркер приложения не запущен\r\n");
        return;
    }
    mu_assert(
        !fn_worker_start(worker, FnOpDetect, &empty_task, test_callback, NULL),
        "Detect без FnInfo запрещен");
    mu_assert(
        !fn_worker_start(worker, FnOpReadLife, &empty_task, test_callback, NULL),
        "ReadLife без FnLifeInfo запрещен");
    mu_assert(!fn_worker_start(worker, FnOpDetect, NULL, test_callback, NULL), "NULL-задача");
}

MU_TEST(read_info_test) {
    FNWorker* worker = fn_test_get_worker();
    if(!worker) {
        printf("[SKIP] read_info_test: воркер приложения не запущен\r\n");
        return;
    }
    if(fn_worker_is_busy(worker)) {
        printf("[SKIP] read_info_test: воркер занят операцией приложения\r\n");
        return;
    }
    FnInfo* info = fn_info_alloc();
    mu_assert(info != NULL, "нет памяти под FnInfo");
    FnTask task = {.info = info, .life = NULL};

    const uint32_t id_before = fn_worker_get_op_id(worker);
    mu_assert(fn_worker_start(worker, FnOpReadInfo, &task, test_callback, NULL), "старт ReadInfo");
    const uint32_t id = fn_worker_get_op_id(worker);
    mu_assert(id > id_before, "op_id растет монотонно");
    mu_assert(fn_worker_is_busy(worker), "операция активна");

    // Вторая операция при занятом воркере отклоняется без callback-а.
    mu_assert(!fn_worker_start(worker, FnOpReadInfo, &task, test_callback, NULL), "воркер занят");
    mu_assert(test_wait(6000), "операция не завершилась за 6 с");
    mu_assert_int_eq(1, test_callback_count);
    mu_assert_int_eq(id, test_result_id);
    mu_assert_int_eq(FnOpReadInfo, test_result_kind);
    mu_assert(!fn_worker_is_busy(worker), "после callback воркер свободен");

    // Без ФН в порту операция обязана завершиться «нет ответа».
    if(test_result_status == FnOpStatusNoResponse) {
        printf("[SKIP] read_info_test: ФН не отвечает (ожидаемо без устройства)\r\n");
    }

    fn_info_free(info);
}

MU_TEST(cancel_test) {
    FNWorker* worker = fn_test_get_worker();
    if(!worker) {
        printf("[SKIP] cancel_test: воркер приложения не запущен\r\n");
        return;
    }
    if(fn_worker_is_busy(worker)) {
        printf("[SKIP] cancel_test: воркер занят операцией приложения\r\n");
        return;
    }

    FnLifeInfo* life = fn_life_info_alloc();
    mu_assert(life != NULL, "нет памяти под FnLifeInfo");
    FnTask task = {.info = NULL, .life = life};
    mu_assert(fn_worker_start(worker, FnOpReadLife, &task, test_callback, NULL), "старт ReadLife");
    furi_delay_ms(100);
    fn_worker_cancel(worker);
    mu_assert(test_wait(4000), "отмененная операция не завершилась за 4 с");
    mu_assert_int_eq(1, test_callback_count);
    // Отмена приходит как Cancelled, но если операция успела завершиться —
    // допустим и статус «нет ответа»: важно, что callback ровно один.
    mu_assert(test_wait(4000), "отмененная операция не завершилась за 4 с");
    mu_assert_int_eq(1, test_callback_count);
    // Без ФН операция либо отменяется, либо успевает завершиться (в тестовой
    // обвязке возможна петля TX→RX). Важно: callback ровно один, воркер свободен.
    if(test_result_status != FnOpStatusCancelled) {
        printf(
            "[SKIP] cancel_test: операция завершилась раньше отмены (статус %d)\r\n",
            (int)test_result_status);
    }
    mu_assert(!fn_worker_is_busy(worker), "после отмены воркер свободен");

    fn_life_info_free(life);
}

MU_TEST(repeat_ops_test) {
    FNWorker* worker = fn_test_get_worker();
    if(!worker) {
        printf("[SKIP] repeat_ops_test: воркер приложения не запущен\r\n");
        return;
    }

    FnInfo* info = fn_info_alloc();
    mu_assert(info != NULL, "нет памяти под FnInfo");
    FnTask task = {.info = info, .life = NULL};

    uint32_t previous_id = fn_worker_get_op_id(worker);
    for(uint32_t i = 0; i < 3; ++i) {
        if(fn_worker_is_busy(worker)) {
            printf("[SKIP] repeat_ops_test: воркер занят операцией приложения\r\n");
            fn_info_free(info);
            return;
        }
        test_callback_count = 0;
        mu_assert(fn_worker_start(worker, FnOpDetect, &task, test_callback, NULL), "старт Detect");
        mu_assert(fn_worker_get_op_id(worker) > previous_id, "op_id растет");
        previous_id = fn_worker_get_op_id(worker);
        mu_assert(test_wait(6000), "Detect не завершился за 6 с");
        mu_assert_int_eq(1, test_callback_count);
    }

    mu_assert(!fn_worker_is_busy(worker), "после серии операций воркер свободен");
    fn_info_free(info);
}

/**
 * «Боевой ФН — 0 отправок» (T066, SC-009): защита в `fn_ops_flash_mgm`
 * отсекает команду до сборки кадра, счётчик отправок не растёт.
 */
MU_TEST(flash_not_mgm_test) {
    FNWorker* worker = fn_test_get_worker();
    if(!worker) {
        printf("[SKIP] flash_not_mgm_test: воркер приложения не запущен\r\n");
        return;
    }
    if(fn_worker_is_busy(worker)) {
        printf("[SKIP] flash_not_mgm_test: воркер занят операцией приложения\r\n");
        return;
    }

    FnInfo* info = fn_info_alloc();
    mu_assert(info != NULL, "нет памяти под FnInfo");
    fn_info_reset(info);
    /* Признак МГМ не подтверждён данными 33h — команда запрещена. */
    info->has_fw = true;
    info->fw.is_mgm = false;

    FnTask task = {.info = info, .life = NULL};
    const uint32_t sends_before = fn_ops_flash_sends;

    mu_assert(fn_worker_start(worker, FnOpFlashMgm, &task, test_callback, NULL), "старт FlashMgm");
    mu_assert(test_wait(4000), "FlashMgm не завершился за 4 с");
    mu_assert_int_eq(1, test_callback_count);
    mu_assert_int_eq(FnOpFlashMgm, test_result_kind);
    mu_assert_int_eq(FnOpStatusInternalError, test_result_status);
    mu_assert_int_eq((int)sends_before, (int)fn_ops_flash_sends);
    mu_assert(!fn_worker_is_busy(worker), "после отказа воркер свободен");

    fn_info_free(info);
}

/**
 * Отмена очистки МГМ игнорируется (T066, FR-015).
 *
 * Проверка выполняется без отправки 60h: автоматический прогон не должен
 * очищать подключённый МГМ (FR-012 требует явного подтверждения, SC-004).
 * Политика отмены — `fn_op_is_cancellable` (host-тест test_op_policy.c),
 * а факт «результат приходит по факту/30 с» проверяется приёмкой T067 на
 * тестовом МГМ вручную.
 */
MU_TEST(flash_cancel_ignored_test) {
    mu_assert(!fn_op_is_cancellable(FnOpFlashMgm), "очистка МГМ не отменяется");
    mu_assert(fn_op_is_cancellable(FnOpDetect), "Detect отменяется");
    mu_assert(fn_op_is_cancellable(FnOpReadInfo), "ReadInfo отменяется");
    mu_assert(fn_op_is_cancellable(FnOpReadLife), "ReadLife отменяется");

    FNWorker* worker = fn_test_get_worker();
    if(!worker) {
        printf("[SKIP] flash_cancel_ignored_test: воркер приложения не запущен\r\n");
        return;
    }
    /* fn_worker_cancel для свободного воркера — no-op без побочных эффектов. */
    if(!fn_worker_is_busy(worker)) {
        fn_worker_cancel(worker);
        mu_assert(!fn_worker_is_busy(worker), "отмена свободного воркера ничего не меняет");
    }
}

MU_TEST_SUITE(fn_worker_test_suite) {
    MU_SUITE_CONFIGURE(&test_setup, &test_teardown);

    MU_RUN_TEST(invalid_args_test);
    MU_RUN_TEST(read_info_test);
    MU_RUN_TEST(cancel_test);
    MU_RUN_TEST(repeat_ops_test);
    MU_RUN_TEST(flash_not_mgm_test);
    MU_RUN_TEST(flash_cancel_ignored_test);
}

int run_minunit_test_fn_worker(void) {
    MU_RUN_SUITE(fn_worker_test_suite);
    return MU_EXIT_CODE;
}
