//
// Created by Игорь Данилов on 24.08.2023.
//
#include "unit_tests/minunit.h"
#include "unit_tests/base_test_vars.h"
#include "fn_objects/life_info/fn_life_info_i.h"

static FNLifeInfo* fnLifeInfo;

static void test_setup(void) {
    fnLifeInfo = malloc(sizeof(FNLifeInfo));
}

static void test_teardown(void) {
    free(fnLifeInfo);
}

MU_TEST(fn_life_info_create_test) {
    mu_assert_not_null(fnLifeInfo);
}

MU_TEST(fn_life_info_date_end_test) {
    mu_assert_not_null(fnLifeInfo);
    fnLifeInfo->date_end.day = 21;
    fnLifeInfo->date_end.mouth = 8;
    fnLifeInfo->date_end.year = 23;
    FuriString* dateStr = furi_string_alloc();
    fn_life_info_get_end_date(fnLifeInfo, dateStr);
    mu_assert_string_eq("21.08.2023", furi_string_get_cstr(dateStr));
    //TODO Добавить тест для не валидной даты и времени последнего ФД + написать проверку в функции;
}

MU_TEST(fn_life_info_days_to_end_test) {
    mu_assert_not_null(fnLifeInfo);
    for(size_t i = 0; i < COUNT_OF(UINT_16_TEST_VAL); i++) {
        fnLifeInfo->days_to_end = UINT_16_TEST_VAL[i];//Тест максимального и минимального значения
        mu_assert_int_eq(UINT_16_TEST_VAL[i], fn_life_info_get_days_to_end(fnLifeInfo));
    }
}

MU_TEST(fn_life_info_reg_report_ctn_remaining_test) {
    mu_assert_not_null(fnLifeInfo);
    for(size_t i = 0; i < COUNT_OF(UINT_8_TEST_VAL); i++) {
        fnLifeInfo->reg_report_ctn_remaining = UINT_8_TEST_VAL[i];
        mu_assert_int_eq(UINT_8_TEST_VAL[i], fn_life_info_get_reg_report_ctn_remaining(fnLifeInfo));
    }
}

MU_TEST(fn_life_info_reg_report_ctn_test) {
    mu_assert_not_null(fnLifeInfo);
    for(size_t i = 0; i < COUNT_OF(UINT_8_TEST_VAL); i++) {
        fnLifeInfo->reg_report_ctn = UINT_8_TEST_VAL[i];
        mu_assert_int_eq(UINT_8_TEST_VAL[i], fn_life_info_get_reg_report_ctn(fnLifeInfo));
    }
}

MU_TEST(fn_life_info_five_year_data_resource_test) {
    mu_assert_not_null(fnLifeInfo);
    for(size_t i = 0; i < COUNT_OF(UINT_32_TEST_VAL); i++) {
        fnLifeInfo->five_year_data_resource = UINT_32_TEST_VAL[i];
        mu_assert_int_eq(UINT_32_TEST_VAL[i], fn_life_info_get_five_year_data_resource(fnLifeInfo));
    }
}

MU_TEST(fn_life_info_thirty_year_data_resource_test) {
    mu_assert_not_null(fnLifeInfo);
    for(size_t i = 0; i < COUNT_OF(UINT_32_TEST_VAL); i++) {
        fnLifeInfo->thirty_year_data_resource = UINT_32_TEST_VAL[i];
        mu_assert_int_eq(UINT_32_TEST_VAL[i], fn_life_info_get_thirty_year_data_resource(fnLifeInfo));
    }
}

MU_TEST(fn_life_info_marking_notifications_resource_test) {
    mu_assert_not_null(fnLifeInfo);
    for(size_t i = 0; i < COUNT_OF(UINT_32_TEST_VAL); i++) {
        fnLifeInfo->marking_notifications_resource = UINT_32_TEST_VAL[i];
        mu_assert_int_eq(
                UINT_32_TEST_VAL[i], fn_life_info_get_marking_notifications_resource(fnLifeInfo));
    }
}

MU_TEST_SUITE(fn_life_info_suite) {
    MU_SUITE_CONFIGURE(&test_setup, &test_teardown);

    MU_RUN_TEST(fn_life_info_create_test);
    MU_RUN_TEST(fn_life_info_date_end_test);
    MU_RUN_TEST(fn_life_info_days_to_end_test);
    MU_RUN_TEST(fn_life_info_reg_report_ctn_remaining_test);
    MU_RUN_TEST(fn_life_info_reg_report_ctn_test);
    MU_RUN_TEST(fn_life_info_five_year_data_resource_test);
    MU_RUN_TEST(fn_life_info_thirty_year_data_resource_test);
    MU_RUN_TEST(fn_life_info_marking_notifications_resource_test);
}

int run_minunit_test_fn_life_info() {
    MU_RUN_SUITE(fn_life_info_suite);
    return MU_EXIT_CODE;
}
