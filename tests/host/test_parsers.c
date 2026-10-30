// Host-тесты парсеров proto/* (T021, R2).
#include "fn_ident.h"
#include "fn_info.h"
#include "fn_life.h"
#include "fn_status.h"
#include "golden_frames.h"
#include "test_common.h"

#include <string.h>

// Данные ответа = кадр без START/LEN/ANSWER и двух байт CRC.
static const uint8_t* payload(const GoldenFrame* frame) {
    return &frame->data[4];
}

static size_t payload_len(const GoldenFrame* frame) {
    return frame->len - 6;
}

static void test_status_30h(void) {
    FnStatus status;
    memset(&status, 0xFF, sizeof(status));

    TEST_CHECK(fn_parse_status_30h(
        payload(&golden_responses[0]), payload_len(&golden_responses[0]), &status));
    TEST_CHECK(status.stage == FnStageRegistered);
    TEST_CHECK_EQ_U16(status.stage_raw, 0x03);
    TEST_CHECK(status.current_doc == FnDocNone);
    TEST_CHECK(!status.doc_data_received);
    TEST_CHECK(status.session_open);
    TEST_CHECK_EQ_U16(status.warn_flags, 0x00);
    TEST_CHECK(status.has_last_doc);
    TEST_CHECK(status.has_last_doc_number);
    TEST_CHECK_EQ_U16(status.last_doc_dt.date.year, 26);
    TEST_CHECK_EQ_U16(status.last_doc_dt.date.month, 10);
    TEST_CHECK_EQ_U16(status.last_doc_dt.date.day, 29);
    TEST_CHECK_EQ_U16(status.last_doc_dt.hour, 12);
    TEST_CHECK_EQ_U16(status.last_doc_dt.minute, 30);
    TEST_CHECK(strcmp(status.serial, "9999078900001234") == 0);
    TEST_CHECK_EQ_U32(status.last_doc_number, 42);
    TEST_CHECK_EQ_U16(status.serial[16], '\0');

    // Предупреждение: код 0, флаг 0x02.
    TEST_CHECK(fn_parse_status_30h(
        payload(&golden_responses[1]), payload_len(&golden_responses[1]), &status));
    TEST_CHECK_EQ_U16(status.warn_flags, 0x02);

    // Невалидная дата (месяц 13) → оба has_last_doc* = false.
    uint8_t data[30];
    memcpy(data, payload(&golden_responses[0]), sizeof(data));
    data[6] = 13;
    TEST_CHECK(fn_parse_status_30h(data, sizeof(data), &status));
    TEST_CHECK(!status.has_last_doc);
    TEST_CHECK(!status.has_last_doc_number);

    // Короткий буфер и NULL: *out не меняется.
    FnStatus keep;
    memset(&keep, 0xA5, sizeof(keep));
    TEST_CHECK(!fn_parse_status_30h(data, 29, &keep));
    TEST_CHECK(!fn_parse_status_30h(NULL, 30, &keep));
    TEST_CHECK(!fn_parse_status_30h(data, 30, NULL));
    TEST_CHECK(keep.stage_raw == 0xA5);
}

static void test_session_10h(void) {
    const uint8_t data[] = {0x01, 0x2C, 0x01, 0x05, 0x00};
    FnSession session;
    TEST_CHECK(fn_parse_session_10h(data, sizeof(data), &session));
    TEST_CHECK(session.session_open);
    TEST_CHECK_EQ_U16(session.shift_number, 300);
    TEST_CHECK_EQ_U16(session.receipt_number, 5);
    TEST_CHECK(!fn_parse_session_10h(data, sizeof(data) - 1, &session));
    TEST_CHECK(!fn_parse_session_10h(NULL, sizeof(data), &session));
}

static void test_ident(void) {
    FnFfdInfo ffd;
    TEST_CHECK(
        fn_parse_ffd_3ah(payload(&golden_responses[3]), payload_len(&golden_responses[3]), &ffd));
    TEST_CHECK(ffd.current == FnFfd1_2);
    TEST_CHECK(ffd.max == FnFfd1_2);

    const uint8_t unknown_ffd[] = {0x09, 0x04};
    TEST_CHECK(fn_parse_ffd_3ah(unknown_ffd, sizeof(unknown_ffd), &ffd));
    TEST_CHECK(ffd.current == FnFfdUnknown);
    TEST_CHECK_EQ_U16(ffd.current_raw, 0x09);
    TEST_CHECK(!fn_parse_ffd_3ah(unknown_ffd, 1, &ffd));

    uint8_t fw[17];
    memset(fw, 0, sizeof(fw));
    memcpy(fw, "FN-1.0.0", 8);
    fw[16] = 0;
    FnFwInfo info;
    TEST_CHECK(fn_parse_fw_33h(fw, sizeof(fw), &info));
    TEST_CHECK(strcmp(info.version, "FN-1.0.0") == 0);
    TEST_CHECK(info.is_mgm);

    fw[16] = 1;
    TEST_CHECK(fn_parse_fw_33h(fw, sizeof(fw), &info));
    TEST_CHECK(!info.is_mgm);
    TEST_CHECK(!fn_parse_fw_33h(fw, 16, &info));
}

static void test_life(void) {
    FnLifeDate date;
    TEST_CHECK(fn_parse_end_date_32h(
        payload(&golden_responses[2]), payload_len(&golden_responses[2]), &date));
    TEST_CHECK_EQ_U16(date.end_date.year, 27);
    TEST_CHECK_EQ_U16(date.end_date.month, 12);
    TEST_CHECK_EQ_U16(date.end_date.day, 31);
    TEST_CHECK_EQ_U16(date.reg_remaining, 12);
    TEST_CHECK_EQ_U16(date.reg_done, 3);
    TEST_CHECK(!fn_parse_end_date_32h(payload(&golden_responses[2]), 4, &date));

    FnLifeDays days;
    TEST_CHECK(fn_parse_end_days_3bh(
        payload(&golden_responses[4]), payload_len(&golden_responses[4]), &days));
    TEST_CHECK_EQ_U16(days.days, 360);
    TEST_CHECK(days.has_days);

    const uint8_t zero_days[] = {0x00, 0x00};
    TEST_CHECK(fn_parse_end_days_3bh(zero_days, sizeof(zero_days), &days));
    TEST_CHECK(!days.has_days);

    FnMemory memory;
    TEST_CHECK(fn_parse_memory_3dh(
        payload(&golden_responses[6]), payload_len(&golden_responses[6]), &memory));
    TEST_CHECK_EQ_U32(memory.five_year_documents, 10000);
    TEST_CHECK_EQ_U32(memory.thirty_day_kb, 300);
    TEST_CHECK(!memory.has_marking);
    TEST_CHECK_EQ_U16(memory.marking_percent, 0);

    TEST_CHECK(fn_parse_memory_3dh(
        payload(&golden_responses[7]), payload_len(&golden_responses[7]), &memory));
    TEST_CHECK(memory.has_marking);
    TEST_CHECK_EQ_U16(memory.marking_percent, 7);

    FnMemory keep;
    memset(&keep, 0xA5, sizeof(keep));
    TEST_CHECK(!fn_parse_memory_3dh(payload(&golden_responses[7]), 7, &keep));
    TEST_CHECK(!fn_parse_memory_3dh(payload(&golden_responses[7]), 10, &keep));
    TEST_CHECK(!fn_parse_memory_3dh(NULL, 8, &keep));
    // При ошибке парсер не трогает выходной буфер: остаются байты 0xA5.
    const uint8_t* keep_bytes = (const uint8_t*)&keep;
    bool untouched = true;
    for(size_t i = 0; i < sizeof(keep); ++i) {
        if(keep_bytes[i] != 0xA5) untouched = false;
    }
    TEST_CHECK(untouched);
}

static void test_aggregates(void) {
    FnInfo* info = fn_info_alloc();
    TEST_CHECK(info != NULL);
    TEST_CHECK(info->status.serial[0] == '\0');
    TEST_CHECK(!info->has_ffd);
    fn_info_free(info);

    FnLifeInfo* life = fn_life_info_alloc();
    TEST_CHECK(life != NULL);
    life->has_days = true;
    fn_life_info_reset(life);
    TEST_CHECK(!life->has_days);
    fn_life_info_free(life);
}

int main(void) {
    test_status_30h();
    test_session_10h();
    test_ident();
    test_life();
    test_aggregates();
    return TEST_SUMMARY();
}
