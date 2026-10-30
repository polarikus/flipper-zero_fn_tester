// On-device тесты каталога кодов ответа (T027, R3).
#include "unit_tests/minunit.h"

#include <furi.h>

#include "core/fn_error.h"

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

MU_TEST(catalog_complete_test) {
    for(size_t i = 0; i < COUNT_OF(known_codes); ++i) {
        const FNError code = known_codes[i];
        mu_assert(fn_error_is_known(code), "код из заголовка должен быть в каталоге");
        const char* title = fn_error_title(code);
        mu_assert(title != NULL && title[0] != '\0', "у кода должен быть заголовок");
        mu_assert(fn_error_hint(code) != NULL, "подсказка никогда не NULL");
    }

    mu_assert_string_eq("Успешно", fn_error_title(FNOk));
    mu_assert_string_eq("", fn_error_hint(FNOk));
}

MU_TEST(unknown_code_test) {
    const FNError unknown[] = {(FNError)0x06, (FNError)0x3F, (FNError)0x7F};
    for(size_t i = 0; i < COUNT_OF(unknown); ++i) {
        mu_assert(!fn_error_is_known(unknown[i]), "код не должен быть известен");
        mu_assert_string_eq("Неизвестный код ответа ФН", fn_error_title(unknown[i]));
        mu_assert_string_eq("", fn_error_hint(unknown[i]));
    }
}

MU_TEST(non_null_test) {
    mu_assert(fn_error_title((FNError)0x55) != NULL, "title никогда не NULL");
    mu_assert(fn_error_hint((FNError)0x55) != NULL, "hint никогда не NULL");
}

MU_TEST_SUITE(fn_error_test_suite) {
    MU_RUN_TEST(catalog_complete_test);
    MU_RUN_TEST(unknown_code_test);
    MU_RUN_TEST(non_null_test);
}

int run_minunit_test_fn_error(void) {
    MU_RUN_SUITE(fn_error_test_suite);
    return MU_EXIT_CODE;
}
