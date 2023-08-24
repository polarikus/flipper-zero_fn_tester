//
// Created by Игорь Данилов on 22.08.2023.
//
#include "unit_tests/minunit.h"
#include "unit_tests/base_test_vars.h"
#include "fn_objects/fn_info/fn_i.h"

static char* testSn = "999907890201361100";
static FN_FFD testFFD[] = {FFD_NONE, FFD_1_05, FFD_1_1, FFD_1_2, 100};
static char* testFFDString[] = {"Not registered", "1.05", "1.1", "1.2", "Unknown"};

static FNState testFNState[] = {FNStage1, FNStage2, FNStage3, FNStage4, 100};
static char* testFNStageString[] = {
    "Ready registration (1)",
    "Registered (2)",
    "The closed FN archive is awaiting transfer to the OFD (3)",
    "FN archive is closed (4)",
    "Unknown"};

static char* testFwVersion = "MgmTeest12345789";
static uint8_t testWarnFlags[] = {
    0b00000001,
    0b00000010,
    0b00000100,
    0b00001000,
    0b00010000,
    0b00100000,
    0b01000000,
    0b10000000};

FNInfo* fnInfo;

static void test_setup(void) {
    fnInfo = malloc(sizeof(FNInfo));
    BTV_UNUSED();
}

static void test_teardown(void) {
    free(fnInfo);
}

MU_TEST(fn_info_create_test) {
    mu_assert_not_null(fnInfo);
}

MU_TEST(fn_info_fn_state_test) {
    mu_assert_not_null(fnInfo);
    for(size_t i = 0; i < COUNT_OF(testFNState); i++) {
        fnInfo->fn_state = testFNState[i];
        mu_assert_int_eq(testFNState[i], fn_get_fn_state_enum(fnInfo));
        mu_assert_string_eq(testFNStageString[i], fn_get_fn_state(fnInfo));
    }
}

MU_TEST(fn_info_ffd_version_test) {
    mu_assert_not_null(fnInfo);
    //Тест версии ФФД
    for(size_t i = 0; i < COUNT_OF(testFFD); i++) {
        fnInfo->ffd_version = testFFD[i];
        mu_assert_int_eq(testFFD[i], fn_get_ffd_enum(fnInfo));
        mu_assert_string_eq(testFFDString[i], fn_get_ffd_version_string(fnInfo));
    }
}

MU_TEST(fn_info_max_ffd_version_test) {
    mu_assert_not_null(fnInfo);
    //Тест максимальной версии ФФД
    for(size_t i = 1; i < COUNT_OF(testFFD); i++) {
        fnInfo->max_ffd_version = testFFD[i];
        mu_assert_int_eq(testFFD[i], fn_get_max_ffd_enum(fnInfo));
        mu_assert_string_eq(testFFDString[i], fn_get_max_ffd_version_string(fnInfo));
    }
}

MU_TEST(fn_info_serial_number_test) {
    mu_assert_not_null(fnInfo);
    //Тест серийного номера ФН
    strcpy(fnInfo->serial_number, testSn);
    mu_assert_string_eq(testSn, fn_get_sn(fnInfo));
}

MU_TEST(fn_info_fw_version_test) {
    mu_assert_not_null(fnInfo);
    //Тест версии прошивки ФН
    strcpy(fnInfo->fw_version.fw_version, testFwVersion);
    mu_assert_string_eq(testFwVersion, fn_get_fw_version(fnInfo));
    //Тест на флаг является ли ФН МГМ
    for(size_t i = 0; i < COUNT_OF(testBoolVal); i++) {
        fnInfo->fw_version.fw_mode = testBoolVal[i];
        mu_assert_int_eq(testBoolVal[i], !fn_is_mgm(fnInfo));
    }
}

MU_TEST(fn_info_is_session_open_test) {
    mu_assert_not_null(fnInfo);
    //Тест флага открытия смены
    for(size_t i = 0; i < COUNT_OF(testBoolVal); i++) {
        fnInfo->is_session_open = testBoolVal[i];
        mu_assert_int_eq(testBoolVal[i], fn_is_session_open(fnInfo));
    }
}

MU_TEST(fn_info_last_doc_number_test) {
    mu_assert_not_null(fnInfo);

    fnInfo->last_doc_number = uint32TestVal[0]; //Тест максимального значения для uint32
    mu_assert_int_eq(uint32TestVal[0], fn_get_last_document_number(fnInfo));

    fnInfo->last_doc_number = uint32TestVal[1]; //Тест минимального значения для uint32
    mu_assert_int_eq(uint32TestVal[1], fn_get_last_document_number(fnInfo));
}

MU_TEST(fn_info_last_doc_date_time_test) {
    mu_assert_not_null(fnInfo);
    fnInfo->date_time.date =
        21; //TODO заменить на day да ив вообще совместить структуры Date и Datetime
    fnInfo->date_time.mouth = 8;
    fnInfo->date_time.year = 23;
    fnInfo->date_time.hour = 22;
    fnInfo->date_time.minute = 59;
    FuriString* dateTimeStr = furi_string_alloc();
    fn_get_last_document_datetime(fnInfo, dateTimeStr);
    mu_assert_string_eq("21.08.2023 22:59", furi_string_get_cstr(dateTimeStr));
    //TODO Добавить тест для не валидной даты и времени последнего ФД + написать проверку в функции;
}

MU_TEST(fn_info_fn_warn_flags_test) {
    mu_assert_not_null(fnInfo);
    //Тест на флаги предупреждения ФН
    fnInfo->fn_warn_flags = 0;
    mu_assert(!fn_is_warn_flags_not_null(fnInfo), "fn_warn_flags not null!");
    for(size_t i = 0; i < COUNT_OF(testWarnFlags); i++) {
        fnInfo->fn_warn_flags = testWarnFlags[i];
        mu_assert_int_eq(testWarnFlags[i], fnInfo->fn_warn_flags);
        mu_assert(fn_is_warn_flags_not_null(fnInfo), "fn_warn_flags not setted!");
    }
    //TODO Добавить тест fn_get_warn_flags_str(); для получения строк
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
}

int run_minunit_test_fn_info() {
    MU_RUN_SUITE(fn_info_suite);
    return MU_EXIT_CODE;
}
