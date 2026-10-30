// Host-тесты сборки экрана «Информация о фазе жизни ФН» (T059, US2).
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

static FnLifeInfo make_reference_life(void) {
    FnLifeInfo life;
    memset(&life, 0, sizeof(life));

    life.date.end_date.year = 26;
    life.date.end_date.month = 12;
    life.date.end_date.day = 31;
    life.date.reg_done = 5;
    life.date.reg_remaining = 8;
    life.has_date = true;

    life.days.days = 360;
    life.days.has_days = true;
    life.has_days = true;

    life.memory.five_year_documents = 5000;
    life.memory.thirty_day_kb = 8192;
    life.memory.marking_percent = 7;
    life.memory.has_marking = true;
    life.has_memory = true;

    return life;
}

static void test_reference(void) {
    const FnLifeInfo life = make_reference_life();
    FnTextModel model;
    fn_format_life(&life, &model);

    TEST_CHECK_EQ_U16(model.section_count, 3);
    TEST_CHECK(title_is(&model.sections[0], FN_STR_SECTION_LIFE_DATE));
    TEST_CHECK(title_is(&model.sections[1], FN_STR_SECTION_LIFE_DAYS));
    TEST_CHECK(title_is(&model.sections[2], FN_STR_SECTION_MEMORY));

    expect_line(&model, FN_STR_SECTION_LIFE_DATE, "Дата окончания: 31.12.26");
    expect_line(&model, FN_STR_SECTION_LIFE_DATE, "Выполнено регистраций: 5");
    expect_line(&model, FN_STR_SECTION_LIFE_DATE, "Осталось регистраций: 8");

    expect_line(&model, FN_STR_SECTION_LIFE_DAYS, "360 дней");

    expect_line(&model, FN_STR_SECTION_MEMORY, "5-летнее хранение: 5000 документов");
    expect_line(&model, FN_STR_SECTION_MEMORY, "30-дневное хранение: 8192 КБ");
    expect_line(&model, FN_STR_SECTION_MEMORY, "Область уведомлений о маркировке: 7 %");
}

static void test_days_missing(void) {
    FnLifeInfo life = make_reference_life();
    life.days.days = 0;
    life.days.has_days = false;
    life.has_days = false;

    FnTextModel model;
    fn_format_life(&life, &model);

    // Нулевой остаток — «нет данных», а не скрытие блока (FR-011).
    const FnTextSection* days = find_section(&model, FN_STR_SECTION_LIFE_DAYS);
    TEST_CHECK(days != NULL);
    TEST_CHECK(section_has_line(days, FN_STR_NO_DATA));
    TEST_CHECK_EQ_U16(days->line_count, 1);

    // Остальные блоки остаются на экране.
    TEST_CHECK(find_section(&model, FN_STR_SECTION_LIFE_DATE) != NULL);
    TEST_CHECK(find_section(&model, FN_STR_SECTION_MEMORY) != NULL);
    expect_line(&model, FN_STR_SECTION_LIFE_DATE, "Дата окончания: 31.12.26");
}

static void test_days_plural(void) {
    struct {
        uint16_t days;
        const char* text;
    } cases[] = {
        {1, "1 день"},
        {2, "2 дня"},
        {4, "4 дня"},
        {5, "5 дней"},
        {11, "11 дней"},
        {21, "21 день"},
        {112, "112 дней"},
    };

    for(size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        FnLifeInfo life = make_reference_life();
        life.days.days = cases[i].days;

        FnTextModel model;
        fn_format_life(&life, &model);
        expect_line(&model, FN_STR_SECTION_LIFE_DAYS, cases[i].text);
    }
}

static void test_without_marking(void) {
    FnLifeInfo life = make_reference_life();
    life.memory.has_marking = false;
    life.memory.marking_percent = 0;

    FnTextModel model;
    fn_format_life(&life, &model);

    // Строки о маркировке нет вовсе; «0 %» не подставляется (FR-028).
    TEST_CHECK(!model_has_prefix(&model, "Область уведомлений"));
    TEST_CHECK(!model_has_line(&model, "Область уведомлений о маркировке: 0 %"));
    expect_line(&model, FN_STR_SECTION_MEMORY, "5-летнее хранение: 5000 документов");
}

static void test_memory_refused(void) {
    FnLifeInfo life = make_reference_life();
    life.has_memory = false;
    memset(&life.memory, 0, sizeof(life.memory));

    FnTextModel model;
    fn_format_life(&life, &model);

    // Отказ 3Dh не скрывает секции 32h/3Bh (FR-011).
    TEST_CHECK_EQ_U16(model.section_count, 3);
    const FnTextSection* memory = find_section(&model, FN_STR_SECTION_MEMORY);
    TEST_CHECK(memory != NULL);
    TEST_CHECK(section_has_line(memory, FN_STR_NO_DATA));
    TEST_CHECK(!model_has_prefix(&model, "5-летнее хранение"));

    expect_line(&model, FN_STR_SECTION_LIFE_DATE, "Дата окончания: 31.12.26");
    expect_line(&model, FN_STR_SECTION_LIFE_DAYS, "360 дней");
}

static void test_not_activated_and_no_date(void) {
    FnLifeInfo life = make_reference_life();
    life.date.end_date.year = 0;
    life.date.end_date.month = 0;
    life.date.end_date.day = 0;

    FnTextModel model;
    fn_format_life(&life, &model);
    expect_line(&model, FN_STR_SECTION_LIFE_DATE, "Дата окончания: не активизирован");

    life.has_date = false;
    fn_format_life(&life, &model);
    expect_line(&model, FN_STR_SECTION_LIFE_DATE, "Дата окончания: нет данных");
    expect_line(&model, FN_STR_SECTION_LIFE_DATE, "Выполнено регистраций: нет данных");
    expect_line(&model, FN_STR_SECTION_LIFE_DATE, "Осталось регистраций: нет данных");
}

static void test_null_arguments(void) {
    FnTextModel model;
    memset(&model, 0, sizeof(model));
    fn_format_life(NULL, &model);
    TEST_CHECK_EQ_U16(model.section_count, 0);

    const FnLifeInfo life = make_reference_life();
    fn_format_life(&life, NULL);
    TEST_CHECK(true);
}

int main(void) {
    test_reference();
    test_days_missing();
    test_days_plural();
    test_without_marking();
    test_memory_refused();
    test_not_activated_and_no_date();
    test_null_arguments();
    return TEST_SUMMARY();
}
