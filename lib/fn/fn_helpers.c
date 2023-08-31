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

uint16_t calc_crc16(const uint8_t* data, size_t start, size_t end){
    if(start > end) furi_crash("calc_crc16: start bit can't be more end bit\r\n");
    uint16_t crc = 0xffff;
    end = end-1;//TODO Это хотфикс.(Костыль) Переделать + во всех местах поменять расчёт по массивам.
    for(size_t i = start; i <= end; ++i) {
        crc ^= data[i] << 8;
        for (int j = 0; j < 8; j++){
            crc = crc & 0x8000 ? (crc << 1) ^ 0x1021 : crc << 1;
        }
    }
    return (crc << 8 | crc >> 8); //LE format
}

bool check_crc(uint16_t crc, const uint8_t* data, size_t shift, size_t len)
{
    uint16_t crc_check = calc_crc16(data, shift, len);
    FURI_LOG_D("check_crc()", "crc = %#08x | crc_check = %#08x", crc, crc_check);
    FURI_LOG_D("check_crc()", "data_start = %x | data_end = %x", data[shift], data[len]);
    return crc_check == crc;
}

void prepend_data_to_array(uint8_t **data, size_t old_size, const uint8_t *new_data, size_t new_size) {
    uint8_t *new_buffer = malloc(old_size + new_size);
    memcpy(new_buffer, new_data, new_size);
    memcpy(new_buffer + new_size, *data, old_size);
    free(*data);
    *data = new_buffer;
}

uint16_t two_uint8t_to_uint16t_LE(const uint8_t bytes[2]){
    FURI_LOG_D("two_uint8t_to_uint16t_LE", "bytes[0] = %x bytes[1] = %x", bytes[0], bytes[1]);
    return (bytes[0] << 8) | bytes[1];
}

uint16_t two_uint8t_to_uint16t_BE(const uint8_t bytes[2]){
    FURI_LOG_D("two_uint8t_to_uint16t_BE", "bytes[0] = %x bytes[1] = %x", bytes[0], bytes[1]);
    return (bytes[1] << 8) | bytes[0];
}

void uint16t_LE_to_uint8t_bytes(uint16_t uint16, uint8_t* bytes){
    bytes[0] = uint16 >> 8;
    bytes[1] = uint16;
}

void uint16t_BE_to_uint8t_bytes(uint16_t uint16, uint8_t* bytes){
    bytes[0] = uint16;
    bytes[1] = uint16 >> 8;
}

uint32_t byte_array_to_uint32t_LE(const uint8_t *byteArray) {
    uint32_t result = 0;
    for (int i = 0; i < 4; i++) {
        result |= ((uint32_t) byteArray[i]) << (8 * i);
    }
    return result;
}

void add_bytes_to_arr(
    uint8_t* target_array,
    size_t start, size_t target_arr_size,
    const uint8_t* dest_arr, size_t dest_arr_size)
{
    if(start > target_arr_size) furi_crash("add_bytes_to_arr(): start > target_arr_size");
    if((target_arr_size - start) < dest_arr_size) furi_crash("add_bytes_to_arr(): dest array + shift > target array");
    for(size_t i = 0; i < dest_arr_size; ++i) {
        target_array[i + start] = dest_arr[i];
    }
}
