/**
 * @file fn_ui.h
 * @brief Константы верстки интерфейса 128x64 и стили текста.
 */
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FN_UI_MARGIN_X    2
#define FN_UI_HEADER_H    12
#define FN_UI_CONTENT_Y   13
#define FN_UI_CONTENT_H   50
#define FN_UI_LINE_H      9
#define FN_UI_SMALL_H     7
#define FN_UI_SECTION_GAP 3
#define FN_UI_SCROLLBAR_W 3
#define FN_UI_TEXT_W      120

#define FN_TEXT_MAX_SECTIONS 8
#define FN_TEXT_MAX_LINES    12
#define FN_TEXT_LINE_LEN     40

/** Стиль строки в модели текстового экрана. */
typedef enum {
    FnTextStyleNormal = 0,
    FnTextStyleHeader,
    FnTextStyleWarning,
} FnTextStyle;

#ifdef __cplusplus
}
#endif
