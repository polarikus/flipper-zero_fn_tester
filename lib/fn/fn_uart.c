//
// Created by Игорь Данилов on 28.02.2023.
//
#include "fn_uart_i.h"
#define TAG "FNUartBus"


typedef enum {
    UARTThreadEventStop = (1 << 0),
    UARTThreadEventRxStart = (1 << 1),
    UARTThreadEventRxDone = (1 << 2),
    UARTThreadEventAll = UARTThreadEventStop | UARTThreadEventRxStart | UARTThreadEventRxDone
} UARTThreadEvent;


typedef struct {
    FuriStreamBuffer* rx_stream;
    FuriThreadId thread_id;
    FuriHalUartId uart_id;
    bool rx_cmlt;
} UARTProcess;

static void fn_uart_irq_cb(UartIrqEvent ev, uint8_t data, void* ctx) {
    UARTProcess* app = ctx;
    if(ev == UartIrqEventRXNE) {
        furi_stream_buffer_send(app->rx_stream, &data, 1, 0);
    }
    //Флаг либо есть, либо нет. Выставленный 22 раза флаг == выставленный 1 раз флаг.
}

static void timer_timeout(void* ctx) {
    furi_assert(ctx);
    UARTProcess* app = ctx;
    FURI_LOG_D(TAG, "TIMEOUT_CB");
    furi_thread_flags_set(app->thread_id, UARTThreadEventRxDone);
}

static void fn_init_uart(uint32_t baudrate, void (*cb)(UartIrqEvent ev, uint8_t data, void* ctx), void* ctx) {
    FuriHalUartId uart_id = FuriHalUartIdUSART1;
    furi_assert(baudrate > 0);
    furi_hal_uart_deinit(uart_id);
    furi_hal_uart_init(uart_id, baudrate);
    furi_hal_uart_set_irq_cb(uart_id, cb, ctx);
}

static int32_t uart_process(void* p) {
    UARTApp* uart_app = p;
    FURI_LOG_D(TAG, "START TASK");

    UARTProcess* app = malloc(sizeof(UARTProcess));
    fn_init_uart(uart_app->baudrate, fn_uart_irq_cb, app);

    app->thread_id = furi_thread_get_id(furi_thread_get_current());
    app->rx_stream = furi_stream_buffer_alloc(FN_UART_MAX_PACKAGE_LEN, 1);
    app->rx_cmlt = false;

    const uint16_t rx_buffer_size = FN_UART_MAX_PACKAGE_LEN;
    uint8_t* rx_buffer = malloc(rx_buffer_size);
    size_t data_len = 0;


    FuriTimer* timer = furi_timer_alloc(timer_timeout, FuriTimerTypePeriodic, app);

    uint32_t events;
    while(1) {
        events = furi_thread_flags_wait(UARTThreadEventAll, FuriFlagWaitAny, FuriWaitForever);
        //FURI_LOG_D(TAG, "FLAG %ld", events);
        if(events & UARTThreadEventStop) break;
        if(events & UARTThreadEventRxStart)
        {
            furi_timer_start(timer, pdMS_TO_TICKS(uart_app->timeout));
        }
        if(events & UARTThreadEventRxDone){
            furi_timer_stop(timer);
            data_len = furi_stream_buffer_receive(app->rx_stream, rx_buffer, rx_buffer_size, uart_app->timeout);
            uart_app->state = UARTModeRxDone;
            if(uart_app->rx_cmlt_cb) uart_app->rx_cmlt_cb(rx_buffer, data_len, uart_app);
            furi_stream_buffer_reset(app->rx_stream);
            uart_app->state = UARTModeIdle;
        }

    }

    free(rx_buffer);
    furi_timer_free(timer);
    furi_stream_buffer_free(app->rx_stream);
    free(app);
    FURI_LOG_D(TAG, "END TASK");
    return 0;
}

UARTApp* fn_uart_app_alloc() {
    FURI_LOG_D(TAG, "UART ALLOC");
    UARTApp* uart_app = malloc(sizeof(UARTApp));
    uart_app->timeout = 0;
    uart_app->baudrate = 115200;
    uart_app->uart_id = FuriHalUartIdUSART1;
    uart_app->thread = furi_thread_alloc_ex("FN UART thread", 2 * 1024, uart_process, uart_app);
    uart_app->state = UARTModeIdle;
    furi_thread_set_priority(uart_app->thread, FuriThreadPriorityHighest);
    return uart_app;
}

void fn_uart_app_free(UARTApp* uart_app) {
    furi_assert(uart_app);
    furi_assert(uart_app->thread);
    //furi_assert(furi_thread_get_current() != uart_app->thread);
    FURI_LOG_D(TAG, "UART FREE");
    furi_thread_flags_set(furi_thread_get_id(uart_app->thread), UARTThreadEventStop);
    FURI_LOG_D(TAG, "WAIT SPND");
    furi_thread_join(uart_app->thread);
    FURI_LOG_D(TAG, "WAIT SPNDED");
    furi_thread_free(uart_app->thread);
    free(uart_app);
}

bool fn_uart_trx(UARTApp* uart_app, uint8_t* tx_buff, size_t tx_buff_size, uint32_t timeout) {
    FURI_LOG_D(TAG, "START TRX");
    furi_assert(uart_app);
    furi_assert(tx_buff_size > 0);
    furi_assert(timeout > 0);
    bool result = true;
    if(uart_app->state != UARTModeIdle)
    {
        return false;
    }
    if(timeout != uart_app->timeout) uart_app->timeout = timeout;

    uart_app->state = UARTModeTx;
    if(furi_thread_flags_set(furi_thread_get_id(uart_app->thread), UARTThreadEventRxStart) != FuriStatusOk ) result = false;
    furi_hal_uart_tx(uart_app->uart_id, tx_buff, tx_buff_size);
    uart_app->state = UARTModeRx;
    FURI_LOG_D(TAG, "END TRX %d", result);

    return result;
}

void fn_uart_set_rx_callback(UARTApp* uart_app, void (*rx_cb)(uint8_t* buf, size_t len, void* context)) {
    furi_assert(uart_app);
    furi_thread_start(uart_app->thread);
    uart_app->rx_cmlt_cb = rx_cb;
}