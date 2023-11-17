//
// Created by Игорь Данилов on 16.09.2023.
//

#pragma once

#include "byte_array.h"

struct ByteArray {
    size_t max_size;
    size_t size;
    uint8_t* array;
};