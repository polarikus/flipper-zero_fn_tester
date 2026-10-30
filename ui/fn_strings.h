/**
 * @file fn_strings.h
 * @brief Единый каталог пользовательских строк (русский интерфейс).
 *
 * Файл не включает gui/, furi и fn_view_*: его проверяет host-тест
 * tests/host/test_strings.c и check_layers.sh.
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* Главное меню (FR-001). */
#define FN_STR_MENU_FN_INFO   "Информация об ФН"
#define FN_STR_MENU_LIFE_INFO "Информация о фазе жизни ФН"
#define FN_STR_MENU_FLASH_MGM "Очистка МГМ"
#define FN_STR_MENU_WIRING    "Распиновка"
#define FN_STR_MENU_ABOUT     "О приложении"

/* Заголовки экранов. */
#define FN_STR_APP_TITLE     "Тестер ФН"
#define FN_STR_INFO_TITLE    "Информация об ФН"
#define FN_STR_LIFE_TITLE    "Фаза жизни ФН"
#define FN_STR_RESULT_TITLE  "Результат"
#define FN_STR_WIRING_TITLE  "Распиновка"
#define FN_STR_ABOUT_TITLE   "О приложении"
#define FN_STR_CONFIRM_TITLE "Очистка МГМ"

/* Подписи полей. */
#define FN_STR_SN             "Серийный номер"
#define FN_STR_STAGE          "Этап"
#define FN_STR_SESSION        "Смена"
#define FN_STR_SESSION_OPEN   "открыта"
#define FN_STR_SESSION_CLOSED "закрыта"
#define FN_STR_DOC            "Документ для ОФД"
#define FN_STR_FFD            "ФФД"
#define FN_STR_FW             "Версия ПО"
#define FN_STR_MGM            "МГМ"
#define FN_STR_SHIFT          "Номер смены"
#define FN_STR_RECEIPT        "Номер чека"
#define FN_STR_LAST_DOC       "Последний документ"
#define FN_STR_WARNINGS       "Предупреждения"
#define FN_STR_OTHER_WARNINGS "Прочие предупреждения: 0x%02X"

/* Значения. */
#define FN_STR_YES               "да"
#define FN_STR_NO                "нет"
#define FN_STR_NO_DATA           "нет данных"
#define FN_STR_DOC_NONE          "Документа нет"
#define FN_STR_DOC_DATA_RECEIVED "данные получены"
#define FN_STR_DOC_NO_DATA       "данные не получены"
#define FN_STR_MGM_YES           "опытный образец"
#define FN_STR_MGM_NO            "боевой"

/* Ожидание. */
#define FN_STR_WAIT_DETECT      "Поиск ФН"
#define FN_STR_WAIT_DETECT_HINT "Проверьте подключение"
#define FN_STR_WAIT_READ        "Ожидание ответа ФН"
#define FN_STR_WAIT_READ_HINT   "Назад - вернуться в меню"
#define FN_STR_WAIT_FLASH       "Очистка МГМ"
#define FN_STR_WAIT_FLASH_HINT  "Может занять до 30 секунд, не прерывайте"

/* Результаты и ошибки. */
#define FN_STR_RESULT_OK        "Очистка МГМ выполнена успешно"
#define FN_STR_NO_RESPONSE      "ФН не отвечает"
#define FN_STR_NO_RESPONSE_HINT "Проверьте подключение и повторите"
#define FN_STR_BAD_FRAME        "Поврежденный ответ ФН"
#define FN_STR_NOT_MGM          "Очистка недоступна: ФН не является МГМ"
#define FN_STR_UNKNOWN_CODE_FMT "Неизвестный код ответа ФН: 0x%02X"
#define FN_STR_ERROR_CODE_FMT   "Код: 0x%02X"
#define FN_STR_BUSY             "Дождитесь завершения операции"
#define FN_STR_PORT_BUSY        "Не удалось занять порт 13/14"

/* Подтверждение. */
#define FN_STR_CONFIRM_BODY   "Операция необратима. ФН будет сброшен в начальное состояние."
#define FN_STR_CONFIRM_ACCEPT "Очистить"
#define FN_STR_CONFIRM_CANCEL "Отмена"

/* О приложении. */
#define FN_STR_ABOUT_APP         "Тестер фискальных накопителей"
#define FN_STR_ABOUT_AUTHOR      "Автор: Игорь Данилов"
#define FN_STR_ABOUT_VERSION_FMT "Версия: %s"
#define FN_STR_ABOUT_URL         "github.com/polarikus/flipper-zero_fn_tester"
#define FN_STR_ABOUT_DESCRIPTION \
    "Показывает сведения об ФН, фазу жизни, предупреждения и позволяет сбросить МГМ."

#ifdef __cplusplus
}
#endif
