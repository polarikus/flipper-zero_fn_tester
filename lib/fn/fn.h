//
// Created by Игорь Данилов on 23.02.2023.
//
#include <furi.h>

typedef struct FNInfo FNInfo;



bool fn_is_session_open(const FNInfo* fn);

char* fn_get_fn_state(const FNInfo* fn);

uint32_t fn_get_last_document_number(const FNInfo* fn);

void fn_info_copy(FNInfo* dest, const FNInfo* src);

void fn_get_last_document_datetime(const FNInfo* fn, FuriString* string);

void fn_get_warn_flags_str(const FNInfo* fn, FuriString* string);

bool fn_is_warn_flags_not_null(const FNInfo* fn);

/**
 * Является ли ФН МГМ
 * @param fn
 * @return bool (true = МГМ)
 */
bool fn_is_mgm(const FNInfo* fn);

const char* fn_get_max_ffd_version_string(const FNInfo* fn);

/**
 * Получить версию ФФД ФН
 * @param fn
 * @return Строка с версией ФФД
 */
const char* fn_get_ffd_version_string(const FNInfo* fn);

/**
 * Получить серийный номер ФН
 * @param fn
 * @return Строка с серийным номером
 */
const char* fn_get_sn(const FNInfo* fn);

/**
 * Получить версию ПО ФН
 * @param fn
 * @return Строка с версией ПО
 */
const char* fn_get_fw_version(const FNInfo* fn);

/**
 * Получить ENUM с версией ФФД
 * @param fn
 * @return uint8_t ???
 */
uint8_t fn_get_ffd_enum(const FNInfo* fn);