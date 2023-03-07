//
// Created by Игорь Данилов on 04.03.2023.
//
#include "fn_worker_i.h"
#include "fn_helpers.h"
#include "fn_uart_i.h"
#include "fn.h"


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
    FNCustomEventWorker event;

    fn_uart_start_thread(worker->uart);
    uint8_t cmd[6];
    cmd[0] = 0x04;
    cmd[1] = 0x01;
    cmd[2] = 0x00;
    cmd[3] = 0x31;
    cmd[4] = 0xDE;
    cmd[5] = 0xDD;
    fn_uart_trx(worker->uart, cmd, 6, 300);
    RxStatus status = fn_worker_wait_uart_cb(worker, 300);
    FURI_LOG_D("FN_WORKER_MODE", "RX_STATUS1 = %d", status);

    while(status != FNRxStatusOk)
    {
        fn_uart_trx(worker->uart, cmd, 6, 2000);
        status = fn_worker_wait_uart_cb(worker, 500);
        FURI_LOG_D("FN_WORKER_MODE", "RX_STATUS2 = %d", status);
        furi_delay_tick(10); // to give some time to OS
        if(fn_worker_check_for_stop(worker)) return;
    }
    uint8_t result[50];
    result[0] = 0;
    size_t len = furi_stream_buffer_receive(worker->stream,result, 50, 0);
    FURI_LOG_D("FN_WORKER_MODE", "furi_stream_buffer_len = %d", len);
    FURI_LOG_D("FN_WORKER_MODE", "result[0]= %#08x", result[0]);
    FURI_LOG_D("FN_WORKER_MODE", "result[3] = %#08x", result[3]);
    if(result[0] == 0x04 && len > 0)
    {
        if(result[3] == 0x00)
        {
            event = FNCustomEventWorkerFNIdentified;
        } else
        {
            event = FNCustomEventWorkerFNError;
        }
    } else {
        event = FNCustomEventWorkerFNNotResponse;
    }
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
    fn_uart_stop_thread(worker->uart);
    fn_worker_run_callback(worker, event);
}