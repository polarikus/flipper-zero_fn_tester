//
// Created by Игорь Данилов on 23.02.2023.
//
#pragma once

#include "fn_worker.h"

typedef enum {
    SPIMemWorkerModeIdle,
    SPIMemWorkerModeFNDetect,
    SPIMemWorkerModeRead,
    SPIMemWorkerModeSendCMD
} FNWorkerMode;

struct FNWorker {
    SPIMemChip* chip_info;
    found_chips_t* found_chips;
    FNWorkerMode mode_index;
    FNWorkerCallback callback;
    void* cb_ctx;
    FuriThread* thread;
};

extern const FNWorkerModeType fn_worker_modes[];