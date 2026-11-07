// Host-тесты каталога пользовательских строк (T043).
#include "fn_strings.h"
#include "fn_ui.h"
#include "test_common.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static void test_catalog(void) {
    for(int lang = FnLangEn; lang <= FnLangRu; ++lang) {
        for(int id = 0; id < FnStrCount; ++id) {
            const char* text = fn_str((FnLang)lang, (FnStrId)id);
            if(!text || text[0] == '\0') {
                fprintf(stderr, "FAIL пустая строка id=%d lang=%d\n", id, lang);
                TEST_CHECK(false);
                continue;
            }
            TEST_CHECK(strlen(text) < 128);
        }
    }
}

static void test_languages(void) {
    TEST_CHECK(strcmp(fn_str(FnLangEn, FnStrMenuFnInfo), "FN information") == 0);
    TEST_CHECK(strcmp(fn_str(FnLangRu, FnStrMenuFnInfo), "Информация об ФН") == 0);
    TEST_CHECK(strcmp(fn_str(FnLangEn, FnStrAppTitle), "FN Tester") == 0);
    TEST_CHECK(strcmp(fn_str(FnLangRu, FnStrAppTitle), "Тестер ФН") == 0);
    TEST_CHECK(strcmp(fn_str(FnLangEn, FnStrNoData), "no data") == 0);
    TEST_CHECK(strcmp(fn_str(FnLangRu, FnStrNoData), "нет данных") == 0);
}

static void test_format(void) {
    char buffer[64];
    snprintf(buffer, sizeof(buffer), fn_str(FnLangRu, FnStrUnknownCodeFmt), 0x3F);
    TEST_CHECK(strcmp(buffer, "Неизвестный код ответа ФН: 0x3F") == 0);

    snprintf(buffer, sizeof(buffer), fn_str(FnLangEn, FnStrUnknownCodeFmt), 0x3F);
    TEST_CHECK(strcmp(buffer, "Unknown FN response code: 0x3F") == 0);

    snprintf(buffer, sizeof(buffer), fn_str(FnLangRu, FnStrErrorCodeFmt), 0x03);
    TEST_CHECK(strcmp(buffer, "Код: 0x03") == 0);

    snprintf(buffer, sizeof(buffer), fn_str(FnLangEn, FnStrErrorCodeFmt), 0x03);
    TEST_CHECK(strcmp(buffer, "Code: 0x03") == 0);
}

/**
 * Строки экрана «О приложении» (FR-017) обязаны целиком помещаться в строку
 * модели: vsnprintf обрезает по байтам и может разорвать кириллический символ.
 */
static void test_about_lines(void) {
    static const FnStrId lines[] = {
        FnStrAboutApp,
        FnStrAboutAuthor,
        FnStrAboutUrl1,
        FnStrAboutUrl2,
        FnStrAboutDescription1,
        FnStrAboutDescription2,
        FnStrAboutDescription3,
        FnStrAboutDescription4,
    };

    for(int lang = FnLangEn; lang <= FnLangRu; ++lang) {
        for(size_t i = 0; i < sizeof(lines) / sizeof(lines[0]); ++i) {
            const char* text = fn_str((FnLang)lang, lines[i]);
            if(strlen(text) >= FN_TEXT_LINE_LEN) {
                fprintf(stderr, "FAIL строка «О приложении» длиннее модели: %s\n", text);
            }
            TEST_CHECK(strlen(text) < FN_TEXT_LINE_LEN);
        }
    }

    char buffer[64];
    snprintf(buffer, sizeof(buffer), fn_str(FnLangRu, FnStrAboutVersionFmt), "0.6");
    TEST_CHECK(strcmp(buffer, "Версия: 0.6") == 0);

    snprintf(buffer, sizeof(buffer), fn_str(FnLangEn, FnStrAboutVersionFmt), "0.6");
    TEST_CHECK(strcmp(buffer, "Version: 0.6") == 0);
}

int main(void) {
    test_catalog();
    test_languages();
    test_format();
    test_about_lines();
    return TEST_SUMMARY();
}
