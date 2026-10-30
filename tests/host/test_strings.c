// Host-тесты каталога пользовательских строк (T043).
#include "fn_strings.h"
#include "test_common.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    const char* name;
    const char* text;
    bool allow_latin;
} StringEntry;

static const StringEntry strings[] = {
    {"MENU_FN_INFO", FN_STR_MENU_FN_INFO, false},
    {"MENU_LIFE_INFO", FN_STR_MENU_LIFE_INFO, false},
    {"MENU_FLASH_MGM", FN_STR_MENU_FLASH_MGM, false},
    {"MENU_WIRING", FN_STR_MENU_WIRING, false},
    {"MENU_ABOUT", FN_STR_MENU_ABOUT, false},
    {"APP_TITLE", FN_STR_APP_TITLE, false},
    {"INFO_TITLE", FN_STR_INFO_TITLE, false},
    {"LIFE_TITLE", FN_STR_LIFE_TITLE, false},
    {"RESULT_TITLE", FN_STR_RESULT_TITLE, false},
    {"WIRING_TITLE", FN_STR_WIRING_TITLE, false},
    {"ABOUT_TITLE", FN_STR_ABOUT_TITLE, false},
    {"CONFIRM_TITLE", FN_STR_CONFIRM_TITLE, false},
    // В заголовках секций — техническое обозначение команды протокола (30h и т. п.).
    {"SECTION_FN_STATUS", FN_STR_SECTION_FN_STATUS, true},
    {"SECTION_FFD", FN_STR_SECTION_FFD, true},
    {"SECTION_FW", FN_STR_SECTION_FW, true},
    {"SECTION_LIFE_DATE", FN_STR_SECTION_LIFE_DATE, true},
    {"SECTION_LIFE_DAYS", FN_STR_SECTION_LIFE_DAYS, true},
    {"SECTION_MEMORY", FN_STR_SECTION_MEMORY, true},
    {"SN", FN_STR_SN, false},
    {"STAGE", FN_STR_STAGE, false},
    {"SESSION", FN_STR_SESSION, false},
    {"SESSION_OPEN", FN_STR_SESSION_OPEN, false},
    {"SESSION_CLOSED", FN_STR_SESSION_CLOSED, false},
    {"DOC", FN_STR_DOC, false},
    {"DOC_HEADER", FN_STR_DOC_HEADER, false},
    {"FFD", FN_STR_FFD, false},
    {"FW", FN_STR_FW, false},
    {"MGM", FN_STR_MGM, false},
    {"SHIFT", FN_STR_SHIFT, false},
    {"RECEIPT", FN_STR_RECEIPT, false},
    {"LAST_DOC", FN_STR_LAST_DOC, false},
    {"LAST_DOC_DATE", FN_STR_LAST_DOC_DATE, false},
    {"FFD_CURRENT", FN_STR_FFD_CURRENT, false},
    {"FFD_MAX", FN_STR_FFD_MAX, false},
    {"WARNINGS", FN_STR_WARNINGS, false},
    // Спецификатор формата "0x%02X" содержит латинские буквы — это ожидаемо.
    {"OTHER_WARNINGS", FN_STR_OTHER_WARNINGS, true},
    {"LIFE_END_DATE", FN_STR_LIFE_END_DATE, false},
    {"LIFE_NOT_ACTIVATED", FN_STR_LIFE_NOT_ACTIVATED, false},
    {"REG_DONE", FN_STR_REG_DONE, false},
    {"REG_LEFT", FN_STR_REG_LEFT, false},
    {"MEM_FIVE_YEAR", FN_STR_MEM_FIVE_YEAR, false},
    {"MEM_THIRTY_DAY", FN_STR_MEM_THIRTY_DAY, false},
    {"DOCUMENTS", FN_STR_DOCUMENTS, false},
    // Единица измерения «КБ» — кириллица, латиницы нет намеренно.
    {"KB", FN_STR_KB, false},
    {"MARKING_FMT", FN_STR_MARKING_FMT, true},
    {"YES", FN_STR_YES, false},
    {"NO", FN_STR_NO, false},
    {"NO_DATA", FN_STR_NO_DATA, false},
    {"DOC_NONE", FN_STR_DOC_NONE, false},
    {"DOC_DATA_RECEIVED", FN_STR_DOC_DATA_RECEIVED, false},
    {"DOC_NO_DATA", FN_STR_DOC_NO_DATA, false},
    {"MGM_YES", FN_STR_MGM_YES, false},
    {"MGM_NO", FN_STR_MGM_NO, false},
    {"INDENT_FMT", FN_STR_INDENT_FMT, true},
    {"DOC_UNKNOWN_FMT", FN_STR_DOC_UNKNOWN_FMT, true},
    {"DATE_FMT", FN_STR_DATE_FMT, true},
    {"SHORT_DATE_FMT", FN_STR_SHORT_DATE_FMT, true},
    {"DAYS_FMT", FN_STR_DAYS_FMT, true},
    {"DAY_ONE", FN_STR_DAY_ONE, false},
    {"DAY_FEW", FN_STR_DAY_FEW, false},
    {"DAY_MANY", FN_STR_DAY_MANY, false},
    {"FIVE_YEAR_FMT", FN_STR_FIVE_YEAR_FMT, true},
    {"THIRTY_DAY_FMT", FN_STR_THIRTY_DAY_FMT, true},
    {"WAIT_DETECT", FN_STR_WAIT_DETECT, false},
    {"WAIT_DETECT_HINT", FN_STR_WAIT_DETECT_HINT, false},
    {"WAIT_READ", FN_STR_WAIT_READ, false},
    {"WAIT_READ_HINT", FN_STR_WAIT_READ_HINT, false},
    {"WAIT_FLASH", FN_STR_WAIT_FLASH, false},
    {"WAIT_FLASH_HINT", FN_STR_WAIT_FLASH_HINT, false},
    {"RESULT_OK", FN_STR_RESULT_OK, false},
    {"NO_RESPONSE", FN_STR_NO_RESPONSE, false},
    {"NO_RESPONSE_HINT", FN_STR_NO_RESPONSE_HINT, false},
    {"BAD_FRAME", FN_STR_BAD_FRAME, false},
    {"NOT_MGM", FN_STR_NOT_MGM, false},
    {"UNKNOWN_CODE_FMT", FN_STR_UNKNOWN_CODE_FMT, true},
    {"ERROR_CODE_FMT", FN_STR_ERROR_CODE_FMT, true},
    {"BUSY", FN_STR_BUSY, false},
    {"PORT_BUSY", FN_STR_PORT_BUSY, false},
    {"CONFIRM_BODY", FN_STR_CONFIRM_BODY, false},
    {"CONFIRM_ACCEPT", FN_STR_CONFIRM_ACCEPT, false},
    {"CONFIRM_CANCEL", FN_STR_CONFIRM_CANCEL, false},
    {"ABOUT_APP", FN_STR_ABOUT_APP, false},
    {"ABOUT_AUTHOR", FN_STR_ABOUT_AUTHOR, false},
    {"ABOUT_URL", FN_STR_ABOUT_URL, true},
    {"ABOUT_DESCRIPTION", FN_STR_ABOUT_DESCRIPTION, false},
};

static bool has_latin(const char* text) {
    for(const char* p = text; *p; ++p) {
        const unsigned char c = (unsigned char)*p;
        if(c < 0x80 && isalpha(c)) return true;
    }
    return false;
}

static void test_strings(void) {
    for(size_t i = 0; i < sizeof(strings) / sizeof(strings[0]); ++i) {
        const StringEntry* entry = &strings[i];
        TEST_CHECK(entry->text != NULL);
        if(!entry->text || entry->text[0] == '\0') {
            fprintf(stderr, "FAIL пустая строка %s\n", entry->name);
            continue;
        }
        if(!entry->allow_latin && has_latin(entry->text)) {
            fprintf(stderr, "FAIL латиница в %s: %s\n", entry->name, entry->text);
            TEST_CHECK(false);
        }
    }
}

static void test_format(void) {
    char buffer[64];
    snprintf(buffer, sizeof(buffer), FN_STR_UNKNOWN_CODE_FMT, 0x3F);
    TEST_CHECK(strcmp(buffer, "Неизвестный код ответа ФН: 0x3F") == 0);

    snprintf(buffer, sizeof(buffer), FN_STR_ERROR_CODE_FMT, 0x03);
    TEST_CHECK(strcmp(buffer, "Код: 0x03") == 0);
}

int main(void) {
    test_strings();
    test_format();
    return TEST_SUMMARY();
}
