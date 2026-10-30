// Host-тесты сборки экрана «Информация об ФН» (T054, US1).
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

static bool model_has_line(const FnTextModel* model, const char* text) {
    for(uint8_t s = 0; s < model->section_count; ++s) {
        if(section_has_line(&model->sections[s], text)) return true;
    }
    return false;
}

/** Есть ли строка, начинающаяся с prefix (например, подпись поля). */
static bool model_has_prefix(const FnTextModel* model, const char* prefix) {
    const size_t prefix_len = strlen(prefix);
    for(uint8_t s = 0; s < model->section_count; ++s) {
        const FnTextSection* section = &model->sections[s];
        for(uint8_t l = 0; l < section->line_count; ++l) {
            if(strncmp(section->lines[l].text, prefix, prefix_len) == 0) return true;
        }
    }
    return false;
}

static bool title_is(const FnTextSection* section, const char* title) {
    return section->title != NULL && strcmp(section->title, title) == 0;
}

static void expect_line(const FnTextModel* model, const char* title, const char* text) {
    const FnTextSection* section = find_section(model, title);
    if(!section_has_line(section, text)) {
        fprintf(stderr, "FAIL нет строки в секции «%s»: %s\n", title, text);
        TEST_CHECK(false);
    } else {
        TEST_CHECK(true);
    }
}

static FnInfo make_reference_info(void) {
    FnInfo info;
    memset(&info, 0, sizeof(info));

    info.status.stage = FnStageRegistered;
    info.status.stage_raw = 0x03;
    info.status.current_doc = FnDocReceipt;
    info.status.current_doc_raw = 0x04;
    info.status.doc_data_received = true;
    info.status.session_open = true;
    info.status.warn_flags = FnWarnResourceSoon;
    info.status.has_serial = true;
    strncpy(info.status.serial, "9999078900001234", sizeof(info.status.serial) - 1);
    info.status.last_doc_dt.date.year = 26;
    info.status.last_doc_dt.date.month = 10;
    info.status.last_doc_dt.date.day = 29;
    info.status.last_doc_dt.hour = 12;
    info.status.last_doc_dt.minute = 30;
    info.status.last_doc_number = 42;
    info.status.has_last_doc = true;
    info.status.has_last_doc_number = true;

    info.ffd.current = FnFfd1_2;
    info.ffd.max = FnFfd1_2;
    info.has_ffd = true;

    strncpy(info.fw.version, "fn-1.2", sizeof(info.fw.version) - 1);
    info.fw.mode_raw = 0;
    info.fw.is_mgm = true;
    info.has_fw = true;

    info.session.session_open = true;
    info.session.shift_number = 300;
    info.session.receipt_number = 5;
    info.has_session = true;

    info.response_has_warning = true;
    return info;
}

static void test_reference(void) {
    const FnInfo info = make_reference_info();
    FnTextModel model;
    fn_format_info(&info, &model);

    TEST_CHECK_EQ_U16(model.section_count, 4);
    TEST_CHECK(title_is(&model.sections[0], FN_STR_SECTION_FN_STATUS));
    TEST_CHECK(title_is(&model.sections[1], FN_STR_WARNINGS));
    TEST_CHECK(title_is(&model.sections[2], FN_STR_SECTION_FFD));
    TEST_CHECK(title_is(&model.sections[3], FN_STR_SECTION_FW));

    const char* fn_title = FN_STR_SECTION_FN_STATUS;
    expect_line(&model, fn_title, "Серийный номер: 9999078900001234");
    expect_line(&model, fn_title, "Этап: Зарегистрирован");
    expect_line(&model, fn_title, "Смена: открыта");
    expect_line(&model, fn_title, "Документ для ОФД:");
    expect_line(&model, fn_title, "  Кассовый чек");
    expect_line(&model, fn_title, "  данные получены");
    expect_line(&model, fn_title, "Номер смены: 300");
    expect_line(&model, fn_title, "Номер чека: 5");
    expect_line(&model, fn_title, "Последний документ: 42");
    expect_line(&model, fn_title, "Дата документа: 26.10.29 12:30");

    expect_line(&model, FN_STR_WARNINGS, "Исчерпание ресурса ФН (до 30 дней)");

    expect_line(&model, FN_STR_SECTION_FFD, "Текущая: 1.2");
    expect_line(&model, FN_STR_SECTION_FFD, "Максимальная: 1.2");

    expect_line(&model, FN_STR_SECTION_FW, "Версия ПО: fn-1.2");
    expect_line(&model, FN_STR_SECTION_FW, "МГМ: опытный образец");
}

static void test_stage1_and_stage4_without_last_doc(void) {
    FnInfo info = make_reference_info();
    info.status.stage = FnStageReadyRegistration;
    info.status.current_doc = FnDocNone;
    info.status.current_doc_raw = 0x00;

    FnTextModel model;
    fn_format_info(&info, &model);

    TEST_CHECK(!model_has_prefix(&model, "Последний документ"));
    TEST_CHECK(model_has_line(&model, "Документ для ОФД:"));
    TEST_CHECK(model_has_line(&model, "  Документа нет"));
    // Для «документа нет» признак получения данных не выводится.
    TEST_CHECK(!model_has_line(&model, "  данные получены"));
    TEST_CHECK(!model_has_line(&model, "  данные не получены"));

    info.status.stage = FnStageArchiveClosedFinal;
    fn_format_info(&info, &model);
    TEST_CHECK(!model_has_prefix(&model, "Последний документ"));
    TEST_CHECK(!model_has_line(&model, "Дата документа: нет данных"));
}

static void test_partial_data(void) {
    FnInfo info = make_reference_info();
    info.has_ffd = false;
    info.has_fw = false;
    info.has_session = false;
    info.status.has_serial = false;
    info.status.has_last_doc = false;
    info.status.has_last_doc_number = false;
    info.status.warn_flags = 0;

    FnTextModel model;
    fn_format_info(&info, &model);

    // Блок предупреждений при нулевых флагах не выводится.
    TEST_CHECK_EQ_U16(model.section_count, 3);
    TEST_CHECK(find_section(&model, FN_STR_WARNINGS) == NULL);

    const char* fn_title = FN_STR_SECTION_FN_STATUS;
    expect_line(&model, fn_title, "Серийный номер: нет данных");
    expect_line(&model, fn_title, "Номер смены: нет данных");
    expect_line(&model, fn_title, "Номер чека: нет данных");
    expect_line(&model, fn_title, "Последний документ: нет данных");
    expect_line(&model, fn_title, "Дата документа: нет данных");
    // Остальные секции сохраняются.
    expect_line(&model, fn_title, "Этап: Зарегистрирован");
    expect_line(&model, FN_STR_SECTION_FFD, "нет данных");
    expect_line(&model, FN_STR_SECTION_FW, "нет данных");
}

static void test_unknown_doc_and_warnings(void) {
    FnInfo info = make_reference_info();
    info.status.current_doc = FnDocUnknown;
    info.status.current_doc_raw = 0x99;
    info.status.doc_data_received = false;
    info.status.warn_flags = FnWarnReplaceSoon | FnWarnReserved;

    FnTextModel model;
    fn_format_info(&info, &model);

    expect_line(&model, FN_STR_SECTION_FN_STATUS, "  Неизвестный документ (0x99)");
    expect_line(&model, FN_STR_SECTION_FN_STATUS, "  данные не получены");

    expect_line(&model, FN_STR_WARNINGS, "Срочная замена ФН (до 3 дней)");
    expect_line(&model, FN_STR_WARNINGS, "Прочие предупреждения: 0x80");

    // Только неизвестный бит: известных строк нет, блок остаётся.
    info.status.warn_flags = FnWarnReserved;
    fn_format_info(&info, &model);
    const FnTextSection* warnings = find_section(&model, FN_STR_WARNINGS);
    TEST_CHECK(warnings != NULL);
    TEST_CHECK_EQ_U16(warnings->line_count, 1);
    expect_line(&model, FN_STR_WARNINGS, "Прочие предупреждения: 0x80");
}

static void test_mgm_and_ffd_values(void) {
    FnInfo info = make_reference_info();
    info.fw.is_mgm = false;
    info.fw.mode_raw = 1;
    info.ffd.current = FnFfdNone;
    info.ffd.max = FnFfd1_2;

    FnTextModel model;
    fn_format_info(&info, &model);

    expect_line(&model, FN_STR_SECTION_FFD, "Текущая: Не активизирован");
    expect_line(&model, FN_STR_SECTION_FFD, "Максимальная: 1.2");
    expect_line(&model, FN_STR_SECTION_FW, "МГМ: боевой");
}

static void test_null_arguments(void) {
    FnTextModel model;
    memset(&model, 0, sizeof(model));
    fn_format_info(NULL, &model);
    TEST_CHECK_EQ_U16(model.section_count, 0);

    const FnInfo info = make_reference_info();
    fn_format_info(&info, NULL);
    TEST_CHECK(true);
}

int main(void) {
    test_reference();
    test_stage1_and_stage4_without_last_doc();
    test_partial_data();
    test_unknown_doc_and_warnings();
    test_mgm_and_ffd_values();
    test_null_arguments();
    return TEST_SUMMARY();
}
