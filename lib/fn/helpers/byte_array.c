//
// Created by Игорь Данилов on 16.09.2023.
//
#include "byte_array_i.h"

ByteArray* byte_array_alloc(size_t max_size){
    ByteArray* byte_array = malloc(sizeof(ByteArray));
    byte_array->size = 0;
    byte_array->max_size = max_size;
    byte_array->array = malloc(byte_array->max_size);
    return byte_array;
}

void byte_array_free(ByteArray* byte_array){
    free(byte_array->array);
    free(byte_array);
}

size_t byte_array_push(ByteArray* byte_array, const void * data, size_t data_size){
    if(data_size == 0) return BYTE_ARRAY_FAILURE;
    if(byte_array->size > (byte_array->size + data_size)) return BYTE_ARRAY_FAILURE;
    ( void ) memcpy( ( void * ) ( &( byte_array->array[byte_array->size] ) ), ( const void * ) data, data_size );
    return byte_array->size = byte_array->size + data_size;
}

void byte_array_read_int_LE(ByteArray* byte_array, void* dest, size_t size) {
    UNUSED(byte_array);
    UNUSED(size);
    UNUSED(dest);
}

void byte_array_read_int_BE(ByteArray* byte_array,void* dest, size_t size) {
    UNUSED(byte_array);
    UNUSED(size);
    UNUSED(dest);
}

void byte_array_get_int_LE(ByteArray* byte_array, void* dest, size_t size) {
    UNUSED(byte_array);
    UNUSED(size);
    UNUSED(dest);
}

void byte_array_get_int_BE(ByteArray* byte_array, void* dest, size_t size) {
    UNUSED(byte_array);
    UNUSED(size);
    if (size == sizeof(uint8_t)) {
        *((uint8_t*) dest) = 1;
    }
}