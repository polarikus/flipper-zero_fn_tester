---

description: "Task list for 001-fn-tester-ux — русскоязычный интерфейс тестера ФН + полный рефакторинг lib/fn и UI"
---

# Tasks: Русскоязычный интерфейс тестера ФН (полный рефакторинг `lib/fn` и UI)

**Input**: Design documents from `/specs/001-fn-tester-ux/`

**Prerequisites**: `plan.md`, `spec.md`, `research.md`, `data-model.md`,
`contracts/wire-frame.md`, `contracts/fn-api.md`, `contracts/worker-events.md`,
`contracts/ui.md`, `contracts/module-boundaries.md`, `quickstart.md`,
`.specify/memory/constitution.md`

**Tests**: Тесты **обязательны** (конституция, принцип IV — NON-NEGOTIABLE; план
R0–R8 и `contracts/*` требуют host-уровень + уровень устройства). Каждая новая
или изменённая функция библиотеки получает тест, зарегистрированный в реестре;
golden-кадры — общий источник истины для host- и device-уровней.

**Организация**: Задачи сгруппированы по историям пользователя, каждая история
независимо реализуема и проверяема. Этапы R0–R8 из `plan.md` отражены: R0 →
Фаза 1, R1–R5 + базовый UI → Фаза 2, R6 (по историям) → Фазы 3–7, R7–R8 → Фаза 8.
Внутри Фазы 2 порядок строгий: R1–R5 → «Базовый UI» (T037–T047) → R5b/T036
(одним изменением с T077), иначе сборка между задачами красная.

## Формат: `[ID] [P?] [Story] Описание`

- **[P]**: можно выполнять параллельно (разные файлы, нет зависимости от
  незавершённых задач)
- **[Story]**: `[US1]`…`[US5]` — только для фаз историй пользователя
- В каждом описании — точный путь к файлу

## Path Conventions

Репозиторий — одновременно корень FAP `fn_test` (не каталог дерева прошивки):

- `lib/fn/` — приватная библиотека (`core/`, `proto/`, `transport/`, воркер)
- `ui/` — сцены и view (`ui/views/`, `ui/scenes/`)
- `unit_tests/` — on-device minunit (`FN_TESTS=1`)
- `tests/host/` — host-тесты чистого ядра (`cc`, без SDK)
- `icons/` — `fap_icon_assets`
- `docs/`, `.catalog/`, `application.fam` — манифест и документация

Все команды сборки — только `ufbt`; пути указаны от корня репозитория.

---

## Phase 1: Setup (R0 — базовая линия, общая инфраструктура)

**Purpose**: Зафиксировать текущее поведение и создать каркас host-тестов, чтобы
этап R1 (атомарная замена байтового слоя) можно было доказать побайтово.

- [X] T001 Создать `tests/host/Makefile`: цели `test`, `asan`, `check-layers`,
  компиляция `cc -std=c11 -Wall -Wextra -Werror`; include-пути: `lib/fn/core`,
  `lib/fn/proto` и `ui` (только ради `fn_strings.h`, `fn_text_model.h`,
  `fn_format.c`); запрещено включать `ui/views`, `ui/scenes`, `ui/fn_font.*`
  и любые пути SDK; собираются `lib/fn/core/*.c`, `lib/fn/proto/*.c` и
  `ui/fn_format.c`; ненулевой код возврата при любом падении
- [X] T002 [P] Создать `tests/host/shim/furi.h` (минимальная заглушка
  `furi_crash`) и `tests/host/test_wire_legacy.c` — тест-«ловушка»,
  фиксирующий фактическое поведение старых helper'ов:
  `two_uint8t_to_uint16t_LE({0x01,0x02}) == 0x0102` (big-endian),
  `two_uint8t_to_uint16t_BE({0x01,0x02}) == 0x0201` (little-endian),
  `calc_crc16()` переставляет байты в возвращаемом значении
- [X] T003 [P] Перенести golden-кадры из
  `.opencode/skills/fn-protocol/examples/frames.md` в `tests/host/golden_frames.h`
  как `static const` массивы (запросы `30h/31h/32h/33h/3Ah/3Bh/3Dh/60h`, ответы
  `30h` обычный и с `0x80`, `32h`, `3Ah`, `3Bh`, `3Dh` 8/9 байт, отказ `03h`)
- [X] T004 [P] Создать `tests/host/check_layers.sh`: падать, если в
  `lib/fn/core/` и `lib/fn/proto/` встречаются `furi`, `gui/`, `furi_hal`; в
  `ui/` — `fn_uart`, `fn_frame`, `fn_parse_`, `fn_wire`, `FN_CMD`; в `lib/fn` —
  `fn_test_app`, `scene_`, `view_`; в `ui/fn_format.c`, `ui/fn_text_model.h`,
  `ui/fn_strings.h` — дополнительно `gui/`, `view_`, `scene_` (эти файлы
  host-тестируемы и не должны тянуть GUI); в `ui/views/` и `ui/scenes/` —
  `fn_parse_`, `fn_wire`, `FN_CMD`
- [X] T005 Замерить и записать baseline в `specs/001-fn-tester-ux/baseline.md`:
  `free` до запуска и после выхода, `top` (стек UART/воркера), `Leaked` из
  отчёта `units.py`, время до данных US1
- [X] T006 Проверить, что тестовый профиль собирается и работает до рефакторинга:
  `FN_TESTS=1 ufbt` (cdefines из `application.fam`) и
  `FN_TESTS=1 ufbt launch` + `python3 .test_scripts/units.py <NAME>`; зафиксировать
  `Status: PASSED` и текущий список наборов из `unit_tests/test_index.c`

**Checkpoint**: Host-каркас существует, baseline зафиксирован, ловушка
задокументирована тестом.

---

## Phase 2: Foundational (R1–R5 + базовый UI — блокирует ВСЕ истории)

**Purpose**: Чистое ядро (байты, кадр, парсеры, каталог ошибок), транспорт,
воркер и базовые view со шрифтом и русскими строками. Без этой фазы ни одна
история не реализуема.

**⚠️ КРИТИЧНО**: Работы по US1–US5 не начинаются до завершения этой фазы.

### R1. Кадр и байтовый порядок (атомарная замена)

- [X] T007 Создать `lib/fn/core/fn_types.h`: `Date`/`DateTime` (`year` 0–99,
  `month` 1–12 — исправлено с `mouth`, `day` 1–31, `hour` 0–23, `minute` 0–59),
  `FnStage` (`0x01/0x03/0x07/0x0F`, иное → `FnStageUnknown`), `FnFfd`
  (`0/1/3/4`), `FnDocCode` (`0x00–0x17` по таблице `data-model.md` §1.4),
  `FnWarnFlags` (биты `0x01–0x80`, сравнение только по полному значению,
  побитовые проверки этапа не используются)
- [X] T007a [P] Создать `lib/fn/core/fn_types.c`: `fn_date_is_valid()` /
  `fn_datetime_is_valid()` (год 0–99, месяц 1–12, день 1–31, час 0–23,
  минута 0–59; `NULL` → `false`), `fn_stage_text()`, `fn_ffd_text()`
  («1.05» / «1.1» / «1.2» / «Не активизирован»), `fn_doc_text()`
  («Неизвестный документ (0xNN)» для `FnDocUnknown`), `fn_warn_flag_text()`
  (ровно одна маска `0x01–0x40`, иначе `NULL`) и `fn_warn_flags_unknown()`
  (`flags & 0x80`); тексты — статические строки, без `malloc`
- [X] T008 [P] Создать `lib/fn/core/fn_wire.h` и `lib/fn/core/fn_wire.c`:
  `fn_wire_u16le_get/put`, `fn_wire_u32le_get/put`,
  `fn_wire_crc16_ccitt(data, len)` (init `0xFFFF`, полином `0x1021`, MSB-first,
  возвращает **число без перестановки**), `NULL`/`len == 0` — не падение
  (`get` → 0, `put` — no-op); BE-помощников не вводить
- [X] T009 [P] Создать `lib/fn/core/fn_frame.h` и `lib/fn/core/fn_frame.c`:
  `FN_FRAME_MAX_LEN 256`, `FN_FRAME_OVERHEAD 6`, `FN_FRAME_MAX_PAYLOAD 250`,
  `FnFrameStatus` (`Ok/Null/TooShort/TooLong/WrongStart/LengthMismatch/WrongCrc`),
  `FnFrame` (`command`, `code = command & 0x7F`, `has_warning = command & 0x80`,
  `data` — view внутрь `buf`, `data_len`), `fn_frame_build()` в буфер
  вызывающего без `malloc`, `fn_frame_parse()` без копирования; порядок
  проверок: `NULL` → границы → `START` → `LEN == len - 5` → CRC последним
- [X] T010 Написать host-тесты `tests/host/test_types.c`, `tests/host/test_wire.c`,
  `tests/host/test_frame.c`: `fn_date_is_valid`/`fn_datetime_is_valid` на
  границах (месяц 0/13, день 0/32, час 24, минута 60);
  `fn_wire_u16le_get({0x68,0x01}) == 0x0168`; CRC кадра `04 01 00 30` по
  `01 00 30` = `0xCDFF` (LE `FF CD`); побайтовая сборка всех golden-запросов;
  разбор всех golden-ответов; негативные: пустой буфер, `len=5`, `START != 04`,
  `LEN` ±1, битый CRC, `len=257`, граница payload `LEN=250`, round-trip u16/u32
- [X] T011 [P] Создать `unit_tests/lib_tests/fn/core/fn_wire_test.c` (те же
  инварианты CRC/LE на устройстве) и зарегистрировать набор в
  `unit_tests/test_index.c`
- [X] T012 [P] Создать `unit_tests/lib_tests/fn/core/fn_frame_test.c` (golden-
  кадры, усечённые/битые ответы, `has_warning` без ошибки) с регистрацией в
  `unit_tests/test_index.c`
- [X] T013 [P] Создать `unit_tests/lib_tests/fn/core/fn_types_test.c` (валидность
  дат, тексты `fn_stage_text`/`fn_ffd_text`/`fn_doc_text`, `fn_warn_flags_unknown`)
  с регистрацией в `unit_tests/test_index.c`
- [X] T014 Атомарно перевести обмен на новый слой: удалить из
  `lib/fn/fn_helpers.c` и `lib/fn/fn_helpers.h` функции
  `two_uint8t_to_uint16t_LE/BE`, `uint16t_LE/BE_to_uint8t_bytes`,
  `byte_array_to_uint32t_LE`, `add_bytes_to_arr`, `prepend_data_to_array`,
  `calc_crc16`, `check_crc`; перевести `lib/fn/fn_tools.c` и
  `lib/fn/uart/fn_uart.c` на `fn_wire_*`/`fn_frame_*`; удалить
  `tests/host/test_wire_legacy.c`, `tests/host/shim/furi.h` и
  `unit_tests/lib_tests/fn/fn_helpers_test.c` вместе с регистрацией набора в
  `unit_tests/test_index.c` (иначе профиль `FN_TESTS=1` не соберётся)
- [X] T015 Создать `docs/byte-order.md` (версия R1): таблица «старое имя → что
  реально делало → замена», правило «имена описывают провод», список критичных
  мест (LEN, CRC, `3Bh`, `3Dh` u32, `30h` № ФД), запрет «чинить по имени» без
  правки CRC и всех вызовов

### R2. Парсеры с флагами достоверности

- [X] T016 [P] Создать `lib/fn/proto/fn_commands.h`: коды только требуемых команд
  (`30h`, `32h`, `33h`, `3Ah`, `3Bh`, `3Dh`, `10h`, `60h`+`16h`) с указанием
  разделов Протокола v1.2.05; никаких кодов в UI
- [X] T017 [P] Создать `lib/fn/proto/fn_status.h` и `lib/fn/proto/fn_status.c`:
  `fn_parse_status_30h()` — `len >= 30`, `stage` (смещение 0), `current_doc`
  (1), `doc_data_received` (2), `session_open` (3), `warn_flags` (4),
  `last_doc_dt` 5–9, `serial[17]` 10–25 (копировать ровно 16 байт + `'\0'` в
  `[16]`), `last_doc_number` 26–29 LE; невалидная дата (месяц вне 1–12, день вне
  1–31, час ≥24, минута ≥60) → `has_last_doc = false` **и**
  `has_last_doc_number = false`; также `fn_parse_session_10h()` — `len == 5`,
  `session_open`, `shift_number`/`receipt_number` LE
- [X] T018 [P] Создать `lib/fn/proto/fn_ident.h` и `lib/fn/proto/fn_ident.c`:
  `fn_parse_ffd_3ah()` (`len == 2`, `current`/`max`), `fn_parse_fw_33h()`
  (`len == 17`, `version[17]` = 16 байт + `'\0'`, `mode_raw`, `is_mgm =
  (mode_raw == 0)` с комментарием «эвристика проекта»)
- [X] T019 [P] Создать `lib/fn/proto/fn_life.h` и `lib/fn/proto/fn_life.c`:
  `fn_parse_end_date_32h()` (`len == 5`, `Date` 0–2, `year == 0` → «не
  активизирован», `reg_remaining`, `reg_done`), `fn_parse_end_days_3bh()`
  (`len == 2`, `days` LE, `days == 0` → `has_days = false`),
  `fn_parse_memory_3dh()` (`len == 8 || len == 9`; `five_year_documents`,
  `thirty_day_kb` (30 дней, **не** 30 лет), `marking_percent` только при
  `len >= 9` и `has_marking = true`; чтение `data[8]` при `len == 8` запрещено)
- [X] T020 Добавить агрегаты: `FnInfo` (`status`, `ffd`, `fw`, `session`,
  `has_ffd`, `has_fw`, `has_session`, `response_has_warning`) с
  `fn_info_alloc/free/reset` в `lib/fn/proto/fn_info.h` и
  `lib/fn/proto/fn_info.c`; `FnLifeInfo` (`date`, `days`, `memory`, `has_date`,
  `has_days`, `has_memory`) с `fn_life_info_alloc/free/reset` в
  `lib/fn/proto/fn_life.h` и `lib/fn/proto/fn_life.c`; `serial[0] = '\0'` при
  инициализации
- [X] T021 Написать `tests/host/test_parsers.c`: эталонные ответы каждого парсера,
  `3Dh` 8 байт → `has_marking == false` (0 % не подставляется), 9 байт → 7 %,
  `3Bh` `68 01` → 360 дней, `3Bh` `00 00` → `has_days == false`, короткие
  буферы (`len` на 1 меньше требуемого) → `false` без изменения `*out`,
  `data == NULL` → `false`, `30h` с невалидной датой → оба `has_last_doc*`
  `false`
- [X] T022 [P] Создать `unit_tests/lib_tests/fn/proto/fn_status_test.c` (`30h`,
  `10h`, предупреждение `0x80`) с регистрацией в `unit_tests/test_index.c`
- [X] T023 [P] Создать `unit_tests/lib_tests/fn/proto/fn_ident_test.c` (`3Ah`,
  `33h`, `is_mgm` для `mode_raw` 0 и 1) с регистрацией в
  `unit_tests/test_index.c`
- [X] T024 [P] Создать `unit_tests/lib_tests/fn/proto/fn_life_test.c` (`32h`,
  `3Bh`, `3Dh` 8/9 байт) с регистрацией в `unit_tests/test_index.c`

### R3. Каталог кодов ответа

- [X] T025 Создать `lib/fn/core/fn_error.h` и `lib/fn/core/fn_error.c`: полный
  каталог `FNError` по `contracts/fn-api.md` §3 (все коды `response-codes.md`),
  `fn_error_is_known`, `fn_error_title` (`FNOk` → «Успешно»; для неизвестного —
  «Неизвестный код ответа ФН», никогда не `NULL`), `fn_error_hint` (`""`, если
  подсказки нет, никогда не `NULL`); тексты — статические строки без `malloc`,
  заголовок ≤ ~40 символов
- [X] T026 Написать `tests/host/test_error.c`: перебор всех значений `FNError` из
  заголовка — каждое известно и имеет непустой title; неизвестный код
  (`0x06`, `0x3F`) → `is_known == false` и title «Неизвестный код ответа ФН»;
  `fn_error_hint` для всех кодов не `NULL`
- [X] T027 [P] Создать `unit_tests/lib_tests/fn/core/fn_error_test.c` (полнота
  таблицы + неизвестный код на устройстве) с регистрацией в
  `unit_tests/test_index.c`

### R4. Транспорт

- [X] T028 Создать `lib/fn/transport/fn_uart.h` и `lib/fn/transport/fn_uart_i.h`:
  API `alloc/free/trx/cancel`, `FN_RX_READY_WAIT_MS 100`, состояние `busy` как
  `volatile bool`, один `FuriStreamBuffer` ёмкостью `FN_FRAME_MAX_LEN`, один
  one-shot `FuriTimer`, `FuriThreadPriorityNormal`; `fn_uart_alloc()` возвращает
  `NULL` при занятом порте (без `furi_check`-краша)
- [X] T029 Переписать `lib/fn/transport/fn_uart.c`: захват USART
  (`FuriHalSerialIdUsart` в release, `FuriHalSerialIdLpuart` под `FURI_DEBUG`),
  парные `expansion_disable/enable` и `furi_record_open(RECORD_EXPANSION)`,
  DMA-RX колбэк без блокировки (`furi_stream_buffer_send(..., 0)`, при
  переполнении — флаг ошибки приёма), `fn_uart_trx()` без `furi_delay_ms`-цикла
  (ожидание `rx_ready` через флаги с таймаутом 100 мс), `fn_uart_cancel()`
  (таймер стоп, буфер очищен, флаги сняты, состояние `Idle`)
- [X] T030 [P] Создать `unit_tests/lib_tests/fn/transport/fn_uart_test.c`: захват/
  освобождение порта парны; `cancel` возвращает `Idle` (следующий `trx`
  работает); обмен с реальным ФН печатает `[SKIP]` при отсутствии ответа;
  регистрация набора в `unit_tests/test_index.c`

### R5. Воркер и операции

- [X] T031 Создать `lib/fn/fn_worker.h` и `lib/fn/fn_worker_i.h`: `FnOpKind`
  (`FnOpDetect/ReadInfo/ReadLife/FlashMgm`), `FnOpStatus` (`Ok/NoResponse/
  FnRefused/BadFrame/Cancelled/Busy/InternalError`), `FnOpResult`
  (`id`, `kind`, `status`, `error`, `has_warning`), `FnTask` (`info`, `life` —
  владелец вызывающий), `FnWorkerCallback`, константы `FN_CMD_TIMEOUT_MS 300`,
  `FN_OP_DEADLINE_READ_MS 10000`, `FN_OP_DEADLINE_FLASH_MS 30000`,
  `FN_DETECT_ATTEMPTS 3`
- [X] T032 Реализовать `lib/fn/fn_worker.c`: один поток, одна активная операция,
  монотонный `op_id`, дедлайн от `furi_get_tick()` в `fn_worker_start`,
  callback из потока воркера ровно один раз (включая `Cancelled`), проверка
  остатка дедлайна перед каждой командой (< `FN_CMD_TIMEOUT_MS` → `NoResponse`),
  `fn_worker_cancel()` без блокировки через `furi_thread_flags_set` +
  `fn_uart_cancel()`, игнор отмены для `FlashMgm`, после завершения — транспорт
  `Idle`, буферы пусты; логи `TAG` + `FURI_LOG_*`
- [X] T033 Реализовать `lib/fn/fn_ops.c`: сценарии `Detect` (`30h` → `3Ah` →
  `33h`), `ReadInfo` (`Detect` + `10h` только для этапа 2), `ReadLife`
  (`32h` → `3Dh` → `3Bh`, при любом этапе), `FlashMgm` (`60h`+`16h` — ровно одна
  отправка без повторов); отказ необязательной команды (`10h`, `3Bh`, `3Dh`) не
  отменяет операцию: `has_* = false`, статус `Ok`, код в `FURI_LOG_W`; `0x80` →
  `Ok` + `has_warning`
- [X] T034 Создать `lib/fn/fn.h` — umbrella-заголовок библиотеки (типы, парсеры,
  ошибки, воркер); потребитель — только `ui/` и `fn_test_app.c`
- [X] T035 [P] Создать `unit_tests/lib_tests/fn/worker/fn_worker_test.c`: `Detect`
  с ФН (иначе `[SKIP]`), `Detect` без ФН → `NoResponse`, `ReadInfo` дважды
  (растущий `id`, данные не «залипают»), `ReadInfo` → `cancel` через 100 мс →
  `Cancelled` ≤ 1 с и следующая операция `Ok`, 10 циклов «операция → отмена» +
  11-я успешная, `FlashMgm` + `cancel` → отмена игнорируется; в этом же
  изменении удалить старый `unit_tests/lib_tests/fn/fn_worker_test.c` и его
  строку регистрации в `unit_tests/test_index.c`; зарегистрировать новый набор
  в `unit_tests/test_index.c`

### R5b. Вывод из эксплуатации старых UI и модулей

Порядок выполнения: R1–R5 → «Базовый UI» (T037–T047) → R5b (T036 одним
коммитом с T077). Задача записана здесь как этап R5 плана, но затрагивает
файлы старого UI, поэтому выполняется последней в Фазе 2.

- [X] T036 Выполняется ПОСЛЕ T037–T047 (базовый UI) и ОДНИМ изменением вместе
  с T077, иначе сборка между задачами красная. Удалить старый UI и старые
  модули библиотеки: `scenes/fn_test_scene*.c`, `scenes/fn_test_scene.h`,
  `scenes/fn_test_scene_config.h`, `views/fn_test_view_detect.{c,h}`,
  `views/fn_test_view_progress.{c,h}`, `lib/fn/fn_tools.c/.h`,
  `lib/fn/fn_worker_modes.c`, `lib/fn/fn_helpers.c`, `lib/fn/fn_helpers.h`,
  `lib/fn/fn_commands.h` (дубликат нового `lib/fn/proto/fn_commands.h` — иначе
  неоднозначность include и провал `check_layers.sh`), каталоги
  `lib/fn/fn_objects/`, `lib/fn/errors/`, `lib/fn/data_types/`, `lib/fn/uart/`;
  удалить оставшиеся файлы из `unit_tests/lib_tests/fn/` и их регистрацию в
  `unit_tests/test_index.c`; проверить, что `lib/fn/fn.h` (T034) включает
  только новые заголовки, а `grep -rn "FN_UART_MIN_PACKAGE_LEN\|two_uint8t_to"`
  по репозиторию пуст. Критерий: `ufbt` и `FN_TESTS=1 ufbt` собираются,
  host-тесты зелёные

### Базовый UI (кириллица, меню, ожидание, текст)

- [X] T037 Создать `ui/fn_ui.h`: `FN_UI_MARGIN_X 2`, `FN_UI_HEADER_H 12`,
  `FN_UI_CONTENT_Y 13`, `FN_UI_CONTENT_H 50`, `FN_UI_LINE_H 9`,
  `FN_UI_SMALL_H 7`, `FN_UI_SECTION_GAP 3`, `FN_UI_SCROLLBAR_W 3`,
  `FN_UI_TEXT_W 120`, `FN_TEXT_MAX_SECTIONS 8`, `FN_TEXT_MAX_LINES`,
  `FN_TEXT_LINE_LEN 40`
- [X] T038 Создать `ui/fn_font.h` и `ui/fn_font.c` — сгенерированный u8g2-массив с
  кириллицей: ASCII `0x20–0x7E`, А–я, Ё/ё, `№`, `—`, `…`, `«»`; два размера
  (основной 9 px, мелкий 7 px), бюджет ≤ 6 КБ; точка входа
  `fn_font_primary()`/`fn_font_small()`; исходники в UTF-8, шрифт с
  Unicode-таблицей (иначе коды символов не совпадут с глифами)
- [X] T039 [P] Создать `ui/fn_strings.h` — единый каталог всех пользовательских
  строк (меню, поля, значения, ожидание, результат) по `contracts/ui.md` §9:
  `FN_STR_MENU_*`, `FN_STR_SN`, `FN_STR_STAGE`, `FN_STR_SESSION`, `FN_STR_DOC`,
  `FN_STR_FFD`, `FN_STR_FW`, `FN_STR_MGM`, `FN_STR_YES/NO/NO_DATA`,
  `FN_STR_DOC_NONE/DOC_DATA_RECEIVED/DOC_NO_DATA`, `FN_STR_WAIT_*`,
  `FN_STR_RESULT_OK`, `FN_STR_NO_RESPONSE`, `FN_STR_BAD_FRAME`,
  `FN_STR_NOT_MGM`, `FN_STR_UNKNOWN_CODE_FMT`
- [X] T040 [P] Создать `ui/views/fn_view_menu.h` и `ui/views/fn_view_menu.c`:
  `fn_view_menu_alloc/free/get_view/set_callback/set_selected/get_selected`,
  собственная отрисовка на `Canvas` с `fn_font`, 5 пунктов в порядке FR-001
  (Информация об ФН, Информация о фазе жизни ФН, Очистка МГМ, Распиновка,
  О приложении), выбранный индекс сохраняется; callback только отправляет
  custom event (без `scene_manager_*`)
- [X] T041 [P] Создать `ui/views/fn_view_wait.h` и `ui/views/fn_view_wait.c`:
  `fn_view_wait_set(view, mode, title, subtitle, cancelable)` для
  `FnWaitModeDetect/Read/Flash`; анимация `A_FNLooking_64x64` (существующий ассет
  `icons/FNLooking_64x64/`) — `view_tie_icon_animation` в `alloc`,
  `icon_animation_start` в enter-callback, `icon_animation_stop` в exit-callback,
  `icon_animation_free` в `free`; повторный `set` не перезапускает анимацию;
  ни одного `furi_delay_ms` в draw/enter/exit
- [X] T037a [P] Создать `ui/fn_text_model.h` — только данные, без GUI:
  `FnTextLine { char text[FN_TEXT_LINE_LEN]; FnTextStyle emphasis; }`,
  `FnTextSection { const char* title; FnTextLine lines[FN_TEXT_MAX_LINES];
  uint8_t line_count; }`, `FnTextModel { FnTextSection sections[FN_TEXT_MAX_SECTIONS];
  uint8_t section_count; }`; `FnTextStyle` (`Normal/Header/Warning`) и
  константы `FN_TEXT_*` — из `ui/fn_ui.h`; заголовок не включает `gui/`,
  `fn_view_text.h` и `furi` — это условие host-тестируемости (T054/T059) и
  `check_layers.sh` (T004)
- [X] T042 [P] Создать `ui/views/fn_view_text.h` и `ui/views/fn_view_text.c`:
  `clear/add_section/add_line(style, fmt, …)/commit` — вспомогательный API для
  экранов результата/справки; `fn_view_text_set_model(view, const FnTextModel*)`
  — рендер готовой модели (перенос по словам для `add_line` выполняется в
  `commit`/draw); секции `{title, lines[FN_TEXT_MAX_LINES], line_count}`, строки
  `{text[FN_TEXT_LINE_LEN = 40], emphasis}`; слово шире `FN_UI_TEXT_W` —
  посимвольно, без обрезки; заголовок не остаётся внизу без строки;
  прокрутка шагом `FN_UI_LINE_H`, `elements_scrollbar` при
  `content_height > FN_UI_CONTENT_H`; `commit`/`set_model` сбрасывают
  `scroll_y = 0`; `malloc` только в `alloc`; `ui/fn_format.c` методы view НЕ
  вызывает (только заполняет `FnTextModel`)
- [X] T043 Написать `tests/host/test_strings.c`: все строки из
  `ui/fn_strings.h` непустые; в пользовательских подписях нет латиницы, кроме
  разрешённого списка (SN, ФФД, URL, «Код: 0xNN»); формат
  `FN_STR_UNKNOWN_CODE_FMT` даёт «Неизвестный код ответа ФН: 0xNN»
- [X] T044 Переписать `fn_test_app_i.h` и `fn_test_app.h`: `FNApp` содержит
  `Gui*`, `NotificationApp*`, `ViewDispatcher*`, `SceneManager*`, `FNWorker*`,
  указатели на 4 view, `FnInfo*`, `FnLifeInfo*`, `FnTask`, `FnOpResult op_result`,
  `uint32_t op_id`; `enum FNAppView` — ровно `FNAppViewMenu`, `FNAppViewWait`,
  `FNAppViewText`, `FNAppViewConfirm`; удалить мёртвые `FNAppMode` и лишние
  значения enum
- [X] T045 Переписать `fn_test_app.c`: `fn_test_app_alloc` открывает только
  `RECORD_GUI` и `RECORD_NOTIFICATION` (без `RECORD_DIALOGS`), создаёт
  `view_dispatcher` (`ViewDispatcherTypeFullscreen`), `SceneManager`, `FNWorker`,
  view и `fn_info_alloc`/`fn_life_info_alloc`; `fn_test_app_free` освобождает в
  обратном порядке, вызывает `fn_worker_free` только при `!busy`; ошибка
  инициализации view/воркера — русское сообщение и выход без краша; никаких
  действий в GUI-потоке с блокировкой
- [X] T046 Создать `ui/scenes/fn_scene.h`, `ui/scenes/fn_scene.c` и
  `ui/scenes/fn_scene_config.h`: декларативная регистрация сцен через `ADD_SCENE`
  (`Start`, `Wait`, `Info`, `Life`, `Confirm`, `Result`, `Wiring`, `About`),
  порядок и обработчики `on_enter/on_exit/on_event`
- [X] T047 Создать `ui/scenes/fn_scene_start.c`: главное меню через
  `FnMenuView`, 5 пунктов, сохранение выбранного индекса, Back — выход из
  приложения; выбор пунктов 0–4 отправляет custom event, обработка переходов —
  в задачах соответствующих историй

**Checkpoint**: `ufbt` собирается, host-тесты (`make -C tests/host`) зелёные,
`FN_TESTS=1` прогон `Status: PASSED`, меню на русском отображается, но пункты
ещё не выполняют операций. Истории можно начинать параллельно.

---

## Phase 3: User Story 1 — Просмотр информации о ФН (Priority: P1) 🎯 MVP

**Goal**: Инженер выбирает «Информация об ФН», видит экран ожидания с анимацией,
затем — русские реквизиты ФН (SN, этап, смена, документ для ОФД, ФФД, ПО,
МГМ/боевой, последний ФД, предупреждения) с прокруткой; Back сразу в меню.

**Independent Test**: Подключить исправный ФН, выбрать пункт меню, дождаться
экрана данных, проверить значения по эталону и возврат в меню; отдельно — ФН в
фазе 1/4 (нет блока «Последний ФД») и ФН с флагами предупреждений.

### Реализация

- [X] T048 [US1] Создать `ui/fn_format.h` и `ui/fn_format.c`: заполнение
  `FnTextModel` (T037a) секциями «Информация об ФН» в фиксированном порядке — «ФН (30h)»
  (SN, этап, состояние смены, текущий документ для ОФД + «данные получены /
  не получены», для этапа 2 — номер смены и чека, для этапов 2–3 — номер и дата
  последнего ФД), «Предупреждения» (только при `warn_flags != 0`: известные
  флаги построчно через `fn_warn_flag_text`, затем «Прочие предупреждения:
  0xNN» из `fn_warn_flags_unknown`), «ФФД (3Ah)», «ПО ФН (33h)» (включая
  «МГМ (тестовый) / боевой»); отсутствующие данные — «нет данных», кроме блока
  предупреждений; для этапов 1 и 4 блок «Последний ФД» не выводится вовсе;
  неизвестный документ — «Неизвестный документ (0xNN)»
- [X] T049 [US1] Создать `ui/scenes/fn_scene_wait.c`: режимы `Detect` и `Read`,
  запуск операции через `fn_worker_start` в `on_enter`, обработка единственного
  события `FNCustomEventWorkerResult` с проверкой `result.id == app->op_id`,
  переходы `Ok` → `Info`, иначе → `Result`; текст ожидания «Поиск ФН…»
  (подзаголовок «Проверьте подключение») и «Ожидание ответа ФН…»
  (подзаголовок «Назад — вернуться в меню»)
- [X] T050 [US1] Создать `ui/scenes/fn_scene_info.c`: сборка модели через
  `fn_format`, `fn_view_text_set_model(...)`,
  `view_dispatcher_switch_to_view(FNAppViewText)`, прокрутка
  Up/Down и Left/Right, Back → `Start` независимо от позиции прокрутки
- [X] T051 [US1] Подключить пункт меню «Информация об ФН» в
  `ui/scenes/fn_scene_start.c`: переход в `Wait` с операцией `FnOpReadInfo`,
  сброс `FnTask`/`FnInfo` (`fn_info_reset`) перед запуском
- [X] T052 [US1] Обработать путь предупреждений: ответ с `0x80` даёт
  `FnOpStatusOk` + `has_warning`, данные показываются, блок предупреждений
  добавляется без сообщения об ошибке (FR-022, SC-011) — правки в
  `ui/scenes/fn_scene_wait.c` и `ui/fn_format.c`
- [X] T053 [US1] Обработать частичные данные: `has_ffd`/`has_fw` = false →
  «нет данных» в соответствующей секции; `has_session` = false на этапе 2 →
  «нет данных» для номера смены и чека при сохранении остальных секций
  (FR-027, SC-012) — `ui/fn_format.c`
- [X] T054 [P] [US1] Написать `tests/host/test_format_info.c`: эталонный
  `FnInfo` → ожидаемая последовательность секций и строк; проверки «нет данных»,
  «Документа нет», «Неизвестный документ (0xNN)», отсутствия блока «Последний
  ФД» на этапах 1/4, строки «Прочие предупреждения: 0x80» при неизвестном бите
- [X] T055 [US1] Приёмка US1 на реальном ФН по `quickstart.md` §6 (пункты 1–4):
  анимация → данные, прокрутка, Back → меню; результат записать в
  `specs/001-fn-tester-ux/baseline.md` (раздел «Приёмка»)

**Checkpoint**: US1 полностью функциональна и проверена независимо — MVP готов.

---

## Phase 4: User Story 2 — Информация о фазе жизни ФН (Priority: P2)

**Goal**: Экран «Информация о фазе жизни ФН»: срок действия, выполненные и
оставшиеся регистрации, дни до окончания, ресурс памяти (5 лет / 30 дней),
процент области уведомлений о маркировке — независимо от версии ФФД и этапа.

**Independent Test**: Проверить на ФН с известными параметрами жизненного цикла
и на ФН, не поддерживающем расширенные показатели (поля «нет данных», экран не
пуст).

### Реализация

- [ ] T056 [US2] Добавить в `ui/fn_format.c` заполнение `FnTextModel` секциями
  «Фаза жизни»:
  «Срок действия (32h)» (дата или «не активизирован» при `year == 0`,
  выполнено/осталось регистраций), «Остаток срока (3Bh)» («N дней» или
  «нет данных» при `has_days == false`), «Ресурс памяти (3Dh)» («ресурс 5-летнего
  хранения (число документов)», «ресурс 30-дневного хранения (КБ)»,
  «Область уведомлений о маркировке: N %» только при `has_marking`); блоки 2 и 3
  показываются всегда, независимо от ФФД и этапа (FR-011, FR-028)
- [ ] T057 [US2] Создать `ui/scenes/fn_scene_life.c`: сборка модели через
  `fn_format`, `fn_view_text_set_model(...)`, `FNAppViewText`, прокрутка,
  Back → `Start`
- [ ] T058 [US2] Подключить пункт меню «Информация о фазе жизни ФН» в
  `ui/scenes/fn_scene_start.c` и операцию `FnOpReadLife` в
  `ui/scenes/fn_scene_wait.c`: `on_enter` сбрасывает `FnLifeInfo`
  (`fn_life_info_reset`), `Ok` → `Life`
- [ ] T059 [P] [US2] Дополнить `tests/host/test_format_info.c` (или создать
  `tests/host/test_format_life.c`) кейсами: `has_days == false` → «нет данных»,
  `has_marking == false` → строки о маркировке нет (0 % не подставляется),
  отказ `3Dh` не скрывает секции 32h/3Bh
- [ ] T060 [US2] Приёмка US2 по `quickstart.md` §6 (пункт 5): значения сверены с
  эталоном, процент маркировки присутствует только при девятом байте

**Checkpoint**: US1 и US2 работают независимо.

---

## Phase 5: User Story 3 — Очистка МГМ (Priority: P3)

**Goal**: «Очистка МГМ»: проверка «МГМ/боевой», предупреждение о необратимости и
подтверждение, одна отправка `60h`+`16h`, неотменяемое ожидание до 30 с, русский
результат.

**Independent Test**: На тестовом МГМ выполнить очистку с подтверждением и без
него; на боевом ФН убедиться, что команда не отправлена.

### Реализация

- [ ] T061 [P] [US3] Создать `ui/views/fn_view_confirm.h` и
  `ui/views/fn_view_confirm.c`: `fn_view_confirm_set(view, title, body)` —
  `title[32]`, `body[128]`, выбранная кнопка `selected` (0 — «Отмена» по
  умолчанию, 1 — «Очистить»), `fn_view_confirm_set_callback(cb, ctx)`;
  отрисовка на `fn_font`, callback отправляет custom event
- [ ] T062 [US3] Создать `ui/scenes/fn_scene_confirm.c`: показ предупреждения
  «Операция необратима…», «Отмена»/Back → `Start` без отправки команды,
  подтверждение → `Wait` с `FnOpFlashMgm`
- [ ] T063 [US3] Реализовать режим `Flash` в `ui/scenes/fn_scene_wait.c`: текст
  «Очистка МГМ…», подзаголовок «Может занять до 30 секунд, не прерывайте»,
  `cancelable = false`; Back игнорируется, ожидание идёт до ответа или таймаута
  30 с; команда не дублируется; результат → `Result`
- [ ] T064 [US3] Реализовать preflight «МГМ/боевой»: проверить `is_mgm` после
  `FnOpDetect` перед показом подтверждения (в `ui/scenes/fn_scene_start.c` /
  `ui/scenes/fn_scene_wait.c`), при `is_mgm == false` — переход в `Result` с
  текстом «Очистка недоступна: ФН не является МГМ», команда не отправляется
  (FR-024, SC-009); `FlashMgm` в `lib/fn/fn_ops.c` дополнительно не собирает
  кадр при ложном признаке
- [ ] T065 [US3] Создать `ui/scenes/fn_scene_result.c` для исходов очистки:
  успех («Очистка МГМ выполнена успешно»), отказ ФН (title + hint из
  `fn_error_title/hint` + «Код: 0xNN»), нет ответа («ФН не отвечает» +
  «Проверьте подключение и повторите»), Back → `Start`
- [ ] T066 [P] [US3] Дополнить `unit_tests/lib_tests/fn/worker/fn_worker_test.c`:
  `FlashMgm` + `fn_worker_cancel()` → отмена игнорируется, результат приходит по
  факту или через 30 с; тест «боевой ФН → команда не отправлена» (проверка по
  логу счётчика отправок в `lib/fn/fn_ops.c` под `FN_TESTS`)
- [ ] T067 [US3] Приёмка US3 по `quickstart.md` §6 (пункты 9–11) на реальном МГМ
  и на боевом ФН; зафиксировать 0 отправок при отказе (SC-004) и 100 % показ
  результата при нажатом Back (SC-008)

**Checkpoint**: US3 работает независимо; изменяющая команда защищена
подтверждением и preflight-проверкой.

---

## Phase 6: User Story 4 — Прерывание ожидания и понятные ошибки связи (Priority: P4)

**Goal**: Back во время чтения возвращает в меню ≤ 1 с; отсутствие ответа,
битый кадр и неизвестный код описываются по-русски; следующая операция
выполняется без перезапуска приложения.

**Independent Test**: Запустить чтение без ФН, нажать Back во время ожидания,
затем повторить операцию с ФН — успешно, без перезапуска; 10 циклов
«операция → отмена».

### Реализация

- [ ] T068 [US4] Реализовать отмену чтения в `ui/scenes/fn_scene_wait.c`: Back при
  `cancelable == true` вызывает `fn_worker_cancel()` и сразу переходит в
  `Start` (≤ 1 с, SC-003); `on_exit` останавливает анимацию; событие
  отменённой операции отбрасывается по `op_id`
- [ ] T069 [US4] Расширить `ui/scenes/fn_scene_result.c` для ошибок чтения:
  `NoResponse` → «ФН не отвечает» + «Проверьте подключение и повторите»;
  `BadFrame` → «Повреждённый ответ ФН»; `FnRefused` → русский текст из
  `fn_error_title/hint`; неизвестный код → «Неизвестный код ответа ФН: 0xNN»
  (FR-029, SC-015); Back → `Start`
- [ ] T070 [P] [US4] Дополнить `unit_tests/lib_tests/fn/worker/fn_worker_test.c`:
  10 циклов «чтение → отмена» подряд, затем 11-я операция `Ok` (SC-005);
  битый CRC → `BadFrame`, транспорт `Idle`, следующая операция `Ok`;
  проверка отсутствия роста `Leaked` между прогонами
- [ ] T071 [US4] Гарантировать защиту от «поздних» результатов: в
  `fn_test_app.c` инкремент `app->op_id` при каждом старте, запись
  `app->op_result` до `view_dispatcher_send_custom_event`, сцена логирует
  `FURI_LOG_W` и игнорирует чужой `id` (файлы `fn_test_app.c`,
  `ui/scenes/fn_scene_wait.c`)
- [ ] T072 [US4] Приёмка US4 по `quickstart.md` §6 (пункты 6–8 и 13): время до
  меню ≤ 1 с, сообщение об отсутствии ответа ≤ 10 с, 10 циклов отмены без
  деградации

**Checkpoint**: Все сценарии отказа и отмены предсказуемы; US1–US4 работают
независимо.

---

## Phase 7: User Story 5 — Распиновка и сведения о приложении (Priority: P5)

**Goal**: Справочные экраны «Распиновка» (иконка) и «О приложении» (название,
версия, автор, URL, описание) работают без подключённого ФН.

**Independent Test**: Открыть оба пункта без ФН, проверить читаемость и возврат
в меню по Back.

### Реализация

- [ ] T073 [P] [US5] Создать `ui/scenes/fn_scene_wiring.c`: полноэкранная иконка
  `I_Wiring_UART_128x64` (ассет `icons/Wiring_UART_128x64.png`) через
  `fn_view_text_set_image()`, без обращения к воркеру/UART, Back → `Start`
- [ ] T074 [P] [US5] Создать `ui/scenes/fn_scene_about.c`: название, версия
  `fap_version`, автор, URL проекта (`fap_weburl`), краткое русское описание
  возможностей; Back → `Start`
- [ ] T075 [US5] Подключить пункты меню «Распиновка» и «О приложении» в
  `ui/scenes/fn_scene_start.c` (переходы без запуска операций)
- [ ] T076 [US5] Приёмка US5 по `quickstart.md` §6 (пункт 12): оба экрана
  открываются без ФН, текст читается на 128×64, Back → меню

**Checkpoint**: Все пять историй функциональны независимо.

---

## Phase 8: Polish & Cross-Cutting (R7–R8)

**Purpose**: Уборка мёртвого кода, манифест/версия, документация, гейты по
памяти и полная приёмка.

- [ ] T077 Проверить полноту вывода из эксплуатации старого UI/библиотеки,
  удалённых в T036 (одно изменение с этой задачей): `grep -rn` не находит
  `fn_test_scene`, `fn_test_view_`, `fn_helpers`, `fn_tools`, `fn_worker_modes`,
  `fn_objects`, `FN_UART_MIN_PACKAGE_LEN`; `ufbt` собирает только `ui/` и новый
  `lib/fn`; в release-сборке нет пунктов det/test/timeout
- [ ] T078 [P] Убрать остатки мёртвого кода: `bool_to_hum()` и неиспользуемые
  поля/enum в `fn_test_app_i.h`, `view_dispatcher_set_tick_event_callback`,
  неиспользуемые ассеты (`icons/DolphinNice_96x59.png`, если не используется)
- [ ] T079 Обновить `application.fam`: `fap_version=(0, 6)`, `requires=["gui",
  "expansion"]` без `dialogs`, сохранить профиль `FN_TESTS` и
  `cdefines=["APP_FN_TEST"]`; проверить, что при добавлении новых
  `furi_record_open` список `requires` соответствует коду
- [ ] T080 [P] Создать/дополнить `docs/protocol.md`: карта «команда → файл
  разбора», реализованные команды (`30h`, `32h`, `33h`, `3Ah`, `3Bh`, `3Dh`,
  `10h`, `60h`), список нереализованных (`31h`, `35h`, `3Fh`, `36h`–`39h`,
  `40h`–`47h`, `50h`, `B*h`), отклонения от Протокола (`is_mgm` как эвристика)
- [ ] T081 [P] Дополнить `docs/byte-order.md` до финальной версии: инструкция
  «при добавлении команды использовать только `fn_wire_*`/`fn_frame_*`, тест на
  golden-кадр обязателен», ссылка на `tests/host/golden_frames.h`
- [ ] T082 [P] Создать `docs/ui.md`: блоки экранов, вёрстка 128×64, прокрутка,
  команда генерации шрифта (BDF → `bdfconv`), бюджет шрифта ≤ 6 КБ, требования
  к анимации `A_FNLooking_64x64`
- [ ] T083 [P] Обновить `.catalog/CHANGELOG.md` (версия 0.6, русский UI, полный
  рефакторинг `lib/fn`, исправление `*_LE/*_BE`, новые тесты), `README.md`
  (русский интерфейс, скриншоты, структура) и `AGENTS.md` (новая структура
  `core/proto/transport/ui`, предупреждение о ловушке байтового порядка)
- [ ] T084 Выполнить `ufbt format` и `ufbt lint`, устранить все warning-и; при
  необходимости зафиксировать изменения форматирования в `.clang-format`
- [ ] T085 Пройти гейт памяти: `free` до запуска и после выхода (не ниже
  baseline − 1 КБ), `top` для стека UART/воркера, прирост RAM от шрифта и
  моделей ≤ 12 КБ (шрифт ≤ 6 КБ); записать результат в
  `specs/001-fn-tester-ux/baseline.md`
- [ ] T086 Полный прогон тестов: `make -C tests/host`, `make -C tests/host asan`,
  `make -C tests/host check-layers`, `FN_TESTS=1 ufbt launch` +
  `python3 .test_scripts/units.py <NAME>` — ожидается `Failed tests: 0`,
  `Status: PASSED`, отсутствие роста `Leaked`; допустимы только `[SKIP]`
- [ ] T087 Закрыть приёмку по `quickstart.md` §5–§8: все пункты таблицы §6
  (1–13), проверка golden-значений (`0x0168`, CRC `0xCDFF`, 360 дней, 7 %),
  отсутствие отладочных пунктов в release-сборке (`FN_TESTS` не задан)

---

## Dependencies & Execution Order

### Phase Dependencies

- **Фаза 1 (Setup)**: без зависимостей, стартует сразу
- **Фаза 2 (Foundational)**: зависит от Фазы 1 — **блокирует все истории**
- **Фазы 3–7 (US1–US5)**: зависят только от Фазы 2 (внутри фазы — порядок задач)
- **Фаза 8 (Polish)**: зависит от завершения всех выбранных историй

### Внутренние зависимости Фазы 2

1. **R1** (`fn_types` → `fn_wire`/`fn_frame` → тесты → атомарная миграция
   `lib/fn/fn_helpers.c`, `lib/fn/fn_tools.c`, `lib/fn/uart/fn_uart.c`) —
   миграция (T014) выполняется одним изменением: слой + CRC + все вызовы +
   тесты
2. **R2** (`proto/*`) требует `core/*` (T007–T009)
3. **R3** (`fn_error`) требует `core/*`, не зависит от R2
4. **R4** (`transport`) требует `core/fn_frame` (T009)
5. **R5** (`worker` + `fn_ops`) требует R2, R3, R4
6. **Удаление старых модулей** (T036) — только после T031–T034 **и** T037–T047,
   одним коммитом с T077; до этого старые сцены остаются рабочими
7. **Базовый UI** (T037–T047) требует `lib/fn/fn.h` (T034) и `core/fn_types.h`
   (T007/T007a)

### Зависимости историй

- **US1 (P1)**: после Фазы 2 — независима (T048 → T049 → T050/T051 → T052/T053 →
  T054/T055)
- **US2 (P2)**: после Фазы 2 — независима; использует те же `ui/fn_format.c`
  (T056 после T048) и `ui/scenes/fn_scene_wait.c` (T058 после T049)
- **US3 (P3)**: после Фазы 2 — независима (T061 → T062 → T063 → T064 → T065 →
  T066/T067)
- **US4 (P4)**: после Фазы 2 — независима по тестам, но расширяет
  `ui/scenes/fn_scene_wait.c` и `ui/scenes/fn_scene_result.c`, поэтому
  выполняется после US1 (T068 после T049/T050) и после US3 в части
  `fn_scene_result.c` (T069 после T065)
- **US5 (P5)**: после Фазы 2 — полностью независима (T073/T074 параллельно,
  T075 после T047, T076)

### Внутри каждой истории

- Тесты пишутся вместе с реализацией модуля; для golden-набора (T010) — тесты
  первыми, до миграции T014
- Модель/парсер → сценарий операции → сцена → приёмка
- История считается завершённой только после собственного checkpoint-теста

---

## Parallel Opportunities

### Фаза 1

```text
T002 tests/host/shim/furi.h + test_wire_legacy.c
T003 tests/host/golden_frames.h
T004 tests/host/check_layers.sh
```

### Фаза 2

```text
# R1
T007a core/fn_types.c            |  T008 core/fn_wire.{h,c}  |  T009 core/fn_frame.{h,c}
T011 fn_wire_test.c              |  T012 fn_frame_test.c  |  T013 fn_types_test.c

# R2 / R3
T016 fn_commands.h | T017 fn_status.{h,c} | T018 fn_ident.{h,c} | T019 fn_life.{h,c}
T022 fn_status_test.c | T023 fn_ident_test.c | T024 fn_life_test.c | T025 core/fn_error.{h,c}

# Базовый UI
T037a ui/fn_text_model.h | T039 ui/fn_strings.h | T040 fn_view_menu.{h,c} | T041 fn_view_wait.{h,c} | T042 fn_view_text.{h,c}

# После UI (не параллельно с T037–T047)
T036 (+T077): удаление старого UI и старых модулей одним коммитом
```

### Истории US1–US5 (после Фазы 2, если позволяет состав исполнителей)

```text
US1: T048 → T049 → T050/T051 → T052/T053 → T054/T055
US2: T056 → T057 → T058 → T059/T060
US3: T061 → T062 → T063 → T064 → T065 → T066/T067
US4: T068 → T069 → T070/T071 → T072
US5: T073 ‖ T074 → T075 → T076
```

Правило: один и тот же файл (`ui/fn_format.c`, `ui/scenes/fn_scene_wait.c`,
`ui/scenes/fn_scene_start.c`, `ui/scenes/fn_scene_result.c`) нельзя править
параллельно — задачи по нему выполняются последовательно.

---

## Implementation Strategy

### MVP First (только US1)

1. Фаза 1 (T001–T006) — baseline и host-каркас
2. Фаза 2 (T007–T047) — ядро, транспорт, воркер, базовый UI (**критично**)
3. Фаза 3 (T048–T055) — US1
4. **STOP и ПРОВЕРКА**: приёмка US1 на реальном ФН по `quickstart.md` §6 (1–4)
5. Демонстрация MVP: русское меню + экран ожидания с анимацией + информация об ФН

### Incremental Delivery

1. Фаза 1 + Фаза 2 → фундамент готов, `ufbt` и тесты зелёные
2. US1 → приёмка → релиз-кандидат MVP
3. US2 → приёмка → диагностика жизненного цикла
4. US3 → приёмка на МГМ и боевом ФН → безопасная очистка
5. US4 → приёмка отмены/ошибок → полевая пригодность
6. US5 → справочные экраны
7. Фаза 8 → версия 0.6, документация, гейт памяти, полная приёмка

### Параллельная работа

1. Setup + Foundational — одной командой (ядро, затем транспорт/воркер/UI)
2. После Фазы 2: разработчик A — US1→US4, разработчик B — US2, разработчик
   C — US3→US4 (`fn_scene_result.c`), разработчик D — US5
3. Любая история завершается и проверяется независимо, до перехода к следующей

---

## Notes

- `[P]` — разные файлы и нет зависимости от незавершённых задач
- `[Story]` — трассировка задачи к истории из `spec.md`
- Каждая история независимо завершаема и тестируема; приёмка — на реальном ФН
  (US1, US2), МГМ и боевом ФН (US3)
- Тесты обязательны (конституция IV): новая/изменённая функция библиотеки —
  тест в `unit_tests/` + host-тест для `core/`/`proto/`
- Запрещено: `furi_delay_ms` в draw/input/сценах, литералы пользовательских
  строк в сценах, коды команд и байты протокола в `ui/`, `malloc` по длине из
  ответа ФН
- Коммит после каждой задачи или логической группы; откат R1 — только целиком
  (см. `quickstart.md` §9)
