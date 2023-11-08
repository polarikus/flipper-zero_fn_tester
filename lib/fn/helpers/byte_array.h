//
// Created by Игорь Данилов on 16.09.2023.
//

#pragma once

#include "furi.h"

/**
 * @brief Byte Array failure constant.
 */
#define BYTE_ARRAY_FAILURE ((size_t)-1)

typedef struct ByteArray ByteArray;

ByteArray* byte_array_alloc(size_t max_size);

void byte_array_free(ByteArray* byte_array);

size_t byte_array_push(ByteArray* byte_array, const void *  data, size_t data_size);

void byte_array_read_int_LE(ByteArray* byte_array, void* dest, size_t size);

void byte_array_read_int_BE(ByteArray* byte_array,void* dest, size_t size);

void byte_array_get_int_LE(ByteArray* byte_array, void* dest, size_t size);

void byte_array_get_int_BE(ByteArray* byte_array, void* dest, size_t size);



