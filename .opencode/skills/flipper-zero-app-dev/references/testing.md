# Тестирование приложений

## Стратегия

Для FAP разумны три уровня:

1. **Хостовые юнит-тесты чистой логики** (обязательны для парсеров, протоколов,
   конечных автоматов, форматирования). Собираются обычным `cc` на macOS/Linux,
   работают за миллисекунды, не требуют Flipper.
2. **On-device smoke/интеграционные проверки** — отдельный debug-FAP или скрытый режим
   в приложении: прогон сценариев на реальном железе (SD, UART, IR, NFC).
   UI-сценарии удобно автоматизировать по USB через RPC — см.
   `references/rpc-ui-testing.md` (снимки экрана, нажатия, файлы на SD).
3. **Ручные чек-листы** для UI и взаимодействия с железом (вставка/извлечение SD,
   разряд батареи, занятость портов, выход по Back в каждом состоянии).

Правило: **логику отделяй от Furi**, чтобы её можно было тестировать без устройства.
Функции, зависящие от `furi_*`, выноси в адаптеры и подменяй через указатели на функции
или `#ifdef`.

## 1. Хостовые тесты (рекомендуется)

Структура:

```
my_app/
├── logic/
│   ├── my_logic.c
│   └── my_logic.h        # без furi.h, только stdint/stdbool/stddef
├── tests/
│   ├── test_main.c
│   └── test_logic.c
├── Makefile              # или CMake
└── application.fam
```

`my_logic.h` — чистый C:

```c
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct {
    uint32_t total_seconds;
    bool running;
} TimerState;

bool timer_state_add(TimerState* state, uint32_t seconds);
bool timer_state_tick(TimerState* state);
```

`my_logic.c`:

```c
#include "my_logic.h"

bool timer_state_add(TimerState* state, uint32_t seconds) {
    if(!state) return false;
    if(UINT32_MAX - state->total_seconds < seconds) return false;
    state->total_seconds += seconds;
    return true;
}

bool timer_state_tick(TimerState* state) {
    if(!state || !state->running) return false;
    if(state->total_seconds == 0) {
        state->running = false;
        return false;
    }
    state->total_seconds--;
    return true;
}
```

Минимальный тестовый каркас (без внешних зависимостей) — `tests/test.h`:

```c
#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int tests_run = 0;
static int tests_failed = 0;

#define CHECK(cond)                                                           \
    do {                                                                      \
        tests_run++;                                                          \
        if(!(cond)) {                                                         \
            tests_failed++;                                                   \
            fprintf(stderr, "%s:%d: CHECK failed: %s\n", __FILE__, __LINE__,  \
                    #cond);                                                   \
        }                                                                     \
    } while(0)

#define CHECK_EQ_INT(expected, actual)                                        \
    do {                                                                      \
        long long e_ = (long long)(expected);                                 \
        long long a_ = (long long)(actual);                                   \
        tests_run++;                                                          \
        if(e_ != a_) {                                                        \
            tests_failed++;                                                   \
            fprintf(stderr, "%s:%d: expected %lld, got %lld\n", __FILE__,     \
                    __LINE__, e_, a_);                                        \
        }                                                                     \
    } while(0)

#define CHECK_EQ_STR(expected, actual)                                        \
    do {                                                                      \
        const char* e_ = (expected);                                          \
        const char* a_ = (actual);                                            \
        tests_run++;                                                          \
        if(!e_ || !a_ || strcmp(e_, a_) != 0) {                               \
            tests_failed++;                                                   \
            fprintf(stderr, "%s:%d: expected \"%s\", got \"%s\"\n", __FILE__, \
                    __LINE__, e_ ? e_ : "(null)", a_ ? a_ : "(null)");        \
        }                                                                     \
    } while(0)

#define TEST_REPORT()                                                         \
    do {                                                                      \
        printf("%d checks, %d failed\n", tests_run, tests_failed);            \
        return tests_failed ? 1 : 0;                                          \
    } while(0)
```

`tests/test_logic.c`:

```c
#include "test.h"
#include "../logic/my_logic.h"

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
    test_tick();
    test_tick_not_running();
    TEST_REPORT();
}
```

`Makefile`:

```make
CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Werror -O2 -g
SAN ?= -fsanitize=address,undefined

SRC := logic/my_logic.c
TEST_SRC := tests/test_logic.c

.PHONY: test clean
test:
	$(CC) $(CFLAGS) $(SAN) $(SRC) $(TEST_SRC) -o build_tests
	./build_tests

clean:
	rm -f build_tests
```

Запуск: `make test`. ASan/UBSan ловят выход за границы и UB — это дешёвый способ
найти баги, которые на устройстве приведут к крашу.

Что тестировать:

- границы (0, максимум, переполнение), пустые входы, NULL;
- кодирование/декодирование (round-trip), парсеры бинарных форматов;
- конечные автоматы: все переходы, включая «неверный» порядок вызовов;
- форматирование строк (`snprintf`-буферы, обрезка);
- обработку ошибок (open/read/write, неверные данные);
- владение памятью: кто освобождает (можно проверить ASan/LSan).

## 2. On-device тесты в стиле прошивки (minunit)

Прошивка содержит приложение `unit_tests` с фреймворком **minunit**:
`applications/debug/unit_tests/tests/minunit.h`. Тесты — плагины (`FlipperAppType.PLUGIN`),
которые запускаются в общем раннере.

Сборка и запуск (для прошивки, не для ufbt-приложения):

```bash
./fbt FIRMWARE_APP_SET=unit_tests updater_package
# прошить прошивку + ресурсы, затем в CLI:
unit_tests              # все тесты
unit_tests test_furi    # только конкретный набор
```

Каркас теста (упрощённо):

```c
#include <furi.h>
#include "../test.h"   // minunit + test_api.h

static void test_setup(void) { /* ... */ }
static void test_teardown(void) { /* ... */ }

MU_TEST(test_message_queue_basic) {
    FuriMessageQueue* q = furi_message_queue_alloc(4, sizeof(uint32_t));
    uint32_t value = 42;
    mu_assert_int_eq(FuriStatusOk, furi_message_queue_put(q, &value, 0));
    uint32_t out = 0;
    mu_assert_int_eq(FuriStatusOk, furi_message_queue_get(q, &out, 0));
    mu_assert_int_eq(42, out);
    furi_message_queue_free(q);
}

MU_TEST_SUITE(test_suite) {
    MU_SUITE_CONFIGURE(&test_setup, &test_teardown);
    MU_RUN_TEST(test_message_queue_basic);
}

int run_minunit_test_my_module(void) {
    MU_RUN_SUITE(test_suite);
    return MU_EXIT_CODE;
}

TEST_API_DEFINE(run_minunit_test_my_module)
```

`TEST_API_DEFINE` объявляет `get_api()` и `FlipperAppPluginDescriptor` — плагин
подхватывается раннером. В `application.fam` прошивки добавляется:

```python
App(
    appid="test_my_module",
    sources=["tests/common/*.c", "tests/my_module/*.c"],
    apptype=FlipperAppType.PLUGIN,
    entry_point="get_api",
    requires=["unit_tests"],
)
```

Основные макросы minunit:

| Макрос | Проверка |
|---|---|
| `mu_check(cond)` | истинность |
| `mu_assert(cond, msg)` | истинность с сообщением |
| `mu_assert_int_eq(expected, actual)` | целые |
| `mu_assert_int_not_eq`, `mu_assert_int_greater_than`, `mu_assert_int_between`, `mu_assert_int_in` | сравнения |
| `mu_assert_string_eq(expected, actual)` | строки |
| `mu_assert_mem_eq(expected, actual, size)` | память |
| `mu_assert_null`, `mu_assert_not_null` | указатели |
| `mu_assert_double_eq`, `mu_assert_double_between` | float |
| `MU_TEST(name)`, `MU_TEST_SUITE(name)`, `MU_RUN_TEST`, `MU_RUN_SUITE` | структура |
| `MU_SUITE_CONFIGURE(setup, teardown)` | setup/teardown |

Тестовые данные (ассеты) кладутся в
`applications/debug/unit_tests/resources/unit_tests/<module>/`.

Для собственного приложения можно сделать аналогичный debug-FAP: `apptype=EXTERNAL`,
`fap_category="Debug"`, который запускает те же `MU_*` тесты и пишет результат в лог,
а завершается с кодом `MU_EXIT_CODE`. Запуск: `ufbt launch`, результат — в `ufbt cli` → `log`.

## 3. Тестирование с железом

- Проверяй занятость ресурсов: запусти приложение при активном CLI/UART/IR-приложении —
  должно быть понятное сообщение, а не зависание.
- Проверь извлечение SD во время работы, низкий заряд, выход по Back на каждом экране,
  повторный запуск приложения (нет ли утечек/мусора).
- Для протоколов используй записанные сигналы/дампы и проверяй round-trip
  (encode → decode → сравнение).
- Логируй ключевые события с `TAG`, но не спамь в горячем пути.

## 4. CI

Минимальный CI для FAP:

```yaml
# .github/workflows/build.yml (пример)
name: build
on: [push, pull_request]
jobs:
  host-tests:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - run: make -C tests test        # или ваш Makefile
  fap:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - uses: flipperdevices/flipperzero-ufbt-action@v0.1
        with:
          sdk-channel: release
      - uses: actions/upload-artifact@v4
        with:
          name: fap
          path: dist/*.fap
```

(Точные версии action-ов проверяй в маркетплейсе; для релизов используй
`flipper-application-package-fap`.)

## 5. Что не является тестом

- `furi_check`/`furi_assert` — проверки инвариантов, а не тестовый фреймворк.
- Ручное «нажал кнопку — работает» без сценария — не регрессионный тест.
- Логи без ожидаемого результата — не проверка.
