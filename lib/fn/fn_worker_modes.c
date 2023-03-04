//
// Created by Игорь Данилов on 04.03.2023.
//
#include "fn_worker_i.h"
#include "fn_helpers.h"
#include "fn_uart_i.h"
#include "fn.h"

static void fn_worker_fn_detect_process(FNWorker * worker);

const FNWorkerModeType spi_mem_worker_modes[] = {
    [FNWorkerModeIdle] = {.process = NULL},
    [FNWorkerModeFNDetect] = {.process = fn_worker_fn_detect_process}
};