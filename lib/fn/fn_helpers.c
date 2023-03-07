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

uint16_t calc_crc16(const uint8_t* data, size_t shift, size_t len){
    uint16_t crc = 0xffff;
    furi_check(len > shift);
    while(len-- && len >= shift){
        crc ^= *data++ << 8;
        for (int i = 0; i < 8; i++){
            crc = crc& 0x8000 ? (crc << 1) ^ 0x1021 : crc << 1;
        }
    }
    return crc;
};

bool check_crc(uint16_t crc, const uint8_t* data, size_t shift, size_t len)
{
    uint16_t crc_check = calc_crc16(data, shift, len);
    return crc_check == crc;
}

uint16_t two_bytes_to_LE(const uint8_t bytes[2]){
    return (bytes[0] << 8) | bytes[1];
}