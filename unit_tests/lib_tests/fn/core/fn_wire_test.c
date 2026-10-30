// On-device тесты байтового слоя (T011, R1): CRC16-CCITT и little-endian.
#include "unit_tests/minunit.h"

#include "core/fn_wire.h"
#include "tests/host/golden_frames.h"

MU_TEST(u16le_get_test) {
    const uint8_t bytes[2] = {0x68, 0x01};
    mu_assert_int_eq(0x0168, fn_wire_u16le_get(bytes));
    mu_assert_int_eq(0, fn_wire_u16le_get(NULL));
}

MU_TEST(u16le_roundtrip_test) {
    uint8_t bytes[2] = {0};
    fn_wire_u16le_put(bytes, 0x9E59);
    mu_assert_int_eq(0x59, bytes[0]);
    mu_assert_int_eq(0x9E, bytes[1]);
    mu_assert_int_eq(0x9E59, fn_wire_u16le_get(bytes));

    fn_wire_u16le_put(NULL, 0x1234); // no-op, не падение
}

MU_TEST(u32le_roundtrip_test) {
    uint8_t bytes[4] = {0};
    fn_wire_u32le_put(bytes, 0x0130A5C3u);
    mu_assert_int_eq(0xC3, bytes[0]);
    mu_assert_int_eq(0xA5, bytes[1]);
    mu_assert_int_eq(0x30, bytes[2]);
    mu_assert_int_eq(0x01, bytes[3]);
    mu_assert_int_eq(0x0130A5C3u, fn_wire_u32le_get(bytes));
    mu_assert_int_eq(0, fn_wire_u32le_get(NULL));
}

MU_TEST(crc16_golden_test) {
    // Кадр 30h: CRC считается по LEN..DATA = 01 00 30.
    const uint8_t request[] = {0x01, 0x00, 0x30};
    mu_assert_int_eq(0xCDFF, fn_wire_crc16_ccitt(request, sizeof(request)));

    // На проводе младший байт первым: FF CD.
    mu_assert_int_eq(golden_req_30h[4], 0xFF);
    mu_assert_int_eq(golden_req_30h[5], 0xCD);
}

MU_TEST(crc16_empty_test) {
    const uint8_t any[] = {0x01};
    mu_assert_int_eq(0, fn_wire_crc16_ccitt(NULL, 1));
    mu_assert_int_eq(0, fn_wire_crc16_ccitt(any, 0));
}

MU_TEST_SUITE(fn_wire_test_suite) {
    MU_RUN_TEST(u16le_get_test);
    MU_RUN_TEST(u16le_roundtrip_test);
    MU_RUN_TEST(u32le_roundtrip_test);
    MU_RUN_TEST(crc16_golden_test);
    MU_RUN_TEST(crc16_empty_test);
}

int run_minunit_test_fn_wire(void) {
    MU_RUN_SUITE(fn_wire_test_suite);
    return MU_EXIT_CODE;
}
