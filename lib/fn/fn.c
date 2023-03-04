//
// Created by Игорь Данилов on 23.02.2023.
//
#include "fn_i.h"

#define FFD_1_STR "1"
#define FFD_1_05_STR "1.05"
#define FFD_1_1_STR "1.1"
#define FFD_1_2_STR "1.2"

bool fn_is_mgm(const FNInfo* fn)
{
    return !(fn->fw_version.fw_mode);
}

const char* fn_get_fw_version(const FNInfo* fn)
{
    return fn->fw_version.fw_version;
}

const char* fn_get_sn(const FNInfo* fn)
{
    return fn->serial_number;
}

const char* fn_get_revision(const FNInfo* fn)
{
    return fn->revision_name;
}

const char* fn_get_ffd_version_string(const FNInfo* fn)
{
    switch(fn->ffd_enum) {
    case FFD_1:
        return FFD_1_STR;
    case FFD_1_05:
        return FFD_1_05_STR;
    case FFD_1_1:
        return FFD_1_1_STR;
    case FFD_1_2:
        return FFD_1_2_STR;
    default:
        return "unknown";
    }
}

uint8_t fn_get_ffd_enum(const FNInfo* fn){
    return fn->ffd_enum;
}