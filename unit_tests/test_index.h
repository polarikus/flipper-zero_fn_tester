//
// Created by Игорь Данилов on 22.08.2023.
//
#pragma once

#include "../lib/fn/fn_worker.h"

/* Регистрирует CLI-команду `fn_test` (только в сборке с FN_TESTS). */
void fn_register_tests(FNWorker* worker);
void fn_unregister_tests(void);

/* Воркер приложения, который переиспользуют on-device тесты. */
FNWorker* fn_test_get_worker(void);
