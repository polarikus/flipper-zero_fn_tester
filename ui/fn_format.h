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

/**
 * Заполнить модель экрана «Информация о фазе жизни ФН» (US2,
 * contracts/ui.md §8.2).
 *
 * Порядок секций фиксирован: «Срок действия (32h)», «Остаток срока (3Bh)»,
 * «Ресурс памяти (3Dh)». Блоки 3Bh и 3Dh выводятся всегда — независимо от
 * версии ФФД и этапа (FR-011, FR-028); недоступный показатель — «нет данных».
 * Строка о маркировке выводится только при `has_marking`.
 *
 * @param life  данные операции (не NULL)
 * @param model модель экрана; полностью перезаписывается
 */
void fn_format_life(const FnLifeInfo* life, FnTextModel* model);

#ifdef __cplusplus
}
#endif
