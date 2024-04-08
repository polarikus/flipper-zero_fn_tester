//
// Created by Игорь Данилов on 04.03.2023.
//
#include "fn_worker_i.h"
#include "fn_helpers.h"
#include "fn_tools.h"
#include "uart/fn_uart_i.h"
#include "fn_objects/fn_info/fn_i.h"

#define TAG "FNWorkerModes"

static void fn_worker_fn_detect_process(FNWorker * worker);
static void fn_worker_get_life_info_process(FNWorker * worker);
static void fn_worker_flash_mgm_process(FNWorker * worker);

const FNWorkerModeType fn_worker_modes[] = {
    [FNWorkerModeIdle] = {.process = NULL},
    [FNWorkerModeFNDetect] = {.process = fn_worker_fn_detect_process},
    [FNWorkerModeGetLifeInfo] = {.process = fn_worker_get_life_info_process},
    [FNWorkerModeFlashMGM] = {.process = fn_worker_flash_mgm_process},
};

static void fn_worker_run_callback(FNWorker* worker, FNCustomEventWorker event, FNError error) {
    if(worker->callback) {
        worker->callback(worker->cb_ctx, event, error);
    }
}

static void fn_worker_fn_detect_process(FNWorker * worker){
    //furi_check(0, TAG" fn_worker_fn_detect_process");
    furi_assert(worker);
    furi_assert(worker->fn_info);
    FNCustomEventWorker event = FNCustomEventWorkerFNNotResponse;
    FNError fn_error = FNOk;


    FNInfo* fnInfo = malloc(sizeof(FNInfo));

    //int ctr = 0;
    FNToolCmdStatus cmd_status = FNToolTimeout;
    //FNToolCmdStatus cmd_status = fn_tool_get_fn_fw(worker, fnInfo);
    FURI_LOG_D(TAG, "cmd_status %d", cmd_status);
    int ctr = 0;
    while(cmd_status != FNToolOk && ctr < 5)
    {
        if(fn_worker_check_for_stop(worker)) return;
        cmd_status = fn_tool_get_fn_info(&fn_error, worker, fnInfo);
        ctr++;
    }


    if(cmd_status == FNToolOk){
        event = FNCustomEventWorkerFNIdentified;
    } else if(cmd_status == FNToolFnError)
    {
        FURI_LOG_D(TAG " detect", "FN Error: %#08x",  fn_error);
        event = FNCustomEventWorkerFNError;
    }
    fn_info_copy(worker->fn_info, fnInfo);
    //cmd_status = fn_tool_get_fn_fw(worker, fnInfo);
    //FURI_LOG_D(TAG " detect", "FN SN: %s",  fnInfo->serial_number);
    free(fnInfo);
    fn_worker_run_callback(worker, event, fn_error);
}

static void fn_worker_get_life_info_process(FNWorker * worker){
    furi_check(worker->fn_answer_data, TAG" fn_worker_get_life_info_process");
    FNCustomEventWorker event = FNCustomEventWorkerFNNotResponse;
    FNLifeInfo* life_info_tmp = fn_life_info_alloc();
    FNError fn_error = FNOk;
    FNToolCmdStatus cmd_status = fn_tool_get_fn_life_data(&fn_error, worker, life_info_tmp);
    if(cmd_status == FNToolOk){
        event = FNCustomEventWorkerDone;
    } else if(cmd_status == FNToolFnError){
        event = FNCustomEventWorkerFNError;
    }
    memcpy(worker->fn_answer_data, life_info_tmp, sizeof(FNLifeInfo));
    fn_life_info_free(life_info_tmp);
    fn_worker_run_callback(worker, event, fn_error);
}

static void fn_worker_flash_mgm_process(FNWorker * worker){
    //furi_check(worker->fn_answer_data);
    FNCustomEventWorker event = FNCustomEventWorkerFNNotResponse;
    FNError fn_error = FNOk;

    FNToolCmdStatus cmd_status = fn_tool_flash_MGM(&fn_error, worker);

    if(cmd_status == FNToolOk){
        event = FNCustomEventWorkerDone;
    } else if(cmd_status == FNToolFnError){
        event = FNCustomEventWorkerFNError;
    }

    fn_worker_run_callback(worker, event, fn_error);
}
