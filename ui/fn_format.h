/**
 * @file fn_format.h
 * @brief Сборка секций экранов из данных ФН: только данные, без GUI.
 *
 * Условие host-тестируемости: файл не включает gui/, furi и методы view;
 * результат — заполненный FnTextModel, который сцена передаёт во view.
 */
#pragma once

#include "../lib/fn/proto/fn_info.h"
#include "../lib/fn/proto/fn_life.h"
#include "fn_text_model.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Заполнить модель экрана «Информация об ФН» (US1, contracts/ui.md §8.1).
 *
 * Порядок секций фиксирован: «ФН (30h)», «Предупреждения» (только при
 * ненулевых флагах), «ФФД (3Ah)», «ПО ФН (33h)». Отсутствующие данные
 * выводятся как «нет данных»; блок «Последний документ» не выводится для
 * этапов 1 и 4.
 *
 * @param info  данные операции (не NULL)
 * @param model модель экрана; полностью перезаписывается
 */
void fn_format_info(const FnInfo* info, FnTextModel* model);

#ifdef __cplusplus
}
#endif
