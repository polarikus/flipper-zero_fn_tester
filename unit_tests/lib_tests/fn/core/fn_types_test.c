// On-device тесты типов и текстов (T013, R1).
#include "unit_tests/minunit.h"

#include "core/fn_types.h"

#include <string.h>

static FnDateTime make_datetime(uint8_t month, uint8_t day, uint8_t hour, uint8_t minute) {
    FnDateTime value;
    memset(&value, 0, sizeof(value));
    value.date.year = 26;
    value.date.month = month;
    value.date.day = day;
    value.hour = hour;
    value.minute = minute;
    return value;
}

MU_TEST(date_valid_test) {
    FnDate date = {.year = 26, .month = 1, .day = 1};
    mu_assert(fn_date_is_valid(&date), "1 января валидно");

    date.month = 0;
    mu_assert(!fn_date_is_valid(&date), "месяц 0 невалиден");
    date.month = 13;
    mu_assert(!fn_date_is_valid(&date), "месяц 13 невалиден");
    date.month = 12;
    date.day = 0;
    mu_assert(!fn_date_is_valid(&date), "день 0 невалиден");
    date.day = 32;
    mu_assert(!fn_date_is_valid(&date), "день 32 невалиден");
    mu_assert(!fn_date_is_valid(NULL), "NULL невалиден");
}

MU_TEST(datetime_valid_test) {
    FnDateTime value = make_datetime(12, 31, 23, 59);
    mu_assert(fn_datetime_is_valid(&value), "23:59 валидно");

    value.hour = 24;
    mu_assert(!fn_datetime_is_valid(&value), "час 24 невалиден");
    value.hour = 0;
    value.minute = 60;
    mu_assert(!fn_datetime_is_valid(&value), "минута 60 невалидна");
    value.minute = 0;
    value.date.month = 13;
    mu_assert(!fn_datetime_is_valid(&value), "невалидная дата делает время невалидным");
    mu_assert(!fn_datetime_is_valid(NULL), "NULL невалиден");
}

MU_TEST(stage_text_test) {
    mu_assert_string_eq("Готов к регистрации", fn_stage_text(FnStageReadyRegistration, FnLangRu));
    mu_assert_string_eq("Зарегистрирован", fn_stage_text(FnStageRegistered, FnLangRu));
    mu_assert_string_eq("Архив ФН закрыт", fn_stage_text(FnStageArchiveClosedFinal, FnLangRu));
    mu_assert_string_eq("Неизвестный этап", fn_stage_text(FnStageUnknown, FnLangRu));
    mu_assert_string_eq("Неизвестный этап", fn_stage_text((FnStage)0x42, FnLangRu));
}

MU_TEST(ffd_text_test) {
    mu_assert_string_eq("Не активизирован", fn_ffd_text(FnFfdNone, FnLangRu));
    mu_assert_string_eq("1.05", fn_ffd_text(FnFfd1_05, FnLangRu));
    mu_assert_string_eq("1.1", fn_ffd_text(FnFfd1_1, FnLangRu));
    mu_assert_string_eq("1.2", fn_ffd_text(FnFfd1_2, FnLangRu));
    mu_assert_string_eq("Неизвестно", fn_ffd_text(FnFfdUnknown, FnLangRu));
}

MU_TEST(doc_text_test) {
    mu_assert_string_eq("Документа нет", fn_doc_text(FnDocNone, FnLangRu));
    mu_assert_string_eq("Кассовый чек", fn_doc_text(FnDocReceipt, FnLangRu));
    mu_assert_string_eq("Отчет о закрытии смены", fn_doc_text(FnDocSessionClose, FnLangRu));
    mu_assert_string_eq("Неизвестный документ", fn_doc_text(FnDocUnknown, FnLangRu));
}

MU_TEST(warn_text_test) {
    mu_assert_string_eq(
        "Срочная замена ФН (до 3 дней)", fn_warn_flag_text(FnWarnReplaceSoon, FnLangRu));
    mu_assert_string_eq("ОФД аннулирован", fn_warn_flag_text(FnWarnOfdCancelled, FnLangRu));
    mu_assert(fn_warn_flag_text(FnWarnReserved, FnLangRu) == NULL, "0x80 не отдельный флаг");
    mu_assert(fn_warn_flag_text(0x00, FnLangRu) == NULL, "0x00 не флаг");
    mu_assert(fn_warn_flag_text(0x03, FnLangRu) == NULL, "комбинация флагов не флаг");

    mu_assert_int_eq(0x80, fn_warn_flags_unknown(0x83));
    mu_assert_int_eq(0x00, fn_warn_flags_unknown(0x40));
}

MU_TEST_SUITE(fn_types_test_suite) {
    MU_RUN_TEST(date_valid_test);
    MU_RUN_TEST(datetime_valid_test);
    MU_RUN_TEST(stage_text_test);
    MU_RUN_TEST(ffd_text_test);
    MU_RUN_TEST(doc_text_test);
    MU_RUN_TEST(warn_text_test);
}

int run_minunit_test_fn_types(void) {
    MU_RUN_SUITE(fn_types_test_suite);
    return MU_EXIT_CODE;
}
