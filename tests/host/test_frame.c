// Host-тесты кодека кадра: golden-кадры и негативные случаи (T010, R1).
#include "fn_frame.h"
#include "golden_frames.h"
#include "test_common.h"

#include <string.h>

static void test_build_golden(void) {
    uint8_t out[FN_FRAME_MAX_LEN];
    FnFrameStatus status = FnFrameOk;

    for(size_t i = 0; i < sizeof(golden_requests) / sizeof(golden_requests[0]); ++i) {
        const GoldenFrame* golden = &golden_requests[i];
        const uint8_t command = golden->data[3];
        const uint8_t* data = &golden->data[4];
        const size_t data_len = golden->len - FN_FRAME_OVERHEAD;

        status = FnFrameNull;
        const size_t built = fn_frame_build(command, data, data_len, out, sizeof(out), &status);
        if(built != golden->len || memcmp(out, golden->data, built) != 0) {
            fprintf(stderr, "FAIL build %s\n", golden->name);
        }
        TEST_CHECK(status == FnFrameOk);
        TEST_CHECK_EQ_U16(built, golden->len);
        TEST_CHECK(memcmp(out, golden->data, golden->len) == 0);
    }
}

static void test_parse_golden(void) {
    for(size_t i = 0; i < sizeof(golden_responses) / sizeof(golden_responses[0]); ++i) {
        const GoldenFrame* golden = &golden_responses[i];
        FnFrame frame;
        memset(&frame, 0, sizeof(frame));
        const FnFrameStatus status = fn_frame_parse(golden->data, golden->len, &frame);
        if(status != FnFrameOk) {
            fprintf(stderr, "FAIL parse %s: status %d\n", golden->name, status);
        }
        TEST_CHECK(status == FnFrameOk);
        if(status != FnFrameOk) continue;
        TEST_CHECK_EQ_U16(frame.data_len, golden->len - FN_FRAME_OVERHEAD);
    }

    FnFrame frame;
    TEST_CHECK(fn_frame_parse(golden_resp_30h, sizeof(golden_resp_30h), &frame) == FnFrameOk);
    TEST_CHECK(frame.code == 0x00);
    TEST_CHECK(!frame.has_warning);
    TEST_CHECK_EQ_U16(frame.data_len, 30);
    TEST_CHECK(frame.data[0] == 0x03); // этап 2

    TEST_CHECK(
        fn_frame_parse(golden_resp_30h_warn, sizeof(golden_resp_30h_warn), &frame) == FnFrameOk);
    TEST_CHECK(frame.code == 0x00);
    TEST_CHECK(frame.has_warning);
    TEST_CHECK(frame.data[4] == 0x02); // флаг «исчерпание ресурса»

    TEST_CHECK(
        fn_frame_parse(golden_resp_error_03h, sizeof(golden_resp_error_03h), &frame) == FnFrameOk);
    TEST_CHECK(frame.code == 0x03);
    TEST_CHECK_EQ_U16(frame.data_len, 0);
}

static void test_negative(void) {
    FnFrame frame;
    uint8_t buf[FN_FRAME_MAX_LEN + 1];
    memcpy(buf, golden_req_30h, sizeof(golden_req_30h));

    TEST_CHECK(fn_frame_parse(NULL, 6, &frame) == FnFrameNull);
    TEST_CHECK(fn_frame_parse(golden_req_30h, sizeof(golden_req_30h), NULL) == FnFrameNull);
    TEST_CHECK(fn_frame_parse(golden_req_30h, 5, &frame) == FnFrameTooShort);
    TEST_CHECK(fn_frame_parse(buf, FN_FRAME_MAX_LEN + 1, &frame) == FnFrameTooLong);

    buf[0] = 0x05;
    TEST_CHECK(fn_frame_parse(buf, 6, &frame) == FnFrameWrongStart);
    buf[0] = 0x04;

    buf[1] = 0x02; // LEN на 1 больше
    TEST_CHECK(fn_frame_parse(buf, 6, &frame) == FnFrameLengthMismatch);
    buf[1] = 0x00; // LEN на 1 меньше
    TEST_CHECK(fn_frame_parse(buf, 6, &frame) == FnFrameLengthMismatch);
    buf[1] = 0x01;

    buf[5] ^= 0xFF; // битый CRC
    TEST_CHECK(fn_frame_parse(buf, 6, &frame) == FnFrameWrongCrc);
    buf[5] ^= 0xFF;
    TEST_CHECK(fn_frame_parse(buf, 6, &frame) == FnFrameOk);

    // Граница payload: LEN = 250.
    uint8_t payload[FN_FRAME_MAX_PAYLOAD];
    memset(payload, 0xA5, sizeof(payload));
    FnFrameStatus status = FnFrameNull;
    const size_t built = fn_frame_build(0x30, payload, sizeof(payload), buf, sizeof(buf), &status);
    TEST_CHECK(status == FnFrameOk);
    TEST_CHECK_EQ_U16(built, FN_FRAME_MAX_LEN);
    TEST_CHECK(fn_frame_parse(buf, built, &frame) == FnFrameOk);
    TEST_CHECK_EQ_U16(frame.data_len, FN_FRAME_MAX_PAYLOAD);

    // Переполнение payload и малый буфер сборки.
    TEST_CHECK(fn_frame_build(0x30, payload, sizeof(payload) + 1, buf, sizeof(buf), &status) == 0);
    TEST_CHECK(status == FnFrameTooLong);
    TEST_CHECK(fn_frame_build(0x30, payload, 10, buf, 8, &status) == 0);
    TEST_CHECK(fn_frame_build(0x30, NULL, 4, buf, sizeof(buf), &status) == 0);
    TEST_CHECK(fn_frame_build(0x30, NULL, 0, NULL, 0, &status) == 0);
    TEST_CHECK(status == FnFrameNull);
}

int main(void) {
    test_build_golden();
    test_parse_golden();
    test_negative();
    return TEST_SUMMARY();
}
