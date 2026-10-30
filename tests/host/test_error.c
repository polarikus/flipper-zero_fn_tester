// Host-тесты каталога кодов ответа (T026, R3).
#include "fn_error.h"
#include "test_common.h"

#include <string.h>

static const FNError known_codes[] = {
    FNOk,
    FNErrorUnknownCMD,
    FNErrorWrongState,
    FNErrorFailure,
    FNErrorFailureKS,
    FNErrorCMDParamsNotMatchFNEndDate,
    FNErrorWrongDateTime,
    FNErrorNoRequestedData,
    FNErrorWrongParamValue,
    FNErrorWrongCommandFfd11,
    FNErrorForbiddenAttribute,
    FNErrorDuplicateData,
    FNErrorNoDataForAccounting,
    FNErrorTooManyPositions,
    FNErrorTlvTooBig,
    FNErrorNoTransport,
    FNErrorResourceExhausted,
    FNErrorResourceLimited,
    FNErrorSessionTooLong,
    FNErrorWrongTimeInterval,
    FNErrorWrongAttribute,
    FNErrorAttributeNotRegistered,
    FNErrorWrongAnswer,
    FNErrorKmUpdateNegative,
    FNErrorKmUpdateUnknown,
    FNErrorKmUpdateRepeat,
    FNErrorMarkingForbidden,
    FNErrorMarkingSequence,
    FNErrorMarkingBlocked,
    FNErrorKmTableOverflow,
    FNErrorKmKeysExpired,
    FNErrorTlvMissingAttribute,
    FNErrorKmNotChecked,
};

static size_t utf8_length(const char* text) {
    size_t count = 0;
    for(const unsigned char* p = (const unsigned char*)text; *p; ++p) {
        if((*p & 0xC0) != 0x80) ++count;
    }
    return count;
}

static void test_catalog(void) {
    for(size_t i = 0; i < sizeof(known_codes) / sizeof(known_codes[0]); ++i) {
        const FNError code = known_codes[i];
        TEST_CHECK(fn_error_is_known(code));
        const char* title = fn_error_title(code);
        TEST_CHECK(title != NULL);
        TEST_CHECK(title && strlen(title) > 0 && utf8_length(title) <= 40);
        TEST_CHECK(fn_error_hint(code) != NULL);
    }

    TEST_CHECK(strcmp(fn_error_title(FNOk), "Успешно") == 0);
}

static void test_unknown(void) {
    const FNError unknown[] = {(FNError)0x06, (FNError)0x3F, (FNError)0x7F};
    for(size_t i = 0; i < sizeof(unknown) / sizeof(unknown[0]); ++i) {
        TEST_CHECK(!fn_error_is_known(unknown[i]));
        TEST_CHECK(strcmp(fn_error_title(unknown[i]), "Неизвестный код ответа ФН") == 0);
        TEST_CHECK(fn_error_hint(unknown[i]) != NULL);
        TEST_CHECK(strcmp(fn_error_hint(unknown[i]), "") == 0);
    }
}

int main(void) {
    test_catalog();
    test_unknown();
    return TEST_SUMMARY();
}
