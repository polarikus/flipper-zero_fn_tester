//
// Created by Игорь Данилов on 23.02.2023.
//
#include "fn_i.h"

bool fn_is_mgm(const FNInfo fn)
{
    return !(fn.fw_version.fw_mode);
}