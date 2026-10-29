#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int tests_run = 0;
static int tests_failed = 0;

#define CHECK(cond)                                                          \
    do {                                                                     \
        tests_run++;                                                         \
        if(!(cond)) {                                                        \
            tests_failed++;                                                  \
            fprintf(                                                         \
                stderr, "%s:%d: CHECK failed: %s\n", __FILE__, __LINE__, #cond); \
        }                                                                    \
    } while(0)

#define CHECK_EQ_INT(expected, actual)                                       \
    do {                                                                     \
        long long e_ = (long long)(expected);                                \
        long long a_ = (long long)(actual);                                  \
        tests_run++;                                                         \
        if(e_ != a_) {                                                       \
            tests_failed++;                                                  \
            fprintf(                                                         \
                stderr, "%s:%d: expected %lld, got %lld\n", __FILE__, __LINE__, e_, a_); \
        }                                                                    \
    } while(0)

#define CHECK_EQ_STR(expected, actual)                                       \
    do {                                                                     \
        const char* e_ = (expected);                                         \
        const char* a_ = (actual);                                           \
        tests_run++;                                                         \
        if(!e_ || !a_ || strcmp(e_, a_) != 0) {                              \
            tests_failed++;                                                  \
            fprintf(                                                         \
                stderr, "%s:%d: expected \"%s\", got \"%s\"\n", __FILE__, __LINE__, \
                e_ ? e_ : "(null)", a_ ? a_ : "(null)");                     \
        }                                                                    \
    } while(0)

#define TEST_REPORT()                                                        \
    do {                                                                     \
        printf("%d checks, %d failed\n", tests_run, tests_failed);           \
        return tests_failed ? 1 : 0;                                         \
    } while(0)
