//
// Created by Игорь Данилов on 28.02.2023.
//
#include "fn_uart_i.h"
#include "furi_hal_console.h"
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
} UARTProcess;

static void fn_uart_irq_cb(UartIrqEvent ev, uint8_t data, void* ctx) {
    UARTProcess* app = ctx;
    if(ev == UartIrqEventRXNE) {
        furi_stream_buffer_send(app->rx_stream, &data, 1, 0);
    }
    //Флаг либо есть, либо нет. Выставленный 22 раза флаг == выставленный 1 раз флаг.
}

static void timer_timeout(void* ctx) {
    furi_check(ctx);
    UARTProcess* app = ctx;
    FURI_LOG_D(TAG, "TIMEOUT_CB");
    furi_thread_flags_set(app->thread_id, UARTThreadEventRxDone);
}

static void fn_init_uart(uint32_t baudrate, void (*cb)(UartIrqEvent ev, uint8_t data, void* ctx), void* ctx) {
#ifdef FURI_DEBUG
    FuriHalUartId uart_id = FuriHalUartIdLPUART1;
#else
    FuriHalUartId uart_id = FuriHalUartIdUSART1;
    furi_hal_console_disable();
#endif
    furi_check(baudrate > 0);

    furi_hal_uart_deinit(uart_id);
    furi_hal_uart_init(uart_id, baudrate);
    furi_hal_uart_set_irq_cb(uart_id, cb, ctx);
}

static void fn_deinit_uart()
{
#ifdef FURI_DEBUG
    FuriHalUartId uart_id = FuriHalUartIdLPUART1;
#else
    FuriHalUartId uart_id = FuriHalUartIdUSART1;
    furi_hal_console_enable();
#endif
    furi_hal_uart_deinit(uart_id);
}

static int32_t uart_process(void* p) {
    UARTApp* uart_app = p;
    FURI_LOG_D(TAG, "START TASK");

    UARTProcess* app = malloc(sizeof(UARTProcess));
    fn_init_uart(uart_app->baudrate, fn_uart_irq_cb, app);

    app->thread_id = furi_thread_get_id(furi_thread_get_current());
    app->rx_stream = furi_stream_buffer_alloc(FN_UART_MAX_PACKAGE_LEN, 1);

    const uint16_t rx_buffer_size = FN_UART_MAX_PACKAGE_LEN;
    uint8_t* rx_buffer = malloc(rx_buffer_size);
    size_t data_len = 0;


    FuriTimer* timer = furi_timer_alloc(timer_timeout, FuriTimerTypePeriodic, app);
    FuriLogLevel old_log_level = furi_log_get_level();//Сохраняем текущие настройки логов
    uint32_t events;
    while(1) {
        events = furi_thread_flags_wait(UARTThreadEventAll, FuriFlagWaitAny, FuriWaitForever);
        //FURI_LOG_D(TAG, "FLAG %ld", events);
        if(events & UARTThreadEventStop) break;
        if(events & UARTThreadEventRxStart)
        {
            furi_log_set_level(FuriLogLevelNone);//Из-за логов теряются байты, поэтому отключаем
            furi_timer_start(timer, furi_ms_to_ticks(uart_app->timeout));
        }
        if(events & UARTThreadEventRxDone){
            furi_log_set_level(old_log_level);//Возвращаем логи
            furi_timer_stop(timer);
            data_len = furi_stream_buffer_receive(app->rx_stream, rx_buffer, rx_buffer_size, uart_app->timeout);
            uart_app->state = UARTModeRxDone;
            if(uart_app->rx_cmlt_cb) uart_app->rx_cmlt_cb(rx_buffer, data_len, uart_app->cb_ctx);
            furi_stream_buffer_reset(app->rx_stream);
            uart_app->state = UARTModeIdle;
        }
        //TODO Ускорить UART.Получаем первые 3 байта, там есть длина. Далее ждём, пока стрим не заполнится на эту длину + CRC. Если заполнился - объеденяем буфер
    }

    fn_deinit_uart();
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
#ifndef FURI_DEBUG
    uart_app->uart_id = FuriHalUartIdUSART1;
#else
    uart_app->uart_id = FuriHalUartIdLPUART1;
#endif
    uart_app->thread = furi_thread_alloc_ex("FN UART thread", 1 * 1024, uart_process, uart_app);
    uart_app->state = UARTModeIdle;
    furi_thread_set_priority(uart_app->thread, FuriThreadPriorityIsr);
    return uart_app;
}

void fn_uart_start_thread(UARTApp* app)
{
    furi_check(app);
    furi_check(app->thread);
    furi_check(furi_thread_get_state(app->thread) != FuriThreadStateRunning);
    furi_thread_start(app->thread);
}

void fn_uart_stop_thread(UARTApp* app)
{
    furi_check(app);
    furi_check(app->thread);
    furi_thread_flags_set(furi_thread_get_id(app->thread), UARTThreadEventStop);
    furi_thread_join(app->thread);
    FURI_LOG_D(TAG, "uart thread stopped");
}

void fn_uart_app_free(UARTApp* app) {
    furi_check(app);
    furi_check(app->thread);
    furi_thread_free(app->thread);
    free(app);
}

bool fn_uart_trx(UARTApp* app, uint8_t* tx_buff, size_t tx_buff_size, uint32_t timeout) {
    FURI_LOG_D(TAG, "START TRX");
    furi_check(app);
    furi_check(tx_buff_size > 0);
    furi_check(timeout > 0);
    bool result = true;
    FURI_LOG_D(TAG, "tx_buff[3] = %x", tx_buff[3]);
    if(app->state != UARTModeIdle)
    {
        FURI_LOG_D(TAG, "!UARTModeIdle. MODE=%d", app->state);
        return false;
    }
    if(timeout != app->timeout) app->timeout = timeout;

    app->state = UARTModeTx;
    if(furi_thread_flags_set(furi_thread_get_id(app->thread), UARTThreadEventRxStart) != FuriStatusOk ) result = false;
    furi_hal_uart_tx(app->uart_id, tx_buff, tx_buff_size);
    app->state = UARTModeRx;
    FURI_LOG_D(TAG, "END TRX %d", result);

    return result;
}

void fn_uart_set_rx_callback(UARTApp* app, void (*rx_cb)(uint8_t* buf, size_t len, void* context), void* context) {
    furi_check(app);
    app->rx_cmlt_cb = rx_cb;
    app->cb_ctx = context;
}
