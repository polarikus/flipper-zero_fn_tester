//
// Created by Игорь Данилов on 28.02.2023.
//

#pragma once

#include "fn_uart.h"

typedef enum {
    UARTModeIdle,
    UARTModeTx,
    UARTModeRx,
    UARTModeRxDone
} UARTState;

struct UARTApp {
    //FuriStreamBuffer* rx_stream;
    FuriThread* thread;
    FuriHalUartId uart_id;
    uint32_t baudrate;
    uint32_t timeout;
    UARTState state;
    void (*rx_cmlt_cb)(uint8_t* buf, size_t len, void* context);
};
