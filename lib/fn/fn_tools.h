//
// Created by Игорь Данилов on 05.03.2023.
//

#pragma once
#include "fn_errors.h"
#include "fn_i.h"
#include "fn_worker_i.h"

typedef enum {
    FNToolOk,
    FNToolTimeout,
    FNToolWrongCRC,
    FNToolWrongFNDataLen,
    FNToolFnError
} FNToolCmdStatus;


FNToolCmdStatus fn_tool_get_fn_info(FNWorker* fn_worker, FNInfo* fnInfo);

FNToolCmdStatus fn_tool_get_fn_fw(FNWorker* fn_worker, FNInfo* fnInfo);
