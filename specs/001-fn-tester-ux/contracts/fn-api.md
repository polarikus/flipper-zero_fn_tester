# Контракт: публичный API библиотеки `lib/fn`

**Слой**: `lib/fn/fn.h` (umbrella) + `core/`, `proto/`, `transport/`, `fn_worker`.
**Потребитель**: только `ui/` и `fn_test_app.c`; обратная зависимость запрещена.

## 1. Границы и зависимости

| Модуль | Зависит от | Не зависит от |
|---|---|---|
| `core/*` | `<stdint.h>`, `<stdbool.h>`, `<stddef.h>`, `<string.h>` | `furi.h`, HAL, UI |
| `proto/*` | `core/*`, `<string.h>` | `furi.h`, UART, UI |
| `transport/*` | `furi`, `furi_hal_serial*`, `expansion` | UI, сцены |
| `fn_worker` | `transport`, `proto`, `core`, `furi` | UI, сцены |
| `ui/*` | `fn` (публичный API), `gui` | `transport`, `core` (кроме типов) |

Правило: если файл из `core/` или `proto/` включает `furi.h`, host-сборка
падает — это осознанный «стоп-кран» (см. `module-boundaries.md`).

## 2. Базовые типы (`core/fn_types.h`)

```c
typedef struct { uint8_t year; uint8_t month; uint8_t day; } Date;
typedef struct { Date date; uint8_t hour; uint8_t minute; } DateTime;

typedef enum {
    FnStageUnknown = 0,
    FnStageReadyRegistration = 0x01,
    FnStageRegistered       = 0x03,
    FnStageArchiveClosed    = 0x07,
    FnStageArchiveClosedFinal = 0x0F,
} FnStage;

typedef enum {
    FnFfdNone = 0, FnFfd1_05 = 1, FnFfd1_1 = 3, FnFfd1_2 = 4, FnFfdUnknown = 0xFF,
} FnFfd;

typedef enum {
    FnDocNone = 0x00, FnDocRegistration = 0x01, FnDocSessionOpen = 0x02,
    FnDocReceipt = 0x04, FnDocSessionClose = 0x08, FnDocFnClose = 0x10,
    FnDocStrictReport = 0x11, FnDocRegChangeFnReplace = 0x12,
    FnDocRegChange = 0x13, FnDocReceiptCorrection = 0x14,
    FnDocStrictReportCorrection = 0x15, FnDocCurrentSettlements = 0x17,
    FnDocUnknown = 0xFF,
} FnDocCode;

enum {
    FnWarnReplaceSoon    = 0x01, FnWarnResourceSoon = 0x02,
    FnWarnMemoryFull     = 0x04, FnWarnOfdTimeout   = 0x08,
    FnWarnOfdFormatLogic = 0x10, FnWarnNeedSetup    = 0x20,
    FnWarnOfdCancelled   = 0x40, FnWarnReserved     = 0x80,
};

bool fn_date_is_valid(const Date* date);
bool fn_datetime_is_valid(const DateTime* date_time);

const char* fn_stage_text(FnStage stage);      // русский текст
const char* fn_ffd_text(FnFfd ffd);            // "1.05" | "1.1" | "1.2" | "Не активизирован"
const char* fn_doc_text(FnDocCode doc);        // русский текст
const char* fn_warn_flag_text(uint8_t mask);   // текст для одного описанного бита или NULL
uint8_t     fn_warn_flags_unknown(uint8_t flags); // flags & 0x80 — неописанные биты
```

`fn_warn_flag_text()` принимает ровно одну маску (`0x01`…`0x40`) и возвращает
`NULL` для неизвестной/составной; UI сам перебирает биты и формирует строку
«Прочие предупреждения: 0xNN» из `fn_warn_flags_unknown()`.
Реализация этих функций — `core/fn_types.c` (T007a): тексты статические, без
`malloc`; `fn_doc_text(FnDocUnknown)` → «Неизвестный документ (0xNN)»
(FR-026), неизвестный этап → «нет данных».

## 3. Каталог кодов ответа (`core/fn_error.h`)

```c
typedef enum {
    FNOk                            = 0x00,
    FNErrorUnknownCMD               = 0x01,
    FNErrorWrongState               = 0x02,
    FNErrorFailure                  = 0x03,
    FNErrorFailureKS                = 0x04,
    FNErrorCMDParamsNotMatchFNEndDate = 0x05,
    FNErrorWrongDateTime            = 0x07,
    FNErrorNoRequestedData          = 0x08,
    FNErrorWrongParamValue          = 0x09,
    FNErrorWrongCommandFfd11        = 0x0A,
    FNErrorForbiddenAttribute       = 0x0B,
    FNErrorDuplicateData            = 0x0C,
    FNErrorNoDataForAccounting      = 0x0D,
    FNErrorTooManyPositions         = 0x0E,
    FNErrorTlvTooBig                = 0x10,
    FNErrorNoTransport              = 0x11,
    FNErrorResourceExhausted        = 0x12,
    FNErrorResourceLimited          = 0x14,
    FNErrorSessionTooLong           = 0x16,
    FNErrorWrongTimeInterval        = 0x17,
    FNErrorWrongAttribute           = 0x18,
    FNErrorAttributeNotRegistered   = 0x19,
    FNErrorWrongAnswer              = 0x20,
    FNErrorKmUpdateNegative         = 0x23,
    FNErrorKmUpdateUnknown          = 0x24,
    FNErrorKmUpdateRepeat           = 0x30,
    FNErrorMarkingForbidden         = 0x32,
    FNErrorMarkingSequence          = 0x33,
    FNErrorMarkingBlocked           = 0x34,
    FNErrorKmTableOverflow          = 0x35,
    FNErrorKmKeysExpired            = 0x36,
    FNErrorTlvMissingAttribute      = 0x3C,
    FNErrorKmNotChecked             = 0x3E,
} FNError;

bool        fn_error_is_known(FNError code);
const char* fn_error_title(FNError code);   // короткий русский текст; никогда NULL
const char* fn_error_hint(FNError code);    // "" если подсказки нет; никогда NULL
```

Правила:
- `fn_error_title(FNOk)` → «Успешно»; в UI блок успеха не использует этот текст.
- Если код не входит в перечисление, `fn_error_is_known()` → `false`,
  `fn_error_title()` → `"Неизвестный код ответа ФН"`, а UI обязан показать
  `«Неизвестный код ответа ФН: 0xNN»` (FR-029).
- Тексты — статические строки (без `malloc`), пригодные для 128 px
  (заголовок ≤ 40 символов; длинная подсказка переносится `FnTextView`).

## 4. Парсеры (`proto/`)

```c
// 30h
typedef struct {
    FnStage   stage;
    FnDocCode current_doc;
    bool      doc_data_received;
    bool      session_open;
    uint8_t   warn_flags;
    DateTime  last_doc_dt;
    char      serial[17];
    uint32_t  last_doc_number;
    bool      has_last_doc;
    bool      has_last_doc_number;
    bool      has_serial;
} FnStatus;
bool fn_parse_status_30h(const uint8_t* data, size_t len, FnStatus* out);

// 10h
typedef struct {
    bool     session_open;
    uint16_t shift_number;
    uint16_t receipt_number;
} FnSession;
bool fn_parse_session_10h(const uint8_t* data, size_t len, FnSession* out);

// 3Ah
typedef struct { FnFfd current; FnFfd max; } FnFfdInfo;
bool fn_parse_ffd_3ah(const uint8_t* data, size_t len, FnFfdInfo* out);

// 33h
typedef struct {
    char    version[17];
    uint8_t mode_raw;   // 0 — опытный образец, 1 — серийное изделие
    bool    is_mgm;     // эвристика проекта: mode_raw == 0
} FnFwInfo;
bool fn_parse_fw_33h(const uint8_t* data, size_t len, FnFwInfo* out);

// 32h
typedef struct {
    Date    end_date;
    uint8_t reg_remaining;
    uint8_t reg_done;
} FnLifeDate;
bool fn_parse_end_date_32h(const uint8_t* data, size_t len, FnLifeDate* out);

// 3Bh
typedef struct { uint16_t days; bool has_days; } FnLifeDays;
bool fn_parse_end_days_3bh(const uint8_t* data, size_t len, FnLifeDays* out);

// 3Dh
typedef struct {
    uint32_t five_year_documents;
    uint32_t thirty_day_kb;
    uint8_t  marking_percent;
    bool     has_marking;
} FnMemory;
bool fn_parse_memory_3dh(const uint8_t* data, size_t len, FnMemory* out);
```

Гарантии парсеров:
- `data == NULL` или недостаточная длина → `false`, `*out` не изменяется.
- Чтения за границей буфера нет ни при какой длине (проверяется тестом и
  ASan-прогоном host-тестов).
- `3Dh` принимает `len == 8` и `len == 9`; `has_marking` только при `len >= 9`.
- `3Bh` с `days == 0` → `has_days == false`.
- `30h` при невалидной дате → `has_last_doc == false` и
  `has_last_doc_number == false`.

## 5. Агрегаты (`proto/fn_info.h`, `proto/fn_life.h`)

```c
typedef struct {
    FnStatus  status;
    FnFfdInfo ffd;
    FnFwInfo  fw;
    FnSession session;
    bool      has_ffd;
    bool      has_fw;
    bool      has_session;
    bool      response_has_warning;
} FnInfo;

FnInfo* fn_info_alloc(void);          // zeroed, serial[0] = '\0'
void    fn_info_free(FnInfo* info);
void    fn_info_reset(FnInfo* info);

typedef struct {
    FnLifeDate date;
    FnLifeDays days;
    FnMemory   memory;
    bool       has_date;
    bool       has_days;
    bool       has_memory;
} FnLifeInfo;

FnLifeInfo* fn_life_info_alloc(void);
void        fn_life_info_free(FnLifeInfo* life);
void        fn_life_info_reset(FnLifeInfo* life);
```

## 6. Воркер (`fn_worker.h`)

```c
#define FN_CMD_TIMEOUT_MS          300u
#define FN_OP_DEADLINE_READ_MS     10000u
#define FN_OP_DEADLINE_FLASH_MS    30000u
#define FN_RX_READY_WAIT_MS        100u

typedef enum { FnOpDetect, FnOpReadInfo, FnOpReadLife, FnOpFlashMgm } FnOpKind;

typedef enum {
    FnOpStatusOk, FnOpStatusNoResponse, FnOpStatusFnRefused,
    FnOpStatusBadFrame, FnOpStatusCancelled, FnOpStatusBusy,
    FnOpStatusInternalError,
} FnOpStatus;

typedef struct {
    uint32_t   id;
    FnOpKind   kind;
    FnOpStatus status;
    FNError    error;
    bool       has_warning;
} FnOpResult;

typedef struct {           // владелец — вызывающий; воркер только заполняет
    FnInfo*     info;      // обязателен для Detect/ReadInfo
    FnLifeInfo* life;      // обязателен для ReadLife
} FnTask;

typedef void (*FnWorkerCallback)(void* ctx, const FnOpResult* result);

FNWorker* fn_worker_alloc(void);
void      fn_worker_free(FNWorker* worker);   // требует: операция не активна

bool      fn_worker_start(FNWorker* worker, FnOpKind kind, FnTask* task,
                          FnWorkerCallback callback, void* ctx);
void      fn_worker_cancel(FNWorker* worker); // не блокирует; игнор для FlashMgm
bool      fn_worker_is_busy(const FNWorker* worker);
```

Семантика:
- `fn_worker_start` не блокирует вызывающий поток: если воркер ещё занят
  (например, завершается отменённая операция), возвращается `false`, callback
  не вызывается; сцена показывает «Дождитесь завершения операции».
  Результат доставляется только через callback.
- Callback вызывается из потока воркера ровно один раз, включая отмену.
- `fn_worker_cancel` не блокирует вызывающий поток; операция завершится
  статусом `Cancelled` и освободит транспорт в пределах 1 с.
- `FlashMgm` нельзя отменить и нельзя повторить: по истечении 30 с —
  `NoResponse`, команда не дублируется.
- `fn_worker_free` вызывает `furi_check(!busy)`; приложение вызывает его только
  из главного меню, где операций нет.
- Операция `Detect`: `30h` → `3Ah` → `33h`. `ReadInfo`: `Detect` + `10h`
  (только этап 2). `ReadLife`: `32h` → `3Dh` → `3Bh` (`3Bh` — на любом этапе;
  при отказе ФН `has_days = false`, операция остаётся `Ok`).
  Отказ необязательной команды не отменяет операцию: выставляется
  `has_* = false`, статус остаётся `Ok` (FR-011, FR-027).
- Коды `03h/04h/12h` и иные отказы ФН → `FnOpStatusFnRefused` + `result.error`;
  успешный ответ с `0x80` → `FnOpStatusOk` + `has_warning = true`.

## 7. Правила использования из UI

1. Сцена не вызывает `fn_uart_*`, `fn_frame_*`, `fn_parse_*` и не содержит
   байтов команд.
2. Сцена запускает операцию в `on_enter` (или по custom event) и реагирует на
   единственное событие `FNCustomEventWorkerResult`.
3. `FnTask` живёт в `FNApp` и переиспользуется; перед запуском — `*_reset`.
4. Back во время отменяемой операции: `fn_worker_cancel()` и сразу переход в
   меню; результат отменённой операции игнорируется по `id`.
5. Никаких `furi_delay_ms` в draw/input/сценах.

## 8. Версионирование API

Публичного внешнего API у библиотеки нет (внутренняя `Lib`), поэтому смена
сигнатур допустима. Версия приложения (`fap_version`) повышается до `(0, 6)`,
изменения фиксируются в `.catalog/CHANGELOG.md`.
