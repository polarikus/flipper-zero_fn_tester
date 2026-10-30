// Golden-кадры протокола ФН v1.2.05 — единый источник истины для host- и
// device-тестов кодека (этап R0, задача T003).
//
// Источник: .opencode/skills/fn-protocol/examples/frames.md (все кадры
// проверены расчётом). CRC16-CCITT: полином 0x1021, init 0xFFFF, MSB-first;
// считается по LEN..DATA, на проводе младший байт первым.
//
// Правило: при добавлении команды сначала добавь сюда проверенный кадр,
// затем тест на сборку/разбор.

#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct {
    const char* name;
    const uint8_t* data;
    size_t len;
} GoldenFrame;

// --- Запросы ККТ -----------------------------------------------------------

static const uint8_t golden_req_30h[] = {0x04, 0x01, 0x00, 0x30, 0xFF, 0xCD}; // статус ФН
static const uint8_t golden_req_31h[] = {0x04, 0x01, 0x00, 0x31, 0xDE, 0xDD}; // номер ФН
static const uint8_t golden_req_32h[] = {0x04, 0x01, 0x00, 0x32, 0xBD, 0xED}; // срок действия
static const uint8_t golden_req_33h[] = {0x04, 0x01, 0x00, 0x33, 0x9C, 0xFD}; // версия ПО
static const uint8_t golden_req_3ah[] = {0x04, 0x01, 0x00, 0x3A, 0xB5, 0x6C}; // формат ФФД
// 3Bh: остаток дней на 29.10.2026 12:30 (DATE_TIME в данных запроса).
static const uint8_t golden_req_3bh[] =
    {0x04, 0x06, 0x00, 0x3B, 0x1A, 0x0A, 0x1D, 0x0C, 0x1E, 0x24, 0xC0};
static const uint8_t golden_req_3dh[] = {0x04, 0x01, 0x00, 0x3D, 0x52, 0x1C}; // ресурс памяти
// 60h — сброс МГМ, расширение проекта (в Протоколе v1.2.05 отсутствует).
static const uint8_t golden_req_60h_16h[] = {0x04, 0x02, 0x00, 0x60, 0x16, 0x75, 0x10};

static const GoldenFrame golden_requests[] = {
    {"30h статус", golden_req_30h, sizeof(golden_req_30h)},
    {"31h номер ФН", golden_req_31h, sizeof(golden_req_31h)},
    {"32h срок действия", golden_req_32h, sizeof(golden_req_32h)},
    {"33h версия ПО", golden_req_33h, sizeof(golden_req_33h)},
    {"3Ah формат ФФД", golden_req_3ah, sizeof(golden_req_3ah)},
    {"3Bh остаток дней", golden_req_3bh, sizeof(golden_req_3bh)},
    {"3Dh ресурс памяти", golden_req_3dh, sizeof(golden_req_3dh)},
    {"60h сброс МГМ (16h)", golden_req_60h_16h, sizeof(golden_req_60h_16h)},
};

// --- Ответы ФН -------------------------------------------------------------

// 30h, успех: этап 2, документ не открыт, данные не получены, смена открыта,
// флагов нет, дата 29.10.2026 12:30, SN 9999078900001234, ФД 2Ah.
static const uint8_t golden_resp_30h[] = {
    0x04, 0x1F, 0x00, 0x00, 0x03, 0x00, 0x00, 0x01, 0x00, 0x1A, 0x0A, 0x1D,
    0x0C, 0x1E, 0x39, 0x39, 0x39, 0x39, 0x30, 0x37, 0x38, 0x39, 0x30, 0x30,
    0x30, 0x30, 0x31, 0x32, 0x33, 0x34, 0x2A, 0x00, 0x00, 0x00, 0x8E, 0xC7,
};

// 30h с предупреждением: answer = 0x80 (код 0x00), флаги 0x02.
static const uint8_t golden_resp_30h_warn[] = {
    0x04, 0x1F, 0x00, 0x80, 0x03, 0x00, 0x00, 0x01, 0x02, 0x1A, 0x0A, 0x1D,
    0x0C, 0x1E, 0x39, 0x39, 0x39, 0x39, 0x30, 0x37, 0x38, 0x39, 0x30, 0x30,
    0x30, 0x30, 0x31, 0x32, 0x33, 0x34, 0x2A, 0x00, 0x00, 0x00, 0xCE, 0xA3,
};

// 32h: срок 31.12.2027, осталось 12 регистраций, выполнено 3.
static const uint8_t golden_resp_32h[] =
    {0x04, 0x06, 0x00, 0x00, 0x1B, 0x0C, 0x1F, 0x0C, 0x03, 0x3E, 0x0C};

// 3Ah: ФФД 1.2, максимум 1.2.
static const uint8_t golden_resp_3ah[] = {0x04, 0x03, 0x00, 0x00, 0x04, 0x04, 0x9E, 0x73};

// 3Bh: осталось 360 дней (0x0168).
static const uint8_t golden_resp_3bh_360[] = {0x04, 0x03, 0x00, 0x00, 0x68, 0x01, 0x7C, 0x6D};

// 3Bh: осталось 200 дней (0x00C8).
static const uint8_t golden_resp_3bh_200[] = {0x04, 0x03, 0x00, 0x00, 0xC8, 0x00, 0x23, 0x60};

// 3Dh без байта маркировки (8 байт данных): 5 лет — 10000, 30 дней — 300 КБ.
static const uint8_t golden_resp_3dh_8[] = {
    0x04,
    0x09,
    0x00,
    0x00,
    0x10,
    0x27,
    0x00,
    0x00,
    0x2C,
    0x01,
    0x00,
    0x00,
    0x82,
    0xAB,
};

// 3Dh с байтом маркировки (9 байт данных): те же значения + 7 %.
static const uint8_t golden_resp_3dh_9[] = {
    0x04,
    0x0A,
    0x00,
    0x00,
    0x10,
    0x27,
    0x00,
    0x00,
    0x2C,
    0x01,
    0x00,
    0x00,
    0x07,
    0xF9,
    0xF3,
};

// Отказ ФН: код 03h, данных нет.
static const uint8_t golden_resp_error_03h[] = {0x04, 0x01, 0x00, 0x03, 0xCF, 0xCB};

static const GoldenFrame golden_responses[] = {
    {"30h успех", golden_resp_30h, sizeof(golden_resp_30h)},
    {"30h с предупреждением 0x80", golden_resp_30h_warn, sizeof(golden_resp_30h_warn)},
    {"32h срок действия", golden_resp_32h, sizeof(golden_resp_32h)},
    {"3Ah формат ФФД", golden_resp_3ah, sizeof(golden_resp_3ah)},
    {"3Bh 360 дней", golden_resp_3bh_360, sizeof(golden_resp_3bh_360)},
    {"3Bh 200 дней", golden_resp_3bh_200, sizeof(golden_resp_3bh_200)},
    {"3Dh 8 байт", golden_resp_3dh_8, sizeof(golden_resp_3dh_8)},
    {"3Dh 9 байт", golden_resp_3dh_9, sizeof(golden_resp_3dh_9)},
    {"отказ 03h", golden_resp_error_03h, sizeof(golden_resp_error_03h)},
};
