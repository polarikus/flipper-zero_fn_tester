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
 * Объединяет два uint8_t байта в uint16_t LE
 * @param bytes
 * @return uint16_t, формат LE
 */
uint16_t two_bytes_to_LE(const uint8_t bytes[2]);

/**
 * Считает CRC16-CCITT, формат LE, полином 0x1021
 * @param data Массив данных для вычисления CRC
 * @param shift Стартовый байт в массиве данных
 * @param len Конечный байт в массиве данных
 * @return uint16_t LE | Полученное значение CRC 16
 */
uint16_t calc_crc16(const uint8_t* data, size_t shift, size_t len);

/**
 * Проверить CRC16 с эталонным
 * @param crc CRC с которым нужно сверится
 * @param data Массив данных для вычисления CRC
 * @param shift Стартовый байт в массиве данных
 * @param len Конечный байт в массиве данных
 * @return bool | Если совпадают true
 */
bool check_crc(uint16_t crc, const uint8_t* data, size_t shift, size_t len);