//
// Created by Игорь Данилов on 28.02.2023.
//

#pragma once

#include "furi.h"
#include "furi_hal_serial.h"
#include <furi_hal_serial_control.h>
#include "fn_worker.h"

#define FN_UART_MAX_PACKAGE_LEN 250
#define FN_UART_MIN_PACKAGE_LEN 6

#define FN_UART_MSG_START_BYTE 0x04
#define FN_UART_MSG_START_SIZE 1
#define FN_UART_LEN_SIZE 2
#define FN_UART_CRC_SIZE 2
#define FN_UART_CMD_SIZE 1
#define FN_UART_ANSWER_CODE_SIZE 1

#define FN_UART_MSG_START_SHIFT 0
#define FN_UART_LEN_SHIFT FN_UART_MSG_START_SIZE
#define FN_UART_CMD_SHIFT (FN_UART_MSG_START_SIZE + FN_UART_LEN_SIZE)
#define FN_UART_ANSWER_CODE_SHIFT (FN_UART_MSG_START_SIZE + FN_UART_LEN_SIZE)
#define FN_UART_DATA_SHIFT 4

typedef struct UARTApp UARTApp;

UARTApp* fn_uart_app_alloc();

void fn_uart_app_free(UARTApp* app);

void fn_uart_start_thread(UARTApp* app);

void fn_uart_stop_thread(UARTApp* app);

/**
 * Задать коллбек, куда вернутся RX данные от ФН
 * @param app
 * @param rx_cb указатель на функцию (коллбек)
 */
void fn_uart_set_rx_callback(
    UARTApp* app,
    void (*rx_cb)(uint8_t* buf, size_t len, void* context),
    void* context
    );

/**
 * Отправить данные в ФН,
 * @param app
 * @param tx_buff Данные, передаваемые в ФН
 * @param tx_buff_size Размер данных
 * @param timeout Время, после которого будет вызван колдбек, заданный fn_uart_set_rx_callback. За это время RX буфер доджен быть заполнен нужными данными
 * @return Возвращает true если данные отправились и воркер готов вызвать коллбек после приёма
 */
bool fn_uart_trx(UARTApp* app, uint8_t *tx_buff, size_t tx_buff_size, uint32_t timeout);

