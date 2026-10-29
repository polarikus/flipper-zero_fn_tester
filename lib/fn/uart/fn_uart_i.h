//
// Created by Игорь Данилов on 28.02.2023.
//

#pragma once

#include "fn_uart.h"
#include "fn_worker.h"


typedef enum {
    UARTModeIdle,
    UARTModeTx,
    UARTModeRx,
    UARTModeRxDone
} UARTState;

struct UARTApp {
    //FuriStreamBuffer* rx_stream;
    FuriThread* thread;
    FuriHalSerialId serial_id;
    FuriHalSerialHandle* serial_handle;
    uint32_t baudrate;
    uint32_t timeout;
    UARTState state;
    /* RX DMA взведён и можно отправлять запрос (см. fn_uart_trx) */
    volatile bool rx_ready;
    /* Маска событий FuriHalSerialRxEvent, накопленная с последнего сброса (диагностика) */
    volatile uint32_t rx_events;
    void (*rx_cmlt_cb)(uint8_t* buf, size_t len, void* context);
    void* cb_ctx;
};
