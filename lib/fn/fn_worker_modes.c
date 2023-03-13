//
// Created by Игорь Данилов on 04.03.2023.
//
#include "fn_worker_i.h"
#include "fn_helpers.h"
#include "fn_tools.h"
#include "fn_uart_i.h"
#include "fn_i.h"

#define TAG "FNWorkerModes"

static void fn_worker_fn_detect_process(FNWorker * worker);

const FNWorkerModeType fn_worker_modes[] = {
    [FNWorkerModeIdle] = {.process = NULL},
    [FNWorkerModeFNDetect] = {.process = fn_worker_fn_detect_process}
};

static void fn_worker_run_callback(FNWorker* worker, FNCustomEventWorker event) {
    if(worker->callback) {
        worker->callback(worker->cb_ctx, event);
    }
}

static void fn_worker_fn_detect_process(FNWorker * worker){
    furi_assert(worker);
    furi_assert(worker->fn_info);
    FNCustomEventWorker event = FNCustomEventWorkerFNNotResponse;


    /*
    while(!spi_mem_tools_read_chip_info(worker->chip_info)) {
        furi_delay_tick(10); // to give some time to OS
        if(spi_mem_worker_check_for_stop(worker)) return;
    }
    if(spi_mem_chip_find_all(worker->chip_info, *worker->found_chips)) {
        event = SPIMemCustomEventWorkerChipIdentified;
    } else {
        event = SPIMemCustomEventWorkerChipUnknown;
    }
     */
    FNInfo* fnInfo = malloc(sizeof(FNInfo));

    //int ctr = 0;
    FNToolCmdStatus cmd_status = fn_tool_get_fn_info(worker, fnInfo);
    //FNToolCmdStatus cmd_status = fn_tool_get_fn_fw(worker, fnInfo);
    FURI_LOG_D(TAG, "cmd_status %d", cmd_status);
    /*
    while((cmd_status == FNToolTimeout || cmd_status == FNToolWrongFNDataLen || cmd_status == FNToolWrongCRC) && ctr < 5)
    {
        if(fn_worker_check_for_stop(worker)) break;
        cmd_status = fn_tool_get_fn_info(worker, fnInfo);
        ctr++;
    }
     */

    if(cmd_status == FNToolOk){
        event = FNCustomEventWorkerFNIdentified;
    } else if(cmd_status == FNToolFnError)
    {
        FURI_LOG_D(TAG " detect", "FN Error: %#08x",  fnInfo->last_fn_error);
        event = FNCustomEventWorkerFNError;
    }
    fn_info_copy(worker->fn_info, fnInfo);
    //cmd_status = fn_tool_get_fn_fw(worker, fnInfo);
    //FURI_LOG_D(TAG " detect", "FN SN: %s",  fnInfo->serial_number);
    free(fnInfo);
    fn_worker_run_callback(worker, event);
}