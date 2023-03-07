//
// Created by Игорь Данилов on 05.03.2023.
//

#pragma once
#include "fn_errors.h"
#include "fn.h"

typedef enum {
    FNToolOk,
    FNToolTimeout,
    FNToolWrongCRC,
    FNToolWrongFNDataLen
} FNToolCmdStatus;

FNToolCmdStatus get_fn_info(FNError fn_error, FNInfo* fn_info);
