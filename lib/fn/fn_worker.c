//
// Created by Игорь Данилов on 04.03.2023.
//
#include "fn_worker_i.h"
typedef enum {
    FNEventStopThread = (1 << 0),
    FNEventDetect = (1 << 1),
    FNEventAll =
        (FNEventStopThread | FNEventDetect)
} SPIMemEventEventType;

static int32_t fn_worker_thread(void* thread_context);

FNWorker* fn_worker_alloc() {
    FNWorker* worker = malloc(sizeof(FNWorker));
    worker->callback = NULL;
    worker->thread = furi_thread_alloc();
    worker->mode_index = FNWorkerModeIdle;
    furi_thread_set_name(worker->thread, "FNWorker");
    furi_thread_set_callback(worker->thread, fn_worker_thread);
    furi_thread_set_context(worker->thread, worker);
    furi_thread_set_stack_size(worker->thread, 10240);
    return worker;
}

static int32_t fn_worker_thread(void* thread_context) {
    FNWorker* worker = thread_context;
    while(true) {
        uint32_t flags = furi_thread_flags_wait(FNEventAll, FuriFlagWaitAny, FuriWaitForever);
        if(flags & FNEventStopThread) break;
        /*
        if(flags != (unsigned)FuriFlagErrorTimeout) {
            if(flags & SPIMemEventStopThread) break;
            if(flags & SPIMemEventChipDetect) worker->mode_index = SPIMemWorkerModeChipDetect;
            if(flags & SPIMemEventRead) worker->mode_index = SPIMemWorkerModeRead;
            if(flags & SPIMemEventVerify) worker->mode_index = SPIMemWorkerModeVerify;
            if(flags & SPIMemEventErase) worker->mode_index = SPIMemWorkerModeErase;
            if(flags & SPIMemEventWrite) worker->mode_index = SPIMemWorkerModeWrite;
            if(spi_mem_worker_modes[worker->mode_index].process) {
                spi_mem_worker_modes[worker->mode_index].process(worker);
            }
            worker->mode_index = SPIMemWorkerModeIdle;
            */
        worker->mode_index = FNWorkerModeIdle;
    }
    return 0;
}