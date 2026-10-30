// On-device тесты парсеров 30h/10h (T022, R2).
#include "unit_tests/minunit.h"

#include "proto/fn_status.h"
#include "tests/host/golden_frames.h"

#include <string.h>

// Данные ответа = кадр без START/LEN/ANSWER и двух байт CRC.
static const uint8_t* payload(const GoldenFrame* frame) {
    return &frame->data[4];
}

static size_t payload_len(const GoldenFrame* frame) {
    return frame->len - 6;
}

MU_TEST(status_30h_test) {
    FnStatus status;
    memset(&status, 0xFF, sizeof(status));

    mu_assert(
        fn_parse_status_30h(
            payload(&golden_responses[0]), payload_len(&golden_responses[0]), &status),
        "30h должен разобраться");
    mu_assert_int_eq(FnStageRegistered, status.stage);
    mu_assert_int_eq(0x03, status.stage_raw);
    mu_assert_int_eq(FnDocNone, status.current_doc);
    mu_assert(!status.doc_data_received, "данные документа не получены");
    mu_assert(status.session_open, "смена открыта");
    mu_assert_int_eq(0x00, status.warn_flags);
    mu_assert(status.has_last_doc, "дата последнего ФД валидна");
    mu_assert(status.has_last_doc_number, "номер последнего ФД валиден");
    mu_assert_int_eq(26, status.last_doc_dt.date.year);
    mu_assert_int_eq(10, status.last_doc_dt.date.month);
    mu_assert_int_eq(29, status.last_doc_dt.date.day);
    mu_assert_int_eq(12, status.last_doc_dt.hour);
    mu_assert_int_eq(30, status.last_doc_dt.minute);
    mu_assert_string_eq("9999078900001234", status.serial);
    mu_assert_int_eq(42, (int)status.last_doc_number);
    mu_assert_int_eq('\0', status.serial[16]);
}

MU_TEST(status_30h_warning_test) {
    FnStatus status;
    mu_assert(
        fn_parse_status_30h(
            payload(&golden_responses[1]), payload_len(&golden_responses[1]), &status),
        "30h с предупреждением должен разобраться");
    mu_assert_int_eq(0x02, status.warn_flags);
}

MU_TEST(status_30h_invalid_date_test) {
    uint8_t data[30];
    memcpy(data, payload(&golden_responses[0]), sizeof(data));
    data[6] = 13; // месяц вне 1..12

    FnStatus status;
    mu_assert(fn_parse_status_30h(data, sizeof(data), &status), "парсер не отказывает");
    mu_assert(!status.has_last_doc, "невалидная дата отбрасывается");
    mu_assert(!status.has_last_doc_number, "номер ФД тоже недостоверен");
}

MU_TEST(status_30h_negative_test) {
    uint8_t data[30];
    memcpy(data, payload(&golden_responses[0]), sizeof(data));

    FnStatus keep;
    memset(&keep, 0xA5, sizeof(keep));
    mu_assert(!fn_parse_status_30h(data, 29, &keep), "короткий буфер");
    mu_assert(!fn_parse_status_30h(NULL, 30, &keep), "NULL-буфер");
    mu_assert(!fn_parse_status_30h(data, 30, NULL), "NULL-выход");
    mu_assert_int_eq(0xA5, keep.stage_raw);
}

MU_TEST(session_10h_test) {
    const uint8_t data[] = {0x01, 0x2C, 0x01, 0x05, 0x00};
    FnSession session;
    mu_assert(fn_parse_session_10h(data, sizeof(data), &session), "10h должен разобраться");
    mu_assert(session.session_open, "смена открыта");
    mu_assert_int_eq(300, session.shift_number);
    mu_assert_int_eq(5, session.receipt_number);
    mu_assert(!fn_parse_session_10h(data, sizeof(data) - 1, &session), "короткий буфер");
    mu_assert(!fn_parse_session_10h(NULL, sizeof(data), &session), "NULL-буфер");
}

MU_TEST_SUITE(fn_status_test_suite) {
    MU_RUN_TEST(status_30h_test);
    MU_RUN_TEST(status_30h_warning_test);
    MU_RUN_TEST(status_30h_invalid_date_test);
    MU_RUN_TEST(status_30h_negative_test);
    MU_RUN_TEST(session_10h_test);
}

int run_minunit_test_fn_status(void) {
    MU_RUN_SUITE(fn_status_test_suite);
    return MU_EXIT_CODE;
}
