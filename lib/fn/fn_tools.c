//
// Created by Игорь Данилов on 05.03.2023.
//

#include "fn_tools.h"
#include "fn_uart_i.h"
#include "fn_helpers.h"
#define TAG "FN_TOOLS"

typedef struct FNAnswer {
    FNToolCmdStatus status;
    FNError error;
    uint16_t data_len;
    uint8_t* data;
} FNAnswer;

static void free_fn_answer(FNAnswer* fn_answer){
    if(fn_answer->data_len > 0){
        free(fn_answer->data);
    }
    free(fn_answer);
}

static bool fn_trx(FNWorker* worker, FNAnswer* answer, FN_CMD cmd, const uint8_t* data, size_t data_len){
    //TODO Рефакторинг и оптимизация (Выглядит как кошмар)
    const size_t tx_size = 6 + data_len;
    uint8_t* tx = malloc(tx_size);
    tx[0] = 0x04;
    UNUSED(cmd);
    tx[3] =  0x31;
    if(data_len > 0){
        uint8_t tx_data_len[2];
        if(data == NULL) furi_crash("fn_trx() Data must be NULL if data_len > 0!");
        uint16t_LE_to_uint8t_bytes((u_int16_t)data_len + 1, tx_data_len);
        add_bytes_to_arr(tx, 2, tx_size, tx_data_len, 2);
    } else{
        tx[1] = 0x01;
        tx[2] = 0x00;
    }
    FURI_LOG_D(TAG, "tx[3] = %#08x", tx[3]);

    uint8_t crc[2];
    crc[0] = 0xDE;
    crc[1] = 0xDD;
    //uint16t_LE_to_uint8t_bytes(calc_crc16(tx, 1, tx_size - 2), crc);
    add_bytes_to_arr(tx, 4, tx_size, crc, 2);
    FURI_LOG_D(TAG, "tx[3] = %#08x", tx[3]);
    answer->status = FNToolTimeout;
    answer->data_len = 0;
    answer->data = NULL;
    answer->error = FNOk;
    fn_uart_start_thread(worker->uart);
    fn_uart_trx(worker->uart, tx, tx_size, 300);
    FURI_LOG_D(TAG, "tx[3] = %#08x", tx[3]);
    RxStatus status = fn_worker_wait_uart_cb(worker, FuriWaitForever);
    FURI_LOG_D(TAG, "RxStatus %d", status);
    size_t rx_data_available = furi_stream_buffer_bytes_available(worker->stream);
    if(status == FNRxStatusOk && rx_data_available > 6)
    {
        uint8_t* rx_buff = malloc(rx_data_available);
        furi_stream_buffer_receive(worker->stream,rx_buff, rx_data_available, 0);
        if(rx_buff[0] == 0x04){
            answer->status = FNToolOk;
            uint8_t rx_data_len_b[2];
            rx_data_len_b[0] = rx_buff[1];
            rx_data_len_b[1] = rx_buff[2];
            uint16_t rx_data_len = two_uint8t_to_uint16t_BE(rx_data_len_b);
            if(rx_data_len != rx_data_available - 5){
                FURI_LOG_D(TAG, "FNToolWrongFNDataLen rx_data_len = %d rx_data_available = %d", rx_data_len, rx_data_available);
                answer->status = FNToolWrongFNDataLen;
                fn_uart_stop_thread(worker->uart);
                free(tx);
                return false;
            }
            answer->error = rx_buff[3];
            answer->data_len = rx_data_len;

            uint8_t rx_crc_b[2];
            rx_crc_b[0] = rx_buff[rx_data_available - 2];
            rx_crc_b[1] = rx_buff[rx_data_available - 1];
            uint16_t rx_crc = two_uint8t_to_uint16t_LE(rx_crc_b);
            if(!check_crc(rx_crc, rx_buff, 1, rx_data_available - 2)){
                answer->status = FNToolWrongCRC;
                fn_uart_stop_thread(worker->uart);
                free(tx);
                return false;
            }
            if(answer->data_len  > 1){
                answer->data = malloc(rx_data_len - 1);
                for(uint16_t i = 0; i < rx_data_len - 2; ++i) {
                    answer->data[0] = rx_buff[i + 3];
                }
            } else {
                answer->data_len = 0;
            }
        }

    }
    fn_uart_stop_thread(worker->uart);
    free(tx);
    return true;
}

FNToolCmdStatus fn_tool_get_fn_info(FNWorker* fn_worker, FNInfo* fnInfo){
    furi_check(fn_worker);
    furi_check(fnInfo);
    FNToolCmdStatus status;

    FNAnswer* fn_answer = malloc(sizeof(FNAnswer));
    fn_trx(fn_worker, fn_answer, FNTestGetFNStatus, NULL, 0);
    FURI_LOG_D(TAG, "fn_answer->status %d", fn_answer->status);
    status = fn_answer->status;
    if(status!= FNToolOk) return status;

    fnInfo->last_fn_error = fn_answer->error;
    if(fn_answer->error != FNOk){
        status = FNToolFnError;
        return status;
    }

    FURI_LOG_D(TAG, "fn_answer->data_len %d", fn_answer->data_len);
    if(fn_answer->data_len != 17) //TODO Поправить
    {
        status = FNToolWrongFNDataLen;
        return status;
    }

    fnInfo->is_session_open = (bool)fn_answer->data[3];

    free_fn_answer(fn_answer);

    return status;
}






