//
// Created by Игорь Данилов on 23.02.2023.
//
#include <furi.h>

typedef struct FNInfo FNInfo;

/**
 * Является ли ФН МГМ
 * @param fn
 * @return bool (true = МГМ)
 */
bool fn_is_mgm(const FNInfo* fn);

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
const char* fn_get_revision(const FNInfo* fn);

/**
 * Получить ENUM с версией ФФД
 * @param fn
 * @return uint8_t ???
 */
uint8_t fn_get_ffd_enum(const FNInfo* fn);