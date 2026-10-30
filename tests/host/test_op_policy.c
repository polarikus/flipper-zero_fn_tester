// Host-тесты политики операций воркера (T066, FR-015, FR-023).
//
// Проверяется чистая политика «какие операции отменяемы»: очистка МГМ —
// единственная неотменяемая операция. Проверка не требует SDK и не отправляет
// Проверка не требует SDK и не отправляет команды в ФН.
#include "../../lib/fn/fn_worker.h"
#include "test_common.h"

#include <stdio.h>

static void test_cancellable(void) {
    TEST_CHECK(fn_op_is_cancellable(FnOpDetect));
    TEST_CHECK(fn_op_is_cancellable(FnOpReadInfo));
    TEST_CHECK(fn_op_is_cancellable(FnOpReadLife));
}

static void test_flash_not_cancellable(void) {
    TEST_CHECK(!fn_op_is_cancellable(FnOpFlashMgm));
}

int main(void) {
    test_cancellable();
    test_flash_not_cancellable();
    return TEST_SUMMARY();
}
