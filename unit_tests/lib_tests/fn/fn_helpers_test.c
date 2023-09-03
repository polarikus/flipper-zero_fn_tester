//
// Created by Игорь Данилов on 25.08.2023.
//
#include "unit_tests/minunit.h"
#include "fn_helpers.h"
#include "unit_tests/base_test_vars.h"

char* bull_to_hum_test_values[] = {"yes", "no"};
uint8_t test_fn_answer[] = {0x04, 0x13, 0x00, 0x00, 0x66, 0x6E, 0x20, 0x31,
                            0x2E, 0x32, 0x20, 0x6D, 0x67, 0x6D, 0x20, 0x30,
                            0x33, 0x20, 0x20, 0x00, 0x00, 0x07, 0x9E, 0x59};

MU_TEST(bool_to_hum_test) {
    for(size_t i = 0; i < COUNT_OF(BOOL_TEST_VAL); ++i) {
        mu_assert_string_eq(bull_to_hum_test_values[i], bool_to_hum(BOOL_TEST_VAL[i]));
    }
}

MU_TEST(two_uint8t_to_uint16t_LE_test) {
    uint8_t test_data[2] = {0x9E, 0x59};
    uint16_t expected = 0x9E59;
    mu_assert_int_eq(expected, two_uint8t_to_uint16t_LE(test_data));
}

MU_TEST(two_uint8t_to_uint16t_BE_test) {
    uint8_t test_data[2] = {0x9E, 0x59};
    uint16_t expected = 0x599E;
    mu_assert_int_eq(expected, two_uint8t_to_uint16t_BE(test_data));
}

MU_TEST(uint16t_LE_to_uint8t_bytes_test) {
    uint16_t test_data = 0x9E59;
    uint8_t expected[2] = {0x9E, 0x59};
    uint8_t result[2];
    uint16t_LE_to_uint8t_bytes(test_data, result);
    for(size_t i = 0; i < COUNT_OF(expected); ++i) {
        mu_assert_int_eq(expected[i], result[i]);
    }
}

MU_TEST(uint16t_BE_to_uint8t_bytes_test) {
    uint16_t test_data = 0x599E;
    uint8_t expected[2] = {0x9E, 0x59};
    uint8_t result[2];
    uint16t_BE_to_uint8t_bytes(test_data, result);
    for(size_t i = 0; i < COUNT_OF(expected); ++i) {
        mu_assert_int_eq(expected[i], result[i]);
    }
}

//TODO какие-то плохие названия у функций
MU_TEST(byte_array_to_uint32t_LE_test) {
    uint8_t test_data[4] = {0xac, 0x9a, 0x13, 0x01};
    uint32_t expected = 18061996;
    uint32_t result = byte_array_to_uint32t_LE(test_data);
    mu_assert_int_eq(expected, result);
}

MU_TEST(calc_crc16_test) {
    uint16_t result = calc_crc16(test_fn_answer, 1, COUNT_OF(test_fn_answer) - 2);
    uint16_t expected;
    uint8_t crcCmd[2] = {
        test_fn_answer[COUNT_OF(test_fn_answer) - 2],
        test_fn_answer[COUNT_OF(test_fn_answer) - 1]};
    expected = two_uint8t_to_uint16t_LE(crcCmd);
    mu_assert_int_eq(expected, result);
}

MU_TEST(check_crc_test) {
    uint16_t expected_crc;
    uint8_t crcCmd[2] = {
        test_fn_answer[COUNT_OF(test_fn_answer) - 2],
        test_fn_answer[COUNT_OF(test_fn_answer) - 1]};
    expected_crc = two_uint8t_to_uint16t_LE(crcCmd);
    mu_assert(
        check_crc(expected_crc, test_fn_answer, 1, COUNT_OF(test_fn_answer) - 2),
        "Wrong check_crc result!!!");
}

MU_TEST(add_bytes_to_arr_test) {
    uint8_t target_array[4] = {0xac, 0x9a, 0x00, 0x00};
    uint8_t dest_array[2] = {0x13, 0x01};
    uint32_t expected = 18061996;
    add_bytes_to_arr(target_array, 2, COUNT_OF(target_array), dest_array, COUNT_OF(dest_array));
    uint32_t result = byte_array_to_uint32t_LE(target_array);
    mu_assert_int_eq(expected, result);
}

MU_TEST_SUITE(fn_helpers_test_suite) {
    MU_RUN_TEST(bool_to_hum_test);
    MU_RUN_TEST(two_uint8t_to_uint16t_LE_test);
    MU_RUN_TEST(two_uint8t_to_uint16t_BE_test);
    MU_RUN_TEST(uint16t_LE_to_uint8t_bytes_test);
    MU_RUN_TEST(uint16t_BE_to_uint8t_bytes_test);
    MU_RUN_TEST(byte_array_to_uint32t_LE_test);
    MU_RUN_TEST(calc_crc16_test);
    MU_RUN_TEST(check_crc_test);
    MU_RUN_TEST(add_bytes_to_arr_test);
}

int run_minunit_test_fn_helpers() {
    MU_RUN_SUITE(fn_helpers_test_suite);
    return MU_EXIT_CODE;
}
