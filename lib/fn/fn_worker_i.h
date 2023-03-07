//
// Created by Игорь Данилов on 23.02.2023.
//
#pragma once

#include "fn_worker.h"
#include "fn_uart_i.h"

typedef enum {
    FNWorkerModeIdle,
    FNWorkerModeFNDetect
} FNWorkerMode;

struct FNWorker {
    FNInfo* fn_info;
    FNWorkerMode mode_index;
    FNWorkerCallback callback;
    void* cb_ctx;
    FuriThread* thread;
    UARTApp* uart;
    FuriStreamBuffer* stream;
};

extern const FNWorkerModeType fn_worker_modes[];