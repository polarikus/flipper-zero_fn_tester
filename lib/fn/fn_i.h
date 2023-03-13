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
    FFD_NONE,
    FFD_1,
    FFD_1_05,
    FFD_1_1,
    FFD_1_2
} FN_FFD;
/**
 * <a href="https://data.nalog.ru/html/sites/www.new.nalog.ru/docs/kkt/1_2_05_090621.pdf">Команды к ФН</a>
 */
typedef enum {
    FN_CMDGetFNStatus = 0x30,/** Запрос статуса ФН */
    FN_CMDGetFNEndDate = 0x32, /** Запрос срока действия ФН */
    FN_CMDGetFNFwVersion= 0x33, /** Запрос версии ПО ФН */
    FN_CMDGetFFD = 0x3A /** Запрос версии ФФД */
} FN_CMD;

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

typedef enum {
    FNStage1 = 1,
    FNStage2 = 3,
    FNStage3 = 7,
    FNStage4 = 15
} FNState;

/**
 * Структура с основной информацией об ФН, необходима для исполнения остальных команд
 */
struct FNInfo {
    FNError last_fn_error;
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

