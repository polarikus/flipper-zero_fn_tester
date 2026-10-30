// Тест-«ловушка» этапа R0.
//
// Фиксирует ФАКТИЧЕСКОЕ поведение старых helper'ов из lib/fn/fn_helpers.c:
// имена функций противоположны реальному порядку байтов, а calc_crc16()
// переставляет байты в возвращаемом значении. Тест не «исправляет» код —
// он документирует ловушку, чтобы замена на fn_wire_*/fn_frame_* (R1) была
// доказательно эквивалентной на проводе.
//
// Удаляется вместе с helper'ами в задаче T014.

#include <stdint.h>

#include "fn_helpers.h"

#include "test_common.h"

// Независимый эталон: CRC16-CCITT, init 0xFFFF, полином 0x1021, MSB-first,
// без какой-либо перестановки байтов в результате.
static uint16_t crc16_ccitt_reference(const uint8_t* data, size_t len) {
    uint16_t crc = 0xFFFF;
    for(size_t i = 0; i < len; ++i) {
        crc ^= (uint16_t)data[i] << 8;
        for(int bit = 0; bit < 8; ++bit) {
            crc = (crc & 0x8000) ? (uint16_t)((crc << 1) ^ 0x1021) : (uint16_t)(crc << 1);
        }
    }
    return crc;
}

int main(void) {
    // 1. «LE» на самом деле читает big-endian.
    const uint8_t bytes[2] = {0x01, 0x02};
    TEST_CHECK_EQ_U16(two_uint8t_to_uint16t_LE(bytes), 0x0102);
    // 2. «BE» на самом деле читает little-endian.
    TEST_CHECK_EQ_U16(two_uint8t_to_uint16t_BE(bytes), 0x0201);

    // 3. «LE_to_bytes» пишет big-endian, «BE_to_bytes» — little-endian.
    uint8_t out_le[2] = {0, 0};
    uint8_t out_be[2] = {0, 0};
    uint16t_LE_to_uint8t_bytes(0x0102, out_le);
    uint16t_BE_to_uint8t_bytes(0x0102, out_be);
    TEST_CHECK(out_le[0] == 0x01 && out_le[1] == 0x02);
    TEST_CHECK(out_be[0] == 0x02 && out_be[1] == 0x01);

    // 4. Кадр запроса 30h: 04 01 00 30 FF CD.
    //    CRC считается по LEN..DATA, то есть по {01 00 30}.
    const uint8_t crc_area[] = {0x01, 0x00, 0x30};
    const uint16_t raw_crc = crc16_ccitt_reference(crc_area, sizeof(crc_area));
    TEST_CHECK_EQ_U16(raw_crc, 0xCDFF);
    // Старый calc_crc16(data, start, end) берёт end как «последний индекс + 1»
    // (внутри end = end - 1) и возвращает значение с переставленными байтами.
    TEST_CHECK_EQ_U16(calc_crc16(crc_area, 0, 3), 0xFFCD);
    // Именно поэтому check_crc() работает: оба значения переставлены одинаково.
    TEST_CHECK(check_crc(calc_crc16(crc_area, 0, 3), crc_area, 0, 3));

    // 5. Байты CRC в кадре читаются тем же «LE»-helper'ом, который на деле BE.
    const uint8_t crc_on_wire[2] = {0xFF, 0xCD};
    TEST_CHECK_EQ_U16(two_uint8t_to_uint16t_LE(crc_on_wire), 0xFFCD);

    return TEST_SUMMARY();
}
