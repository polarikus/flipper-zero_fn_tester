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
uint16_t two_uint8t_to_uint16t_LE(const uint8_t bytes[2]);

uint16_t two_uint8t_to_uint16t_BE(const uint8_t bytes[2]);

void uint16t_LE_to_uint8t_bytes(uint16_t uint16, uint8_t* bytes);

void uint16t_BE_to_uint8t_bytes(uint16_t uint16, uint8_t* bytes);

uint32_t byte_array_to_uint32t_LE(const uint8_t *byteArray);

void add_bytes_to_arr(
    uint8_t* target_array,
    size_t start, size_t target_arr_size,
    const uint8_t* dest_arr, size_t dest_arr_size);

/**
 * Считает CRC16-CCITT, формат LE, полином 0x1021
 * @param data Массив данных для вычисления CRC
 * @param start Стартовый байт в массиве данных
 * @param end Конечный байт в массиве данных
 * @return uint16_t LE | Полученное значение CRC 16
 */
uint16_t calc_crc16(const uint8_t* data, size_t start, size_t end);

/**
 * Проверить CRC16 с эталонным
 * @param crc CRC с которым нужно сверится
 * @param data Массив данных для вычисления CRC
 * @param shift Стартовый байт в массиве данных
 * @param len Конечный байт в массиве данных
 * @return bool | Если совпадают true
 */
bool check_crc(uint16_t crc, const uint8_t* data, size_t shift, size_t len);

void prepend_data_to_array(uint8_t **data, size_t old_size, const uint8_t *new_data, size_t new_size);