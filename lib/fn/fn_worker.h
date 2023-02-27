//
// Created by Игорь Данилов on 23.02.2023.
//
#pragma once

#include <furi.h>
#include "fn.h"

typedef struct FNWorker FNWorker;

typedef struct {
    void (*const process)(FNWorker* worker);
} FNWorkerModeType;

typedef enum {
    SPIMemCustomEventWorkerChipIdentified,
    SPIMemCustomEventWorkerChipUnknown,
    SPIMemCustomEventWorkerBlockReaded,
    SPIMemCustomEventWorkerChipFail,
    SPIMemCustomEventWorkerFileFail,
    SPIMemCustomEventWorkerDone,
    SPIMemCustomEventWorkerVerifyFail,
} FNCustomEventWorker;

typedef void (*SPIMemWorkerCallback)(void* context, FNCustomEventWorker event);

FNWorker* fn_test_worker_alloc();
void fn_test_worker_free(FNWorker* worker);
void fn_test_worker_start_thread(FNWorker* worker);
void fn_test_worker_stop_thread(FNWorker* worker);
bool fn_test_worker_check_for_stop(FNWorker* worker);
void fn_worker_fn_detect_start(
    FNInfo* fn_info,
    FNWorker* worker,
    SPIMemWorkerCallback callback,
    void* context);
void spi_mem_worker_read_start(
    FNInfo* fn_info,
    FNWorker* worker,
    SPIMemWorkerCallback callback,
    void* context);
void spi_mem_worker_verify_start(
    FNInfo* fn_info,
    FNWorker* worker,
    SPIMemWorkerCallback callback,
    void* context);
void spi_mem_worker_erase_start(
    FNInfo* fn_info,
    FNWorker* worker,
    SPIMemWorkerCallback callback,
    void* context);
void spi_mem_worker_write_start(
    FNInfo * fn_info,
    FNWorker* worker,
    SPIMemWorkerCallback callback,
    void* context);


