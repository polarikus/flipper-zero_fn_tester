//
// Created by Игорь Данилов on 05.03.2023.
//

#include "fn_tools.h"
#include "fn_uart_i.h"
#include "fn_helpers.h"
#include <furi_hal_rtc.h>
#define TAG "FN_TOOLS"

#define CHECK_STATUS(status) \
if(status != 0) return status;


typedef struct FNAnswer {
    FNToolCmdStatus status;
    FNError error;
    uint16_t data_len;
    uint8_t* data;
} FNAnswer;

static FNToolCmdStatus check_data_len(FNAnswer* answer, uint16_t expected_length)
{
    if(answer->data_len != expected_length) return FNToolWrongFNDataLen;
    return FNToolOk;
}

static FNToolCmdStatus check_errors(FNAnswer* answer)
{
    if(answer->error != FNOk) return FNToolFnError;
    return FNToolOk;
}

static void log_fn_data(uint8_t* data, uint16_t len, bool is_answer)
{
    FuriLogLevel logLevel = furi_log_get_level();
    if(logLevel == FuriLogLevelDebug)
    {
        FuriString* string = furi_string_alloc();
        if(is_answer){
            furi_string_cat_printf(string, "<< ");
        } else{
            furi_string_cat_printf(string, ">> ");
        }
        for(int i = 0; i < len; ++i) {
            furi_string_cat_printf(string, "%02x ", data[i]);
        }
        FURI_LOG_D(TAG, "%s", furi_string_get_cstr(string));
        furi_string_free(string);
    }
}


static void free_fn_answer_data(FNAnswer* fn_answer)
{
    assert(fn_answer->data);
    free(fn_answer->data);
}

static void free_fn_answer(FNAnswer* fn_answer){
    if(fn_answer->data != NULL){
        free_fn_answer_data(fn_answer);
    }
    free(fn_answer);
}

static bool fn_trx(FNWorker* worker, FNAnswer* answer, FN_CMD cmd, const uint8_t* data, size_t data_len, uint32_t timeout){
    //TODO Рефакторинг и оптимизация (Выглядит как кошмар)
    const size_t tx_size = FN_UART_MIN_PACKAGE_LEN + data_len;
    uint8_t* tx = malloc(tx_size);
    tx[FN_UART_MSG_START_SHIFT] = FN_UART_MSG_START_BYTE;
    UNUSED(cmd);
    tx[FN_UART_CMD_SHIFT] =  cmd;
    if(data_len > 0){
        uint8_t tx_data_len[2];
        if(data == NULL) furi_crash("fn_trx() Data must be NULL if data_len > 0!");
        uint16t_BE_to_uint8t_bytes((u_int16_t)data_len + 1, tx_data_len);
        add_bytes_to_arr(tx, 1, tx_size, tx_data_len, 2);
        add_bytes_to_arr(tx, 4, tx_size, data, data_len);
    } else{
        tx[FN_UART_LEN_SHIFT] = 0x01;
        tx[FN_UART_LEN_SHIFT + 1] = 0x00;
    }
    FURI_LOG_D(TAG, "tx[3] = %#08x", tx[3]);

    uint8_t crc[FN_UART_CRC_SIZE];// FF CD для 0x30
    //crc[0] = 0xff;
    //crc[1] = 0xcd;

    uint16t_LE_to_uint8t_bytes(calc_crc16(tx, FN_UART_LEN_SHIFT, tx_size - FN_UART_CRC_SIZE), crc);
    add_bytes_to_arr(tx, FN_UART_DATA_SHIFT + data_len, tx_size, crc, FN_UART_CRC_SIZE);
    log_fn_data(tx, tx_size, false);
    answer->status = FNToolTimeout;
    answer->data_len = 0;
    answer->data = NULL;
    answer->error = FNOk;
    fn_uart_trx(worker->uart, tx, tx_size, timeout);
    FURI_LOG_D(TAG, "tx[3] = %#08x", tx[3]);
    RxStatus status = fn_worker_wait_uart_cb(worker, FuriWaitForever);
    FURI_LOG_D(TAG, "RxStatus %d", status);
    size_t rx_data_available = furi_stream_buffer_bytes_available(worker->stream);
    if(status == FNRxStatusOk && rx_data_available >= FN_UART_MIN_PACKAGE_LEN)
    {
        uint8_t* rx_buff = malloc(rx_data_available);
        furi_stream_buffer_receive(worker->stream,rx_buff, rx_data_available, 0);
        log_fn_data(rx_buff, rx_data_available, true);

        if(rx_buff[FN_UART_MSG_START_SHIFT] == FN_UART_MSG_START_BYTE){
            answer->status = FNToolOk;
            uint8_t rx_data_len_b[FN_UART_LEN_SIZE];
            rx_data_len_b[0] = rx_buff[FN_UART_LEN_SHIFT];
            rx_data_len_b[1] = rx_buff[FN_UART_LEN_SHIFT + 1];
            uint16_t rx_data_len = two_uint8t_to_uint16t_BE(rx_data_len_b);
            if(rx_data_len != rx_data_available - (FN_UART_MSG_START_SIZE + FN_UART_LEN_SIZE + FN_UART_CRC_SIZE)){
                FURI_LOG_D(TAG, "FNToolWrongFNDataLen rx_data_len = %d rx_data_available = %d", rx_data_len, rx_data_available - 5);
                answer->status = FNToolWrongFNDataLen;
                fn_uart_stop_thread(worker->uart);
                free(tx);
                return false;
            }
            answer->error = rx_buff[FN_UART_ANSWER_CODE_SHIFT];
            answer->data_len = rx_data_len - 1;

            uint8_t rx_crc_b[FN_UART_CRC_SIZE];
            rx_crc_b[0] = rx_buff[rx_data_available - 2];
            rx_crc_b[1] = rx_buff[rx_data_available - 1];
            uint16_t rx_crc = two_uint8t_to_uint16t_LE(rx_crc_b);
            if(!check_crc(rx_crc, rx_buff, FN_UART_LEN_SHIFT, rx_data_available - FN_UART_CRC_SIZE)){
                answer->status = FNToolWrongCRC;
                fn_uart_stop_thread(worker->uart);
                free(tx);
                return false;
            }
            if(answer->data_len  > 1){
                answer->data = malloc(rx_data_len);
                for(uint16_t i = 0; i < rx_data_len - 1; ++i) {
                    answer->data[i] = rx_buff[i + 4];
                }
            } else {
                answer->data_len = 0;
            }
            answer->status = check_errors(answer);
            free(rx_buff);
            furi_stream_buffer_reset(worker->stream);
        }

    }
    free(tx);
    return true;
}

FNToolCmdStatus fn_tool_get_fn_info(FNError *fn_error, FNWorker* fn_worker, FNInfo* fnInfo){
    furi_check(fn_worker);
    furi_check(fnInfo);
    FNToolCmdStatus status;
    bool trx_ok;
    fn_uart_start_thread(fn_worker->uart);
    FNAnswer* fn_answer = malloc(sizeof(FNAnswer));
    trx_ok = fn_trx(fn_worker, fn_answer, FN_CMDGetFNStatus, NULL, 0, 300);
    FURI_LOG_D(TAG, "FN_CMDGetFNStatus %d", trx_ok);
    FURI_LOG_D(TAG, "fn_answer->status %d", fn_answer->status);
    status = fn_answer->status;
    if(status!= FNToolOk) return status;

    *fn_error = fn_answer->error;
    status = check_errors(fn_answer);
    CHECK_STATUS(status)

    FURI_LOG_D(TAG, "fn_answer->data_len %d", fn_answer->data_len);
    status = check_data_len(fn_answer, 30);
    CHECK_STATUS(status);


    fnInfo->fn_state = (FNState)fn_answer->data[0];
    //TODO 1 и 2 бит Чёт там про ОФД, оно может и не надо
    fnInfo->is_session_open = (bool)fn_answer->data[3];
    fnInfo->fn_warn_flags = fn_answer->data[4];
    fnInfo->date_time.year = fn_answer->data[5];
    fnInfo->date_time.mouth = fn_answer->data[6];
    fnInfo->date_time.date = fn_answer->data[7];
    fnInfo->date_time.hour = fn_answer->data[8];
    fnInfo->date_time.minute = fn_answer->data[9];
    //Считываем СН ФН с 10 по 16 бит
    for(int i = 0; i < 16; ++i) {
        fnInfo->serial_number[i] = (char)fn_answer->data[10 + i];
        //FURI_LOG_D(TAG, "FN_SN[%d] = %x", i + 10, (char)fn_answer->data[10 + i]);
    }
    fnInfo->serial_number[17] = (char)'\0';
    uint8_t last_FD[4];
    for(int i = 0; i < 4; ++i) {
        last_FD[i] = fn_answer->data[(9 + 17) + i];
        FURI_LOG_D(TAG, "last_FD[%d] = %x", i, last_FD[i]);
    }
    fnInfo->last_doc_number = byte_array_to_uint32t_LE(last_FD);

    //Get FFD Verion
    FURI_LOG_D(TAG, "GET FFD VERSION");
    free_fn_answer_data(fn_answer);
    trx_ok = fn_trx(fn_worker, fn_answer, FN_CMDGetFFD, NULL, 0, 300);
    *fn_error = fn_answer->error;
    FURI_LOG_D(TAG, "FN_CMDGetFFD %d", trx_ok);
    FURI_LOG_D(TAG, "fn_answer->status (getFFD) %d", fn_answer->status);
    FURI_LOG_D(TAG, "fn_answer->error (getFFD) %d", fn_answer->error);
    FURI_LOG_D(TAG, "fn_answer->data_len (getFFD) %d", fn_answer->data_len);
    //log_fn_data(fn_answer->data, fn_answer->data_len, true);
    status = check_data_len(fn_answer, 2);
    CHECK_STATUS(status)
    fnInfo->ffd_version = (FN_FFD)fn_answer->data[0];
    fnInfo->max_ffd_version = (FN_FFD)fn_answer->data[1];

    free_fn_answer_data(fn_answer);
    trx_ok = fn_trx(fn_worker, fn_answer, FN_CMDGetFNFwVersion, NULL, 0, 300);
    *fn_error = fn_answer->error;
    status = check_data_len(fn_answer, 17);
    CHECK_STATUS(status);
    for(int i = 0; i < 16; ++i) {
        fnInfo->fw_version.fw_version[i] = fn_answer->data[i];
    }
    fnInfo->fw_version.fw_mode = fn_answer->data[16];

    free_fn_answer(fn_answer);
    fn_uart_stop_thread(fn_worker->uart);

    return status;
}

FNToolCmdStatus fn_tool_get_fn_life_data(FNError *fn_error, FNWorker* fn_worker, FNLifeInfo* lifeInfo){
    furi_check(fn_worker);
    furi_check(lifeInfo);
    furi_check(fn_worker->fn_info);
    FNToolCmdStatus status;
    bool trx_ok;
    fn_uart_start_thread(fn_worker->uart);
    FNAnswer* fn_answer = malloc(sizeof(FNAnswer));
    trx_ok = fn_trx(fn_worker, fn_answer, FN_CMDGetFNEndDate, NULL, 0, 300);
    FURI_LOG_D(TAG, "FN_CMDGetFNEndDate %d", trx_ok);
    *fn_error = fn_answer->error;
    status = check_data_len(fn_answer, 5);
    CHECK_STATUS(status)
    status = check_errors(fn_answer);
    CHECK_STATUS(status)
    lifeInfo->date_end.year = fn_answer->data[0];
    lifeInfo->date_end.mouth = fn_answer->data[1];
    lifeInfo->date_end.day = fn_answer->data[2];
    lifeInfo->reg_report_ctn_remaining = fn_answer->data[3];
    lifeInfo->reg_report_ctn = fn_answer->data[4];

    free_fn_answer_data(fn_answer);
    trx_ok = fn_trx(fn_worker, fn_answer, FN_CMDGetFNMemoryInfo, NULL, 0, 300);
    FURI_LOG_D(TAG, "FN_CMDGetFNMemoryInfo %d", trx_ok);
    *fn_error = fn_answer->error;
    status = check_data_len(fn_answer, 9);
    CHECK_STATUS(status)
    status = check_errors(fn_answer);
    CHECK_STATUS(status)
    uint8_t uint32tmp[4];
    for(int i = 0; i < 4; ++i) {
        uint32tmp[i] = fn_answer->data[i];
    }
    lifeInfo->five_year_data_resource = byte_array_to_uint32t_LE(uint32tmp);
    for(int i = 0; i < 4; ++i) {
        uint32tmp[i] = fn_answer->data[i+4];
    }
    lifeInfo->thirty_year_data_resource = byte_array_to_uint32t_LE(uint32tmp);
    lifeInfo->marking_notifications_resource = fn_answer->data[fn_answer->data_len];

    FuriHalRtcDateTime datetime;
    furi_hal_rtc_get_datetime(&datetime);
    uint8_t tx_datetime_param[5];
    tx_datetime_param[0] = (uint8_t)(datetime.year % 100);
    tx_datetime_param[1] = datetime.month;
    tx_datetime_param[2] = datetime.day;
    tx_datetime_param[3] = datetime.hour;
    tx_datetime_param[4] = datetime.minute;
    free_fn_answer_data(fn_answer);
    trx_ok = fn_trx(fn_worker, fn_answer, FN_CMDGetFNEndDays, tx_datetime_param, 5, 300);
    FURI_LOG_D(TAG, "FN_CMDGetFNEndDays %d", trx_ok);
    *fn_error = fn_answer->error;
    status = check_data_len(fn_answer, 2);
    CHECK_STATUS(status)
    status = check_errors(fn_answer);
    CHECK_STATUS(status)

    lifeInfo->days_to_end = two_uint8t_to_uint16t_BE(fn_answer->data);


    free_fn_answer(fn_answer);
    fn_uart_stop_thread(fn_worker->uart);
    return status;
}




