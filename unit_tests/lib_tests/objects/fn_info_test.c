//
// Created by Игорь Данилов on 22.08.2023.
//
#include "../../minunit.h"
#include "fn_objects/fn_info/fn_i.h"

MU_TEST(fn_info_fill_test){
    FNInfo *fnInfo = malloc(sizeof(FNInfo));
    mu_assert_not_null(fnInfo);
    fnInfo->last_doc_number = 9;
    mu_assert_int_eq(1, 1);
}

MU_TEST(fn_info_fail_test){
    mu_assert_int_eq(1, 0);
}

MU_TEST(fn_info_ok_test){
    mu_assert_int_eq(1, 1);
}

MU_TEST_SUITE(fn_info_suite) {
    MU_RUN_TEST(fn_info_fill_test);
    MU_RUN_TEST(fn_info_fail_test);
    MU_RUN_TEST(fn_info_ok_test);
}

int run_minunit_test_fn_info_fill()
{
    MU_RUN_SUITE(fn_info_suite);
    return MU_EXIT_CODE;
}
