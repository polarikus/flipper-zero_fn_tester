//
// Created by Игорь Данилов on 22.08.2023.
//
#include "unit_tests/minunit.h"
#include "unit_tests/base_test_vars.h"
#include "fn_objects/fn_info/fn_i.h"

static char* test_fn_sn = "999907890201361100";
static FN_FFD test_ffd[] = {FFD_NONE, FFD_1_05, FFD_1_1, FFD_1_2, 100};
static char* test_ffd_string[] = {"Not registered", "1.05", "1.1", "1.2", "Unknown"};

static FNState test_fn_state[] = {FNStage1, FNStage2, FNStage3, FNStage4, 100};
static char* tes_fn_stage_string[] = {
    "Ready registration (1)",
    "Registered (2)",
    "The closed FN archive is awaiting transfer to the OFD (3)",
    "FN archive is closed (4)",
    "Unknown"};

static char* test_fw_version = "MgmTeest12345789";
static uint8_t test_warn_flags[] = {
    0b00000001,
    0b00000010,
    0b00000100,
    0b00001000,
    0b00010000,
    0b00100000,
    0b01000000,
};

static char* test_warn_flags_strs[] = {
    "Urgent replacement of FN (before the expiration date 3 days)",
    "FN resource exhaustion (before the expiration of 30 days)",
    "FN memory overflow (FN archive is 99% full)",
    "OFD response timeout exceeded",
    "Refusal according to the format-logical control data (the attribute is transmitted in the Confirmation from the OFD)",
    "KKT setting required (the attribute is transmitted in the Confirmation from the OFD)",
    "OFD canceled (sign is transmitted in Confirmation from OFD)"};

static FNInfo* fnInfo;
//TODO Test for fn_info_copy

static void test_setup(void) {
    fnInfo = malloc(sizeof(FNInfo));
}

static void test_teardown(void) {
    free(fnInfo);
}

MU_TEST(fn_info_create_test) {
    mu_assert_not_null(fnInfo);
}

MU_TEST(fn_info_fn_state_test) {
    mu_assert_not_null(fnInfo);
    for(size_t i = 0; i < COUNT_OF(test_fn_state); i++) {
        fnInfo->fn_state = test_fn_state[i];
        mu_assert_int_eq(test_fn_state[i], fn_get_fn_state_enum(fnInfo));
        mu_assert_string_eq(tes_fn_stage_string[i], fn_get_fn_state(fnInfo));
    }
}

MU_TEST(fn_info_ffd_version_test) {
    mu_assert_not_null(fnInfo);
    //Тест версии ФФД
    for(size_t i = 0; i < COUNT_OF(test_ffd); i++) {
        fnInfo->ffd_version = test_ffd[i];
        mu_assert_int_eq(test_ffd[i], fn_get_ffd_enum(fnInfo));
        mu_assert_string_eq(test_ffd_string[i], fn_get_ffd_version_string(fnInfo));
    }
}

MU_TEST(fn_info_max_ffd_version_test) {
    mu_assert_not_null(fnInfo);
    //Тест максимальной версии ФФД
    for(size_t i = 1; i < COUNT_OF(test_ffd); i++) {
        fnInfo->max_ffd_version = test_ffd[i];
        mu_assert_int_eq(test_ffd[i], fn_get_max_ffd_enum(fnInfo));
        mu_assert_string_eq(test_ffd_string[i], fn_get_max_ffd_version_string(fnInfo));
    }
}

MU_TEST(fn_info_serial_number_test) {
    mu_assert_not_null(fnInfo);
    //Тест серийного номера ФН
    strcpy(fnInfo->serial_number, test_fn_sn);
    mu_assert_string_eq(test_fn_sn, fn_get_sn(fnInfo));
}

MU_TEST(fn_info_fw_version_test) {
    mu_assert_not_null(fnInfo);
    //Тест версии прошивки ФН
    strcpy(fnInfo->fw_version.fw_version, test_fw_version);
    mu_assert_string_eq(test_fw_version, fn_get_fw_version(fnInfo));
    //Тест на флаг является ли ФН МГМ
    for(size_t i = 0; i < COUNT_OF(BOOL_TEST_VAL); i++) {
        fnInfo->fw_version.fw_mode = BOOL_TEST_VAL[i];
        mu_assert_int_eq(BOOL_TEST_VAL[i], !fn_is_mgm(fnInfo));
    }
}

MU_TEST(fn_info_is_session_open_test) {
    mu_assert_not_null(fnInfo);
    //Тест флага открытия смены
    for(size_t i = 0; i < COUNT_OF(BOOL_TEST_VAL); i++) {
        fnInfo->is_session_open = BOOL_TEST_VAL[i];
        mu_assert_int_eq(BOOL_TEST_VAL[i], fn_is_session_open(fnInfo));
    }
}

MU_TEST(fn_info_last_doc_number_test) {
    mu_assert_not_null(fnInfo);

    fnInfo->last_doc_number = UINT_32_TEST_VAL[0]; //Тест максимального значения для uint32
    mu_assert_int_eq(UINT_32_TEST_VAL[0], fn_get_last_document_number(fnInfo));

    fnInfo->last_doc_number = UINT_32_TEST_VAL[1]; //Тест минимального значения для uint32
    mu_assert_int_eq(UINT_32_TEST_VAL[1], fn_get_last_document_number(fnInfo));
}

MU_TEST(fn_info_last_doc_date_time_test) {
    mu_assert_not_null(fnInfo);
    fnInfo->date_time.date = 21;
    fnInfo->date_time.mouth = 8;
    fnInfo->date_time.year = 23;
    fnInfo->date_time.hour = 22;
    fnInfo->date_time.minute = 59;
    FuriString* date_time_str = furi_string_alloc();
    fn_get_last_document_datetime(fnInfo, date_time_str);
    mu_assert_string_eq("21.08.2023 22:59", furi_string_get_cstr(date_time_str));
    furi_string_free(date_time_str);
}

MU_TEST(fn_info_fn_warn_flags_test) {
    mu_assert_not_null(fnInfo);
    //Тест на флаги предупреждения ФН
    fnInfo->fn_warn_flags = 0;
    mu_assert(!fn_is_warn_flags_not_null(fnInfo), "fn_warn_flags not null!");
    FuriString* fn_warn_flags_str = furi_string_alloc();
    for(size_t i = 0; i < COUNT_OF(test_warn_flags); i++) {
        fnInfo->fn_warn_flags = test_warn_flags[i];
        mu_assert_int_eq(test_warn_flags[i], fnInfo->fn_warn_flags);
        mu_assert(fn_is_warn_flags_not_null(fnInfo), "fn_warn_flags not setted!");
        fn_get_warn_flags_str(fnInfo, fn_warn_flags_str);

        FuriString* expected_str = furi_string_alloc();
        furi_string_cat_printf(expected_str, "[ %s\n ]", test_warn_flags_strs[i]);
        mu_assert_string_eq(
            furi_string_get_cstr(expected_str), furi_string_get_cstr(fn_warn_flags_str));
        furi_string_free(expected_str);
    }

    for(size_t i = 0; i < COUNT_OF(test_warn_flags); i++) {
        fnInfo->fn_warn_flags = fnInfo->fn_warn_flags | test_warn_flags[i];
    }

    fn_get_warn_flags_str(fnInfo, fn_warn_flags_str);
    for(size_t i = 0; i < COUNT_OF(test_warn_flags_strs); ++i) {
        if(furi_string_search_str(fn_warn_flags_str, test_warn_flags_strs[i]) ==
           FURI_STRING_FAILURE) {
            FuriString* message = furi_string_alloc();
            furi_string_cat_printf(
                message, "String: '%s' not found in fn_warn_flags_str", test_warn_flags_strs[i]);
            mu_fail(furi_string_get_cstr(message));
        }
    }

    furi_string_free(fn_warn_flags_str);
}

MU_TEST(fn_info_copy_test) {
    //TODO Сделать тест
}

MU_TEST_SUITE(fn_info_suite) {
    MU_SUITE_CONFIGURE(&test_setup, &test_teardown);

    MU_RUN_TEST(fn_info_create_test);
    MU_RUN_TEST(fn_info_fn_state_test);
    MU_RUN_TEST(fn_info_ffd_version_test);
    MU_RUN_TEST(fn_info_max_ffd_version_test);
    MU_RUN_TEST(fn_info_serial_number_test);
    MU_RUN_TEST(fn_info_fw_version_test);
    MU_RUN_TEST(fn_info_is_session_open_test);
    MU_RUN_TEST(fn_info_last_doc_number_test);
    MU_RUN_TEST(fn_info_last_doc_date_time_test);
    MU_RUN_TEST(fn_info_fn_warn_flags_test);
    MU_RUN_TEST(fn_info_copy_test);
}

int run_minunit_test_fn_info() {
    MU_RUN_SUITE(fn_info_suite);
    return MU_EXIT_CODE;
}
