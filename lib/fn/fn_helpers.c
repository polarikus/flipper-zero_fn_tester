//
// Created by Игорь Данилов on 28.02.2023.
//
#include "fn_helpers.h"
const char* bool_to_hum(bool b){
    if(b){
        return "yes";
    }
    return "no";
}

uint16_t two_bytes_to_LE(const uint8_t bytes[2]){
    return (bytes[0] << 8) | bytes[1];
}