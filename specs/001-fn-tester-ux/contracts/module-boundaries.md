# Контракт: границы модулей и правила зависимостей

**Цель**: удержать слоистую архитектуру (принцип II конституции) после
рефакторинга и не дать UI снова «протечь» в протокол.

## 1. Разрешённые зависимости

Строки — модуль, столбцы — что он может подключать.

| Модуль | `core/` | `proto/` | `transport/` | `worker` | `ui/` | `furi`/HAL | `gui` | `expansion` |
|---|---|---|---|---|---|---|---|---|
| `core/*` | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| `proto/*` | ✅ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| `transport/*` | ✅ | ❌ | ✅ | ❌ | ❌ | ✅ | ❌ | ✅ |
| `fn_worker` | ✅ | ✅ | ✅ | ✅ | ❌ | ✅ | ❌ | ❌ |
| `ui/*` | ✅ (только типы) | ❌ | ❌ | ✅ (публичный API) | ✅ | ✅ | ✅ | ❌ |
| `fn_test_app.c` | ✅ (типы) | ❌ | ❌ | ✅ | ✅ | ✅ | ✅ | ❌ |

Дополнительно:
- `core/` и `proto/` **не включают** `furi.h`, `furi_hal_*.h`, `gui/*`.
  Логирование в них отсутствует; диагностика — на уровне `worker`/`transport`.
- Исключение внутри `ui/`: `fn_strings.h`, `fn_text_model.h`, `fn_format.c` —
  GUI-независимая часть (`gui` ❌, `furi` ❌), её и только её компилирует
  host-сборка; `fn_format` заполняет `FnTextModel`, а не вызывает методы view.
- `ui/` не включает `proto/*` и `transport/*`; данные приходят только через
  `FnInfo`/`FnLifeInfo`/`FnOpResult`.
- `lib/fn` не знает про `FNApp`, сцены и `ViewDispatcher`.
- Коды команд (`FN_CMD*`) используются только в `fn_ops.c`; сцены и view их не
  видят.

## 2. Как это проверяется

1. **Host-сборка**: `tests/host/Makefile` компилирует `core/` + `proto/` и
   GUI-независимые файлы UI (`fn_strings.h`, `fn_text_model.h`, `fn_format.c`)
   с `-std=c11 -Wall -Wextra -Werror` и без include-путей SDK. Любой `#include
   <furi.h>` или `gui/…` в этих файлах ломает сборку.
2. **Статическая проверка** (`tests/host/check_layers.sh`, вызывается из
   `make check-layers`):
   - в `lib/fn/core/` и `lib/fn/proto/` нет строк `furi`, `gui/`, `furi_hal`;
   - в `ui/` нет `fn_uart`, `fn_frame`, `fn_parse_`, `fn_wire`, `FN_CMD`;
   - в `ui/fn_format.c`, `ui/fn_text_model.h`, `ui/fn_strings.h` нет `gui/`,
     `view_`, `scene_`;
   - в `lib/fn` нет `fn_test_app`, `scene_`, `view_`.
3. **Ревью-чеклист** (в `docs/protocol.md`): новые команды добавляются только
   в `proto` + `fn_ops`, экраны — только в `ui`.

## 3. Правила владения ресурсами

| Ресурс | Открывает | Закрывает | Условие |
|---|---|---|---|
| `RECORD_GUI` | `fn_test_app_alloc` | `fn_test_app_free` | всегда |
| `RECORD_NOTIFICATION` | `fn_test_app_alloc` | `fn_test_app_free` | всегда |
| `RECORD_EXPANSION` | `transport` (на время захвата USART) | `transport` (при освобождении) | парно, даже при ошибке |
| `furi_hal_serial_control_acquire` | `fn_uart_alloc` | `fn_uart_free` | при `NULL` — ошибка инициализации, не краш |
| `FuriStreamBuffer` (RX) | `fn_uart_alloc` | `fn_uart_free` | ровно один |
| `FuriTimer` | `fn_uart_alloc` | `fn_uart_free` | one-shot |
| `FuriThread` UART | `fn_uart_alloc` | `fn_uart_free` | сначала stop/join |
| `FuriThread` worker | `fn_worker_alloc` | `fn_worker_free` | сначала stop/join, `!busy` |
| `view_dispatcher` | `alloc` | `free` | все view удалены до `free` |
| `FnInfo`/`FnLifeInfo` | `FNApp` | `FNApp` | `*_alloc`/`*_free` |
| `IconAnimation` | view `alloc` | view `free` | `start`/`stop` в enter/exit |

Правило: ошибка на любом шаге `alloc` освобождает уже созданное и возвращает
`NULL`; приложение показывает русское сообщение и завершает работу без краша.

## 4. Соглашения об именах

| Сущность | Префикс | Пример |
|---|---|---|
| Ядро провода | `fn_wire_` | `fn_wire_u16le_get` |
| Кадр | `fn_frame_` | `fn_frame_parse` |
| Парсер | `fn_parse_` | `fn_parse_status_30h` |
| Команды | `FN_CMD` | `FN_CMDGetFNStatus` |
| Воркер | `fn_worker_` | `fn_worker_cancel` |
| Операции | `FnOp` | `FnOpFlashMgm` |
| View | `fn_view_` | `fn_view_wait_set` |
| Сцены | `fn_scene_` | `fn_scene_wait_on_enter` |
| Строки | `FN_STR_` | `FN_STR_NO_RESPONSE` |

Идентификаторы — английские; комментарии, логи, пользовательские строки —
русские. Ошибки компиляции/`furi_check` сообщения содержат `TAG` и имя модуля.
