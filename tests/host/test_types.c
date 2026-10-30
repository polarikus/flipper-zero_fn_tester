// Host-тесты базовых типов (T010, T007a).
#include "fn_types.h"
#include "test_common.h"

#include <string.h>

static void test_dates(void) {
    FnDate date = {.year = 26, .month = 10, .day = 30};
    TEST_CHECK(fn_date_is_valid(&date));

    date.month = 0;
    TEST_CHECK(!fn_date_is_valid(&date));
    date.month = 13;
    TEST_CHECK(!fn_date_is_valid(&date));

    date.month = 1;
    date.day = 0;
    TEST_CHECK(!fn_date_is_valid(&date));
    date.day = 32;
    TEST_CHECK(!fn_date_is_valid(&date));

    date.day = 31;
    TEST_CHECK(fn_date_is_valid(&date));

    TEST_CHECK(!fn_date_is_valid(NULL));

    FnDateTime dt = {.date = {.year = 26, .month = 12, .day = 1}, .hour = 23, .minute = 59};
    TEST_CHECK(fn_datetime_is_valid(&dt));
    dt.hour = 24;
    TEST_CHECK(!fn_datetime_is_valid(&dt));
    dt.hour = 0;
    dt.minute = 60;
    TEST_CHECK(!fn_datetime_is_valid(&dt));
    TEST_CHECK(!fn_datetime_is_valid(NULL));
}

static void test_texts(void) {
    TEST_CHECK(strcmp(fn_stage_text(FnStageReadyRegistration), "Готов к регистрации") == 0);
    TEST_CHECK(strcmp(fn_stage_text(FnStageRegistered), "Зарегистрирован") == 0);
    TEST_CHECK(
        strcmp(fn_stage_text(FnStageArchiveClosed), "Архив закрыт, ждет передачи в ОФД") == 0);
    TEST_CHECK(strcmp(fn_stage_text(FnStageArchiveClosedFinal), "Архив ФН закрыт") == 0);
    TEST_CHECK(strcmp(fn_stage_text(FnStageUnknown), "Неизвестный этап") == 0);

    TEST_CHECK(strcmp(fn_ffd_text(FnFfdNone), "Не активизирован") == 0);
    TEST_CHECK(strcmp(fn_ffd_text(FnFfd1_05), "1.05") == 0);
    TEST_CHECK(strcmp(fn_ffd_text(FnFfd1_1), "1.1") == 0);
    TEST_CHECK(strcmp(fn_ffd_text(FnFfd1_2), "1.2") == 0);
    TEST_CHECK(strcmp(fn_ffd_text(FnFfdUnknown), "Неизвестно") == 0);

    TEST_CHECK(strcmp(fn_doc_text(FnDocNone), "Документа нет") == 0);
    TEST_CHECK(strcmp(fn_doc_text(FnDocReceipt), "Кассовый чек") == 0);
    TEST_CHECK(strcmp(fn_doc_text(FnDocUnknown), "Неизвестный документ") == 0);

    TEST_CHECK(fn_warn_flag_text(0x01) != NULL);
    TEST_CHECK(fn_warn_flag_text(0x40) != NULL);
    TEST_CHECK(fn_warn_flag_text(0x80) == NULL);
    TEST_CHECK(fn_warn_flag_text(0x00) == NULL);
    TEST_CHECK(fn_warn_flag_text(0x03) == NULL);

    TEST_CHECK_EQ_U16(fn_warn_flags_unknown(0x82), 0x80);
    TEST_CHECK_EQ_U16(fn_warn_flags_unknown(0x02), 0x00);
}

int main(void) {
    test_dates();
    test_texts();
    return TEST_SUMMARY();
}
