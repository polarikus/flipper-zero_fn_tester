//
// Created by Игорь Данилов on 23.02.2023.
//
#pragma once

#include "furi.h"
#include "fn.h"
#include "fn_commands.h"
#include "applications_user/fn_test/lib/fn/errors/fn_errors.h"
#include "applications_user/fn_test/lib/fn/data_types/fn_ffd.h"
#include "applications_user/fn_test/lib/fn/data_types/fn_data_types.h"

/**
 * Версия ПО ФН
 * fw_version Версия ПО
 * fw_mode Является ли ФН МГМ или он боевой
 */
typedef struct FwVersion {
    char fw_version[16];
    uint8_t fw_mode;
} FNFwVersion;

typedef struct DateTime {
    uint8_t year;
    uint8_t mouth;
    uint8_t date;
    uint8_t hour;
    uint8_t minute;
} DateTime;

/**
 * Структура с основной информацией об ФН, необходима для исполнения остальных команд
 */
struct FNInfo {
    FNState fn_state;
    FN_FFD ffd_version;
    FN_FFD max_ffd_version;
    char serial_number[18];
    FNFwVersion fw_version;
    bool is_session_open;
    uint32_t last_doc_number;
    DateTime date_time;
    uint8_t fn_warn_flags;
};

