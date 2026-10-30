// On-device тесты кодека кадра (T012, R1): golden-кадры и негативные случаи.
#include "unit_tests/minunit.h"

#include <furi.h>

#include "core/fn_frame.h"
#include "tests/host/golden_frames.h"

#include <string.h>

MU_TEST(build_golden_test) {
    uint8_t out[FN_FRAME_MAX_LEN];
    for(size_t i = 0; i < COUNT_OF(golden_requests); ++i) {
        const GoldenFrame* golden = &golden_requests[i];
        FnFrameStatus status = FnFrameNull;
        const size_t built = fn_frame_build(
            golden->data[3],
            &golden->data[4],
            golden->len - FN_FRAME_OVERHEAD,
            out,
            sizeof(out),
            &status);
        mu_assert_int_eq(FnFrameOk, status);
        mu_assert_int_eq((int)golden->len, (int)built);
        mu_assert_mem_eq(golden->data, out, golden->len);
    }
}

MU_TEST(parse_golden_test) {
    FnFrame frame;
    for(size_t i = 0; i < COUNT_OF(golden_responses); ++i) {
        const GoldenFrame* golden = &golden_responses[i];
        memset(&frame, 0, sizeof(frame));
        mu_assert_int_eq(FnFrameOk, fn_frame_parse(golden->data, golden->len, &frame));
        mu_assert_int_eq((int)(golden->len - FN_FRAME_OVERHEAD), (int)frame.data_len);
    }
}

MU_TEST(warning_flag_test) {
    FnFrame frame;
    mu_assert_int_eq(FnFrameOk, fn_frame_parse(golden_resp_30h, sizeof(golden_resp_30h), &frame));
    mu_assert_int_eq(0x00, frame.code);
    mu_assert(!frame.has_warning, "30h без предупреждения");

    mu_assert_int_eq(
        FnFrameOk, fn_frame_parse(golden_resp_30h_warn, sizeof(golden_resp_30h_warn), &frame));
    mu_assert_int_eq(0x00, frame.code);
    mu_assert(frame.has_warning, "бит 0x80 должен давать has_warning");
}

MU_TEST(refusal_test) {
    FnFrame frame;
    mu_assert_int_eq(
        FnFrameOk, fn_frame_parse(golden_resp_error_03h, sizeof(golden_resp_error_03h), &frame));
    mu_assert_int_eq(0x03, frame.code);
    mu_assert_int_eq(0, (int)frame.data_len);
}

MU_TEST(negative_test) {
    FnFrame frame;
    uint8_t buf[FN_FRAME_MAX_LEN + 1];
    memcpy(buf, golden_req_30h, sizeof(golden_req_30h));

    mu_assert_int_eq(FnFrameNull, fn_frame_parse(NULL, 6, &frame));
    mu_assert_int_eq(FnFrameNull, fn_frame_parse(golden_req_30h, sizeof(golden_req_30h), NULL));
    mu_assert_int_eq(FnFrameTooShort, fn_frame_parse(golden_req_30h, 5, &frame));
    mu_assert_int_eq(FnFrameTooLong, fn_frame_parse(buf, FN_FRAME_MAX_LEN + 1, &frame));

    buf[0] = 0x05;
    mu_assert_int_eq(FnFrameWrongStart, fn_frame_parse(buf, 6, &frame));
    buf[0] = 0x04;

    buf[1] = 0x02;
    mu_assert_int_eq(FnFrameLengthMismatch, fn_frame_parse(buf, 6, &frame));
    buf[1] = 0x00;
    mu_assert_int_eq(FnFrameLengthMismatch, fn_frame_parse(buf, 6, &frame));
    buf[1] = 0x01;

    buf[5] ^= 0xFF;
    mu_assert_int_eq(FnFrameWrongCrc, fn_frame_parse(buf, 6, &frame));
    buf[5] ^= 0xFF;
    mu_assert_int_eq(FnFrameOk, fn_frame_parse(buf, 6, &frame));
}

MU_TEST(payload_border_test) {
    uint8_t payload[FN_FRAME_MAX_PAYLOAD];
    memset(payload, 0xA5, sizeof(payload));
    uint8_t out[FN_FRAME_MAX_LEN];
    FnFrameStatus status = FnFrameNull;

    const size_t built = fn_frame_build(0x30, payload, sizeof(payload), out, sizeof(out), &status);
    mu_assert_int_eq(FnFrameOk, status);
    mu_assert_int_eq(FN_FRAME_MAX_LEN, (int)built);

    FnFrame frame;
    mu_assert_int_eq(FnFrameOk, fn_frame_parse(out, built, &frame));
    mu_assert_int_eq(FN_FRAME_MAX_PAYLOAD, (int)frame.data_len);

    mu_assert_int_eq(
        0, (int)fn_frame_build(0x30, payload, sizeof(payload) + 1, out, sizeof(out), &status));
    mu_assert_int_eq(FnFrameTooLong, status);
    mu_assert_int_eq(0, (int)fn_frame_build(0x30, payload, 10, out, 8, &status));
    mu_assert_int_eq(0, (int)fn_frame_build(0x30, NULL, 4, out, sizeof(out), &status));
    mu_assert_int_eq(0, (int)fn_frame_build(0x30, NULL, 0, NULL, 0, &status));
    mu_assert_int_eq(FnFrameNull, status);
}

MU_TEST_SUITE(fn_frame_test_suite) {
    MU_RUN_TEST(build_golden_test);
    MU_RUN_TEST(parse_golden_test);
    MU_RUN_TEST(warning_flag_test);
    MU_RUN_TEST(refusal_test);
    MU_RUN_TEST(negative_test);
    MU_RUN_TEST(payload_border_test);
}

int run_minunit_test_fn_frame(void) {
    MU_RUN_SUITE(fn_frame_test_suite);
    return MU_EXIT_CODE;
}
