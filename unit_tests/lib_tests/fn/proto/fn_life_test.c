// On-device тесты парсеров 32h/3Bh/3Dh (T024, R2).
#include "unit_tests/minunit.h"

#include "proto/fn_life.h"
#include "tests/host/golden_frames.h"

#include <string.h>

static const uint8_t* payload(const GoldenFrame* frame) {
    return &frame->data[4];
}

static size_t payload_len(const GoldenFrame* frame) {
    return frame->len - 6;
}

MU_TEST(end_date_32h_test) {
    FnLifeDate date;
    mu_assert(
        fn_parse_end_date_32h(
            payload(&golden_responses[2]), payload_len(&golden_responses[2]), &date),
        "32h должен разобраться");
    mu_assert_int_eq(27, date.end_date.year);
    mu_assert_int_eq(12, date.end_date.month);
    mu_assert_int_eq(31, date.end_date.day);
    mu_assert_int_eq(12, date.reg_remaining);
    mu_assert_int_eq(3, date.reg_done);

    mu_assert(!fn_parse_end_date_32h(payload(&golden_responses[2]), 4, &date), "короткий буфер");
    mu_assert(!fn_parse_end_date_32h(NULL, 5, &date), "NULL-буфер");
}

MU_TEST(end_days_3bh_test) {
    FnLifeDays days;
    mu_assert(
        fn_parse_end_days_3bh(
            payload(&golden_responses[4]), payload_len(&golden_responses[4]), &days),
        "3Bh должен разобраться");
    mu_assert_int_eq(360, days.days);
    mu_assert(days.has_days, "360 дней достоверны");

    const uint8_t zero_days[] = {0x00, 0x00};
    mu_assert(fn_parse_end_days_3bh(zero_days, sizeof(zero_days), &days), "0 разбирается");
    mu_assert(!days.has_days, "0 дней — данных нет");
    mu_assert(!fn_parse_end_days_3bh(zero_days, 1, &days), "короткий буфер");
}

MU_TEST(memory_3dh_test) {
    FnMemory memory;
    mu_assert(
        fn_parse_memory_3dh(
            payload(&golden_responses[6]), payload_len(&golden_responses[6]), &memory),
        "3Dh 8 байт должен разобраться");
    mu_assert_int_eq(10000, (int)memory.five_year_documents);
    mu_assert_int_eq(300, (int)memory.thirty_day_kb);
    mu_assert(!memory.has_marking, "8 байт — маркировки нет");
    mu_assert_int_eq(0, memory.marking_percent);

    mu_assert(
        fn_parse_memory_3dh(
            payload(&golden_responses[7]), payload_len(&golden_responses[7]), &memory),
        "3Dh 9 байт должен разобраться");
    mu_assert(memory.has_marking, "9 байт — маркировка есть");
    mu_assert_int_eq(7, memory.marking_percent);

    FnMemory keep;
    memset(&keep, 0xA5, sizeof(keep));
    mu_assert(!fn_parse_memory_3dh(payload(&golden_responses[7]), 7, &keep), "короткий буфер");
    mu_assert(!fn_parse_memory_3dh(payload(&golden_responses[7]), 10, &keep), "длинный буфер");
    mu_assert(!fn_parse_memory_3dh(NULL, 8, &keep), "NULL-буфер");
    mu_assert_int_eq(0xA5, ((const uint8_t*)&keep)[0]);
}

MU_TEST(aggregate_test) {
    FnLifeInfo* life = fn_life_info_alloc();
    mu_assert(life != NULL, "нет памяти под FnLifeInfo");
    mu_assert(!life->has_date && !life->has_days && !life->has_memory, "начальное состояние");

    life->has_days = true;
    life->days.days = 10;
    fn_life_info_reset(life);
    mu_assert(!life->has_days, "reset очищает флаги");
    mu_assert_int_eq(0, life->days.days);
    fn_life_info_free(life);
}

MU_TEST_SUITE(fn_life_test_suite) {
    MU_RUN_TEST(end_date_32h_test);
    MU_RUN_TEST(end_days_3bh_test);
    MU_RUN_TEST(memory_3dh_test);
    MU_RUN_TEST(aggregate_test);
}

int run_minunit_test_fn_life(void) {
    MU_RUN_SUITE(fn_life_test_suite);
    return MU_EXIT_CODE;
}
