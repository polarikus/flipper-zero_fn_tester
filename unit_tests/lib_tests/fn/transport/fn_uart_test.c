// On-device тесты транспорта (T030, R4).
//
// ВАЖНО: порт ФН занимает воркер запущенного приложения. Вызывать второй
// fn_uart_alloc() из теста нельзя: неудачный захват восстанавливает состояние
// службы expansion, а это переинициализирует тот же USART и ломает воркер.
// Поэтому реальный обмен проверяется через воркер (worker/fn_worker_test.c),
// а здесь — инварианты, не трогающие железо.
#include "unit_tests/minunit.h"
#include "unit_tests/test_index.h"

#include "core/fn_frame.h"
#include "transport/fn_uart.h"

#include <string.h>

MU_TEST(null_safety_test) {
    uint8_t buf[16] = {0};
    size_t rx_len = 123;

    mu_assert_int_eq(FnUartTrxError, fn_uart_trx(NULL, buf, 1, buf, sizeof(buf), &rx_len, 100));
    mu_assert_int_eq(0, (int)rx_len);

    // Неполные аргументы — ошибка без обращения к железу.
    FnUartTrxStatus status = fn_uart_trx(NULL, NULL, 0, NULL, 0, NULL, 100);
    mu_assert_int_eq(FnUartTrxError, status);

    FnUart* uart = NULL;
    mu_assert(!fn_uart_is_busy(uart), "NULL-порт не занят");
    fn_uart_cancel(uart); // no-op
    fn_uart_free(uart); // no-op
}

MU_TEST(port_owned_by_app_test) {
    // Порт занимает воркер приложения — это штатное состояние при FN_TESTS=1.
    if(fn_test_get_worker()) {
        printf("[SKIP] port_owned_by_app_test: порт занят воркером приложения\r\n");
        return;
    }

    // Приложение не запущено: проверяем парный захват/освобождение порта.
    FnUart* uart = fn_uart_alloc();
    mu_assert(uart != NULL, "порт должен захватываться, когда свободен");
    mu_assert(!fn_uart_is_busy(uart), "свежий порт свободен");

    uint8_t tx[FN_FRAME_MAX_LEN];
    FnFrameStatus frame_status = FnFrameNull;
    const size_t tx_len = fn_frame_build(0x30, NULL, 0, tx, sizeof(tx), &frame_status);
    mu_assert_int_eq(FnFrameOk, frame_status);

    uint8_t rx[FN_FRAME_MAX_LEN];
    size_t rx_len = 0;
    const FnUartTrxStatus status = fn_uart_trx(uart, tx, tx_len, rx, sizeof(rx), &rx_len, 200);
    if(status == FnUartTrxOk) {
        printf("[SKIP] port_owned_by_app_test: получен ответ, ФН подключен\r\n");
    } else {
        mu_assert_int_eq(FnUartTrxTimeout, status);
        mu_assert_int_eq(0, (int)rx_len);
    }
    mu_assert(!fn_uart_is_busy(uart), "после trx порт свободен");

    fn_uart_free(uart);
}

MU_TEST_SUITE(fn_uart_test_suite) {
    MU_RUN_TEST(null_safety_test);
    MU_RUN_TEST(port_owned_by_app_test);
}

int run_minunit_test_fn_uart(void) {
    MU_RUN_SUITE(fn_uart_test_suite);
    return MU_EXIT_CODE;
}
