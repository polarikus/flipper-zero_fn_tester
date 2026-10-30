// Минимальный набор проверок для host-тестов (без внешних зависимостей).
//
// Проверки не прерывают тест: выводятся все падения, код возврата main()
// формируется через TEST_SUMMARY().

#pragma once

#include <stdio.h>

static unsigned test_checks = 0;
static unsigned test_failures = 0;

#define TEST_CHECK(cond)                                                    \
    do {                                                                    \
        test_checks++;                                                      \
        if(!(cond)) {                                                       \
            test_failures++;                                                \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        }                                                                   \
    } while(0)

#define TEST_CHECK_EQ_U16(actual, expected) TEST_CHECK((unsigned)(actual) == (unsigned)(expected))

#define TEST_CHECK_EQ_U32(actual, expected) \
    TEST_CHECK((unsigned long)(actual) == (unsigned long)(expected))

// Итог прогона: 0 — все проверки прошли, 1 — есть падения.
#define TEST_SUMMARY()                                                              \
    (printf("%s: проверок %u, падений %u\n", __FILE__, test_checks, test_failures), \
     test_failures == 0 ? 0 : 1)
