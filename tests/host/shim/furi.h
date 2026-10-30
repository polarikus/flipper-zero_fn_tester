// Минимальная заглушка <furi.h> для host-теста-«ловушки» этапа R0.
//
// Нужна только чтобы скомпилировать старые helper'ы
// (lib/fn/fn_helpers.c) на хосте и зафиксировать их фактическое поведение.
// После этапа R1 вместе с helper'ами удаляется (задача T014).
//
// Заглушка НЕ даёт доступа к SDK: любые попытки использовать настоящий
// Furi API приведут к ошибке компиляции.

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UNUSED(x) (void)(x)

#ifndef COUNT_OF
#define COUNT_OF(x) (sizeof(x) / sizeof((x)[0]))
#endif

// В логах хоста нет; макросы оставлены пустыми, чтобы код компилировался.
#define FURI_LOG_D(tag, ...) \
    do {                     \
        UNUSED(tag);         \
    } while(0)
#define FURI_LOG_I(tag, ...) FURI_LOG_D(tag)
#define FURI_LOG_W(tag, ...) FURI_LOG_D(tag)
#define FURI_LOG_E(tag, ...) FURI_LOG_D(tag)
#define FURI_LOG_T(tag, ...) FURI_LOG_D(tag)

// На устройстве это аварийное завершение приложения; на хосте — abort().
static inline void furi_crash(const char* message) {
    fprintf(stderr, "furi_crash: %s\n", message ? message : "(без сообщения)");
    abort();
}
