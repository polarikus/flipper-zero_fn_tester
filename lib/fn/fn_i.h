//
// Created by Игорь Данилов on 23.02.2023.
//
#pragma once

#include <furi.h>
#include "fn.h"
#include "fn_errors.h"
/**
 * <a href="http://www.consultant.ru/document/cons_doc_LAW_362322/c7bcefffc3cbf3b1189c0f92e21f7bd930aa5b96/">Версия ФФД</a>
 */
typedef enum {
    FFD_1,
    FFD_1_05,
    FFD_1_1,
    FFD_1_2
} FN_FFD;
/**
 * <a href="https://data.nalog.ru/html/sites/www.new.nalog.ru/docs/kkt/1_2_05_090621.pdf">Команды к ФН</a>
 */
typedef enum {
    FNTestGetFNStatus = 0x30,/** Запрос статуса ФН */
    FNTestGetFNEndDate = 0x32 /** Запрос срока действия ФН */
} FN_CMD;

/**
 * Версия ПО ФН
 * fw_version Версия ПО
 * fw_mode Является ли ФН МГМ или он боевой
 */
typedef struct FwVersion {
    const char* fw_version;
    uint8_t fw_mode;
} FNFwVersion;

/**
 * Структура с основной информацией об ФН, необходима для исполнения остальных команд
 */
struct FNInfo {
    FNError last_fn_error;
    FN_FFD ffd_enum;
    const char* revision_name;
    const char* serial_number;
    FNFwVersion fw_version;
    bool is_session_open;
};

