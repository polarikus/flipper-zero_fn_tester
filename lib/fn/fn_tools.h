//
// Created by Игорь Данилов on 05.03.2023.
//

#pragma once
#include "errors/fn_errors.h"
#include "fn_objects/fn_info/fn_i.h"
#include "fn_worker_i.h"
#include "fn_objects/life_info/fn_life_info_i.h"

typedef enum {
    FNToolOk,
    FNToolTimeout,
    FNToolWrongCRC,
    FNToolWrongFNDataLen,
    FNToolFnError
} FNToolCmdStatus;


FNToolCmdStatus fn_tool_get_fn_info(FNError *fn_error, FNWorker* fn_worker, FNInfo* fnInfo);

FNToolCmdStatus fn_tool_get_fn_life_data(FNError *fn_error, FNWorker* fn_worker, FNLifeInfo* fnInfo);

FNToolCmdStatus fn_tool_flash_MGM(FNError *fn_error, FNWorker* fn_worker);
