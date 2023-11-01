//
// Created by Игорь Данилов on 17.09.2023.
//
#include "unit_tests/minunit.h"
#include "helpers/byte_array.h"


MU_TEST(byte_array_test) {

    ByteArray* byte_array = byte_array_alloc(5);
    mu_assert_not_null(byte_array);
    uint8_t test_array[5] = {1,2,3,4,5};
    byte_array_push(byte_array, test_array, 5);
    uint8_t result = 0;
    byte_array_get_int_BE(byte_array, &result ,sizeof(uint8_t));
    mu_assert_int_eq(1, result);
}

MU_TEST_SUITE(byte_array_test_suite) {
    MU_RUN_TEST(byte_array_test);
    //UNUSED(byte_array_test);
}

int run_minunit_test_byte_array() {
    MU_RUN_SUITE(byte_array_test_suite);
    return MU_EXIT_CODE;
}
