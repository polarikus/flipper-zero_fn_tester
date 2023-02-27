//
// Created by Игорь Данилов on 23.02.2023.
//
#pragma once

#include <furi.h>
#include "fn.h"

typedef enum {
    FFD_1,
    FFD_1_05,
    FFD_1_1,
    FFD_1_2
} FN_FFD;

typedef enum {
    FNTestGetFNStatus = 0x30,
    FNTestGetFNEndDate = 0x32
} FN_CMD;

typedef struct FwVersion {
    const char* fw_version[16];
    uint8_t fw_mode;
} FNFwVersion;

struct FNInfo {
    FN_FFD ffd_enum;
    const char* revision_name;
    const char* serial_number;
    FNFwVersion fw_version;
};

