//
// Created by Игорь Данилов on 28.02.2023.
//

#pragma once

#include "furi.h"
#include "furi_hal_uart.h"

#define FN_UART_MAX_PACKAGE_LEN 250

typedef struct UARTApp UARTApp;

UARTApp* fn_uart_app_alloc();
void fn_uart_app_free(UARTApp* uart_app);

/**
 * Задать коллбек, куда вернутся RX данные от ФН
 * @param uart_app
 * @param rx_cb указатель на функцию (коллбек)
 */
void fn_uart_set_rx_callback(
    UARTApp* uart_app,
    void (*rx_cb)(uint8_t* buf, size_t len, void* context)
    );

/**
 * Отправить данные в ФН,
 * @param uart_app
 * @param tx_buff Данные, передаваемые в ФН
 * @param tx_buff_size Размер данных
 * @param timeout Время, после которого будет вызван колдбек, заданный fn_uart_set_rx_callback. За это время RX буфер доджен быть заполнен нужными данными
 * @return Возвращает true если данные отправились и воркер готов вызвать коллбек после приёма
 */
bool fn_uart_trx(UARTApp* uart_app, uint8_t *tx_buff, size_t tx_buff_size, uint32_t timeout);

