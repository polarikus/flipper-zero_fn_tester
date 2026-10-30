// Host-тесты сборки экрана результата (T065, US3).
#include "fn_format.h"
#include "fn_strings.h"
#include "test_common.h"

#include <stdio.h>
#include <string.h>

static const FnTextSection* find_section(const FnTextModel* model, const char* title) {
    for(uint8_t s = 0; s < model->section_count; ++s) {
        const FnTextSection* section = &model->sections[s];
        if(section->title && strcmp(section->title, title) == 0) return section;
    }
    return NULL;
}

static bool section_has_line(const FnTextSection* section, const char* text) {
    if(!section) return false;
    for(uint8_t l = 0; l < section->line_count; ++l) {
        if(strcmp(section->lines[l].text, text) == 0) return true;
    }
    return false;
}

static bool section_has_prefix(const FnTextSection* section, const char* prefix) {
    if(!section) return false;
    const size_t len = strlen(prefix);
    for(uint8_t l = 0; l < section->line_count; ++l) {
        if(strncmp(section->lines[l].text, prefix, len) == 0) return true;
    }
    return false;
}

static void expect_line(const FnTextModel* model, const char* text) {
    const FnTextSection* section = find_section(model, FN_STR_RESULT_TITLE);
    if(!section_has_line(section, text)) {
        fprintf(stderr, "FAIL нет строки результата: %s\n", text);
        TEST_CHECK(false);
    } else {
        TEST_CHECK(true);
    }
}

static FnOpResult make_result(FnOpKind kind, FnOpStatus status, FNError error) {
    FnOpResult result;
    memset(&result, 0, sizeof(result));
    result.id = 7;
    result.kind = kind;
    result.status = status;
    result.error = error;
    return result;
}

static void test_flash_success(void) {
    const FnOpResult result = make_result(FnOpFlashMgm, FnOpStatusOk, FNOk);
    FnTextModel model;
    fn_format_result(&result, FnResultReasonOperation, &model);

    TEST_CHECK_EQ_U16(model.section_count, 1);
    expect_line(&model, FN_STR_RESULT_OK);
}

static void test_flash_not_mgm(void) {
    /* Preflight: команда не отправлялась, показываем недоступность (FR-024). */
    const FnOpResult result = make_result(FnOpDetect, FnOpStatusOk, FNOk);
    FnTextModel model;
    fn_format_result(&result, FnResultReasonNotMgm, &model);

    expect_line(&model, FN_STR_NOT_MGM);
}

static void test_refused_known_code(void) {
    const FnOpResult result = make_result(FnOpFlashMgm, FnOpStatusFnRefused, FNErrorFailure);
    FnTextModel model;
    fn_format_result(&result, FnResultReasonOperation, &model);

    expect_line(&model, "Отказ ФН");
    expect_line(&model, "Запросите детали командой 35h");
    expect_line(&model, "Код: 0x03");
}

static void test_refused_unknown_code(void) {
    const FnOpResult result = make_result(FnOpFlashMgm, FnOpStatusFnRefused, (FNError)0x3F);
    FnTextModel model;
    fn_format_result(&result, FnResultReasonOperation, &model);

    const FnTextSection* section = find_section(&model, FN_STR_RESULT_TITLE);
    TEST_CHECK(section_has_line(section, "Неизвестный код ответа ФН: 0x3F"));
    /* Для неизвестного кода отдельная строка «Код:» не нужна. */
    TEST_CHECK(!section_has_prefix(section, "Код:"));
}

static void test_no_response(void) {
    const FnOpResult result = make_result(FnOpFlashMgm, FnOpStatusNoResponse, FNOk);
    FnTextModel model;
    fn_format_result(&result, FnResultReasonOperation, &model);

    expect_line(&model, FN_STR_NO_RESPONSE);
    expect_line(&model, FN_STR_NO_RESPONSE_HINT);
}

static void test_bad_frame_and_busy(void) {
    FnTextModel model;

    const FnOpResult bad = make_result(FnOpReadInfo, FnOpStatusBadFrame, FNOk);
    fn_format_result(&bad, FnResultReasonOperation, &model);
    expect_line(&model, FN_STR_BAD_FRAME);

    const FnOpResult busy = make_result(FnOpReadInfo, FnOpStatusBusy, FNOk);
    fn_format_result(&busy, FnResultReasonOperation, &model);
    expect_line(&model, FN_STR_BUSY);
}

static void test_internal_error(void) {
    FnTextModel model;

    /* Чтение: порт не занят — «Не удалось занять порт 13/14». */
    const FnOpResult read = make_result(FnOpReadInfo, FnOpStatusInternalError, FNOk);
    fn_format_result(&read, FnResultReasonOperation, &model);
    expect_line(&model, FN_STR_PORT_BUSY);

    /* Очистка МГМ: защита от отправки в не-МГМ (FR-024). */
    const FnOpResult flash = make_result(FnOpFlashMgm, FnOpStatusInternalError, FNOk);
    fn_format_result(&flash, FnResultReasonOperation, &model);
    expect_line(&model, FN_STR_NOT_MGM);
}

static void test_cancelled_and_null(void) {
    FnTextModel model;

    const FnOpResult cancelled = make_result(FnOpReadInfo, FnOpStatusCancelled, FNOk);
    fn_format_result(&cancelled, FnResultReasonOperation, &model);
    TEST_CHECK_EQ_U16(model.section_count, 1);
    expect_line(&model, FN_STR_NO_DATA);

    /* NULL-результат и NULL-модель не должны падать. */
    fn_format_result(NULL, FnResultReasonOperation, &model);
    TEST_CHECK_EQ_U16(model.section_count, 1);

    fn_format_result(&cancelled, FnResultReasonOperation, NULL);
    TEST_CHECK(true);
}

int main(void) {
    test_flash_success();
    test_flash_not_mgm();
    test_refused_known_code();
    test_refused_unknown_code();
    test_no_response();
    test_bad_frame_and_busy();
    test_internal_error();
    test_cancelled_and_null();
    return TEST_SUMMARY();
}
