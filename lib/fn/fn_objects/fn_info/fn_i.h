//
// Created by Игорь Данилов on 23.02.2023.
//
#pragma once

#include "furi.h"
#include "fn.h"
#include "fn_commands.h"
#include "errors/fn_errors.h"
#include "data_types/fn_ffd.h"
#include "data_types/fn_data_types.h"
#include "datetime.h"
//TODO Описать все структуры и параметры. Поменять неймминги на нормальные
/**
 * Версия ПО ФН
 * fw_version Версия ПО
 * fw_mode Является ли ФН МГМ или он боевой
 */
typedef struct FwVersion {
    char fw_version[16];
    uint8_t fw_mode;
} FNFwVersion;

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

