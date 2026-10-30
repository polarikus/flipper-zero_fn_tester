// Host-тесты байтового слоя: LE и CRC16-CCITT (T010, R1).
#include "fn_wire.h"
#include "golden_frames.h"
#include "test_common.h"

#include <string.h>

static void test_le(void) {
    const uint8_t le16[] = {0x68, 0x01};
    TEST_CHECK_EQ_U16(fn_wire_u16le_get(le16), 0x0168);
    TEST_CHECK_EQ_U16(fn_wire_u16le_get(NULL), 0);

    uint8_t out16[2] = {0, 0};
    fn_wire_u16le_put(out16, 0x0168);
    TEST_CHECK(out16[0] == 0x68 && out16[1] == 0x01);
    fn_wire_u16le_put(NULL, 0x1234);

    const uint8_t le32[] = {0x78, 0x56, 0x34, 0x12};
    TEST_CHECK_EQ_U32(fn_wire_u32le_get(le32), 0x12345678);
    TEST_CHECK_EQ_U32(fn_wire_u32le_get(NULL), 0);

    uint8_t out32[4] = {0, 0, 0, 0};
    fn_wire_u32le_put(out32, 0x12345678);
    TEST_CHECK(memcmp(out32, le32, sizeof(le32)) == 0);
    fn_wire_u32le_put(NULL, 0x12345678);
}

static void test_crc(void) {
    const uint8_t status_cmd[] = {0x01, 0x00, 0x30};
    TEST_CHECK_EQ_U16(fn_wire_crc16_ccitt(status_cmd, sizeof(status_cmd)), 0xCDFF);
    TEST_CHECK_EQ_U16(fn_wire_crc16_ccitt(NULL, 3), 0);
    TEST_CHECK_EQ_U16(fn_wire_crc16_ccitt(status_cmd, 0), 0);

    // CRC не зависит от START: считаем по LEN..DATA, а не по всему кадру.
    const uint8_t with_start[] = {0x04, 0x01, 0x00, 0x30};
    TEST_CHECK_EQ_U16(fn_wire_crc16_ccitt(&with_start[1], sizeof(with_start) - 1), 0xCDFF);
}

// Сборка всех golden-запросов побайтово, только средствами fn_wire.
static void test_golden_requests(void) {
    for(size_t i = 0; i < sizeof(golden_requests) / sizeof(golden_requests[0]); ++i) {
        const GoldenFrame* frame = &golden_requests[i];
        TEST_CHECK(frame->len >= 6);

        const uint16_t wire_len = fn_wire_u16le_get(&frame->data[1]);
        TEST_CHECK_EQ_U16(wire_len, frame->len - 5);

        const uint16_t crc_calc = fn_wire_crc16_ccitt(&frame->data[1], frame->len - 3);
        const uint16_t crc_wire = fn_wire_u16le_get(&frame->data[frame->len - 2]);
        if(crc_calc != crc_wire) {
            fprintf(stderr, "FAIL golden %s: crc %04X != %04X\n", frame->name, crc_calc, crc_wire);
        }
        TEST_CHECK(crc_calc == crc_wire);
    }
}

int main(void) {
    test_le();
    test_crc();
    test_golden_requests();
    return TEST_SUMMARY();
}
