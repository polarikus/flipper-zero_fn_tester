//
// Created by Игорь Данилов on 28.02.2023.
//

#pragma once
#include <furi.h>

/**
 * Переводит bool значение в человеческий язык
 * @param b Булево значение
 * @return строка "yes" или "no"
 */
const char* bool_to_hum(bool b);

/**
 * Объединяет два uint8_t байта в uint16_t, биты LE
 * @param bytes
 * @return uint16_t
 */
uint16_t two_bytes_to_LE(const uint8_t bytes[2]);