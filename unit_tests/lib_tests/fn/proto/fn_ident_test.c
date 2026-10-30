// On-device тесты парсеров 3Ah/33h (T023, R2).
#include "unit_tests/minunit.h"

#include "proto/fn_ident.h"
#include "tests/host/golden_frames.h"

#include <string.h>

static const uint8_t* payload(const GoldenFrame* frame) {
    return &frame->data[4];
}

static size_t payload_len(const GoldenFrame* frame) {
    return frame->len - 6;
}

MU_TEST(ffd_3ah_test) {
    FnFfdInfo ffd;
    mu_assert(
        fn_parse_ffd_3ah(payload(&golden_responses[3]), payload_len(&golden_responses[3]), &ffd),
        "3Ah должен разобраться");
    mu_assert_int_eq(FnFfd1_2, ffd.current);
    mu_assert_int_eq(FnFfd1_2, ffd.max);

    const uint8_t unknown_ffd[] = {0x09, 0x04};
    mu_assert(fn_parse_ffd_3ah(unknown_ffd, sizeof(unknown_ffd), &ffd), "неизвестный ФФД");
    mu_assert_int_eq(FnFfdUnknown, ffd.current);
    mu_assert_int_eq(0x09, ffd.current_raw);
    mu_assert(!fn_parse_ffd_3ah(unknown_ffd, 1, &ffd), "короткий буфер");
    mu_assert(!fn_parse_ffd_3ah(NULL, 2, &ffd), "NULL-буфер");
    mu_assert(!fn_parse_ffd_3ah(unknown_ffd, 2, NULL), "NULL-выход");
}

MU_TEST(fw_33h_test) {
    uint8_t data[17];
    memset(data, 0, sizeof(data));
    memcpy(data, "FN-1.0.0", 8);

    FnFwInfo info;
    mu_assert(fn_parse_fw_33h(data, sizeof(data), &info), "33h должен разобраться");
    mu_assert_string_eq("FN-1.0.0", info.version);
    mu_assert_int_eq(0, info.mode_raw);
    mu_assert(info.is_mgm, "mode_raw = 0 — опытный образец (МГМ)");

    data[16] = 1;
    mu_assert(fn_parse_fw_33h(data, sizeof(data), &info), "33h серийного изделия");
    mu_assert(!info.is_mgm, "mode_raw = 1 — боевой ФН");
    mu_assert(!fn_parse_fw_33h(data, 16, &info), "короткий буфер");
    mu_assert(!fn_parse_fw_33h(NULL, 17, &info), "NULL-буфер");
}

MU_TEST_SUITE(fn_ident_test_suite) {
    MU_RUN_TEST(ffd_3ah_test);
    MU_RUN_TEST(fw_33h_test);
}

int run_minunit_test_fn_ident(void) {
    MU_RUN_SUITE(fn_ident_test_suite);
    return MU_EXIT_CODE;
}
