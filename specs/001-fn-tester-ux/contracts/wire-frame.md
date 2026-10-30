# Контракт: кадр и байтовый порядок (провод)

**Слой**: `lib/fn/core/fn_wire.{c,h}` + `lib/fn/core/fn_frame.{c,h}`
**Источник истины**: Протокол ФН v1.2.05, разделы 3.2–3.3;
`.opencode/skills/fn-protocol/references/protocol-layers.md`.

## 1. Формат кадра

```text
+--------+--------+--------+--------+---------+--------+--------+
| 04h    | LEN lo | LEN hi | CMD /  | DATA... | CRC lo | CRC hi |
| START  |        |        | ANSWER |         |        |        |
+--------+--------+--------+--------+---------+--------+--------+
  1 байт   2 байта LE  1 байт   N байт      2 байта LE
```

| Правило | Значение |
|---|---|
| `MSG_START` | ровно `0x04` |
| `LEN` | `N + 1` (данные + байт команды/ответа), **LE** |
| Максимум `DATA` | 1536 байт по Протоколу; в приложении — 250 (`FN_FRAME_MAX_LEN 256 − 6`) |
| `CRC16` | CRC16-CCITT, полином `0x1021`, init `0xFFFF`, MSB-first, по байтам `LEN..DATA` (START **не** входит), **LE** на проводе |
| Код ответа | `ANSWER & 0x7F` |
| Предупреждение | `ANSWER & 0x80` — ответ успешен, бит не является ошибкой |
| Направление | инициирует только ККТ; один запрос — один ответ |

## 2. Канонический байтовый API

```c
uint16_t fn_wire_u16le_get(const uint8_t* p);        // p[0] | p[1] << 8
void     fn_wire_u16le_put(uint8_t* p, uint16_t v);  // p[0] = v & 0xFF, p[1] = v >> 8
uint32_t fn_wire_u32le_get(const uint8_t* p);
void     fn_wire_u32le_put(uint8_t* p, uint32_t v);
uint16_t fn_wire_crc16_ccitt(const uint8_t* data, size_t len); // ЧИСЛО, без перестановки
```

Инварианты:
- Функция `*_get` читает ровно то, что лежит на проводе; имена отражают
  порядок байтов, а не историческую путаницу.
- `fn_wire_crc16_ccitt()` не переставляет байты; запись/чтение CRC в кадре
  выполняется через `u16le_put/get`.
- `NULL` и `len == 0` — не падение: `get` возвращает 0, `put` ничего не делает;
  корректность длины проверяет вызывающий.
- Big-endian помощников нет: Протокол использует LE во всех многобайтовых
  полях. При появлении BE-поля вводится `fn_wire_u16be_get` с отдельным тестом.

### Запрещено

- Возвращать к жизни `calc_crc16()`, `check_crc()`,
  `two_uint8t_to_uint16t_LE/BE`, `uint16t_LE/BE_to_uint8t_bytes`,
  `byte_array_to_uint32t_LE`, `add_bytes_to_arr`, `prepend_data_to_array`.
- «Исправлять» имя функции без одновременной правки CRC и всех вызовов.
- Считать `LEN` или CRC в big-endian.

## 3. Кодек кадра

```c
#define FN_FRAME_MAX_LEN     256
#define FN_FRAME_OVERHEAD    6      // START + LEN(2) + CMD/ANSWER + CRC(2)
#define FN_FRAME_MAX_PAYLOAD (FN_FRAME_MAX_LEN - FN_FRAME_OVERHEAD)

typedef enum {
    FnFrameOk = 0,
    FnFrameNull,
    FnFrameTooShort,        // len < 6
    FnFrameTooLong,         // len > FN_FRAME_MAX_LEN
    FnFrameWrongStart,      // buf[0] != 0x04
    FnFrameLengthMismatch,  // LEN != len - 5
    FnFrameWrongCrc,
} FnFrameStatus;

typedef struct {
    uint8_t        command;      // сырой CMD для запроса / ANSWER для ответа
    uint8_t        code;         // command & 0x7F
    bool           has_warning;  // command & 0x80
    const uint8_t* data;         // указывает внутрь исходного буфера
    size_t         data_len;
} FnFrame;

size_t fn_frame_build(uint8_t command, const uint8_t* data, size_t data_len,
                      uint8_t* out, size_t out_cap, FnFrameStatus* status);

FnFrameStatus fn_frame_parse(const uint8_t* buf, size_t len, FnFrame* out);
```

Гарантии:
- `fn_frame_build` возвращает полный размер кадра или `0` при ошибке; при
  ошибке `*status` содержит причину, буфер `out` не считается валидным.
- `fn_frame_build` не аллоцирует память.
- `fn_frame_parse` не аллоцирует и не копирует данные; `out->data` живёт
  столько же, сколько `buf`. При ошибке `*out` не заполняется.
- Проверки порядка: `NULL` → `TooShort/Null`; границы → `TooShort/TooLong`;
  `START` → `WrongStart`; `LEN` → `LengthMismatch`; CRC — последним.

## 4. Golden-кадры (обязательные тесты)

Запросы (собираются `fn_frame_build` побайтово):

| Команда | Кадр |
|---|---|
| `30h` | `04 01 00 30 FF CD` |
| `31h` | `04 01 00 31 DE DD` |
| `32h` | `04 01 00 32 BD ED` |
| `33h` | `04 01 00 33 9C FD` |
| `3Ah` | `04 01 00 3A B5 6C` |
| `3Bh` (29.10.2026 12:30) | `04 06 00 3B 1A 0A 1D 0C 1E 24 C0` |
| `3Dh` | `04 01 00 3D 52 1C` |
| `60h` + `16h` | `04 02 00 60 16 75 10` |

Ответы (разбираются `fn_frame_parse`):

| Случай | Кадр | Ожидание |
|---|---|---|
| `30h`, успех | `04 1F 00 00 … 8E C7` (см. `frames.md`) | `code=0`, `has_warning=false`, `data_len=30` |
| `30h`, предупреждение | `04 1F 00 80 … CE A3` | `code=0`, `has_warning=true`, флаги `0x02` |
| `32h` | `04 06 00 00 1B 0C 1F 0C 03 3E 0C` | `data_len=5` |
| `3Ah` | `04 03 00 00 04 04 9E 73` | `data_len=2` |
| `3Bh` | `04 03 00 00 68 01 7C 6D` | дни 360 |
| `3Dh` (8 байт) | `04 09 00 00 10 27 00 00 2C 01 00 00 82 AB` | `data_len=8`, без маркировки |
| `3Dh` (9 байт) | `04 0A 00 00 10 27 00 00 2C 01 00 00 07 F9 F3` | `data_len=9`, маркировка 7 % |
| Отказ `03h` | `04 01 00 03 CF CB` | `code=0x03`, `data_len=0` |

Негативные тесты: пустой буфер, `len=5`, `START != 04`, `LEN` на 1 больше/меньше,
испорченный CRC, `len=257`, кадр с `LEN=250` (граница payload).

## 5. Совместимость

Поведение на проводе не меняется: все запросы и ответы выше должны совпадать
побайтово до и после рефакторинга. Контракт фиксируется host-тестами
`tests/host/test_frame.c` и повторяется в `unit_tests/lib_tests/fn/core/`.
