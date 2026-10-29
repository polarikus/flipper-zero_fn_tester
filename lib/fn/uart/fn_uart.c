//
// Created by Игорь Данилов on 28.02.2023.
//
#include "fn_uart_i.h"
#define TAG "FNUartBus"
#include <expansion/expansion.h>


typedef enum {
    UARTThreadEventStop = (1 << 0),
    UARTThreadEventRxStart = (1 << 1),
    UARTThreadEventRxDone = (1 << 2),
    UARTThreadEventAll = UARTThreadEventStop | UARTThreadEventRxStart | UARTThreadEventRxDone
} UARTThreadEvent;


typedef struct {
    FuriStreamBuffer* rx_stream;
    FuriThreadId thread_id;
    FuriHalSerialId serial_id;
    UARTApp* uart_app;
} UARTProcess;

static void usb_uart_on_irq_rx_dma_cb(
    FuriHalSerialHandle* handle,
    FuriHalSerialRxEvent ev,
    size_t size,
    void* context) {
    UARTProcess* uart_process = (UARTProcess*)context;

    if(uart_process->uart_app) {
        uart_process->uart_app->rx_events |= ev;
    }

    if(ev & (FuriHalSerialRxEventData | FuriHalSerialRxEventIdle)) {
        uint8_t data[FURI_HAL_SERIAL_DMA_BUFFER_SIZE] = {0};
        while(size) {
            size_t ret = furi_hal_serial_dma_rx(
                handle,
                data,
                (size > FURI_HAL_SERIAL_DMA_BUFFER_SIZE) ? FURI_HAL_SERIAL_DMA_BUFFER_SIZE : size);
            furi_stream_buffer_send(uart_process->rx_stream, data, ret, 0);
            size -= ret;
        };
        furi_thread_flags_set(furi_thread_get_id(uart_process->thread_id), UARTThreadEventRxDone);
    }
}

static void timer_timeout(void* ctx) {
    furi_check(ctx, TAG" timer_timeout | ctx");
    UARTProcess* app = ctx;
    FURI_LOG_D(TAG, "TIMEOUT_CB");
    furi_thread_flags_set(app->thread_id, UARTThreadEventRxDone);
}

// Служба expansion по умолчанию слушает USART (пины 13/14) для поиска модулей
// расширения. Если она уже заняла порт (подключён модуль или на линии 13/14
// есть активность, например перемычка), furi_hal_serial_control_acquire()
// вернёт NULL, и приложение упадёт по furi_check. Поэтому на время работы
// отключаем обнаружение модулей, а на выходе включаем его обратно.
static void fn_expansion_set_enabled(bool enabled) {
    Expansion* expansion = furi_record_open(RECORD_EXPANSION);
    if(enabled) {
        expansion_enable(expansion);
    } else {
        expansion_disable(expansion);
    }
    furi_record_close(RECORD_EXPANSION);
}

static void fn_init_uart(uint32_t baudrate, UARTApp* app) {
    FuriHalSerialId serial_id = FuriHalSerialIdUsart;
    furi_check(baudrate > 0, TAG" baudrate > 0");

    // Освобождаем USART от службы expansion до захвата порта.
    fn_expansion_set_enabled(false);

    furi_check(!app->serial_handle, TAG" !uart_process->serial_handle");
    app->serial_handle = furi_hal_serial_control_acquire(serial_id);
    furi_check(app->serial_handle, TAG" uart_process->serial_handle");

    furi_hal_serial_init(app->serial_handle, baudrate);
}

static void fn_deinit_uart(UARTApp* uart_app)
{

    furi_hal_serial_deinit(uart_app->serial_handle);
    furi_hal_serial_control_release(uart_app->serial_handle);
    uart_app->serial_handle = NULL;

    fn_expansion_set_enabled(true);
}

static int32_t uart_process(void* p) {
    UARTApp* uart_app = p;
    FURI_LOG_D(TAG, "START TASK");

    UARTProcess* app = malloc(sizeof(UARTProcess));

    app->thread_id = furi_thread_get_id(furi_thread_get_current());
    app->rx_stream = furi_stream_buffer_alloc(FN_UART_MAX_PACKAGE_LEN, 1);
    app->uart_app = uart_app;

    furi_hal_serial_dma_rx_start(uart_app->serial_handle,
                                 usb_uart_on_irq_rx_dma_cb, app, true);
    // Приёмник готов: только после этого можно отправлять запрос в ФН,
    // иначе быстрый ответ потеряется (DMA ещё не взведён).
    uart_app->rx_ready = true;

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
            furi_timer_start(timer, furi_ms_to_ticks(uart_app->timeout));
        }
        if(events & UARTThreadEventRxDone){
            furi_timer_stop(timer);
            data_len = furi_stream_buffer_receive(app->rx_stream, rx_buffer, rx_buffer_size, uart_app->timeout);
            uart_app->state = UARTModeRxDone;
            if(uart_app->rx_cmlt_cb) uart_app->rx_cmlt_cb(rx_buffer, data_len, uart_app->cb_ctx);
            furi_stream_buffer_reset(app->rx_stream);
            uart_app->state = UARTModeIdle;
        }
        //TODO Ускорить UART.Получаем первые 3 байта, там есть длина. Далее ждём, пока стрим не заполнится на эту длину + CRC. Если заполнился - объеденяем буфер
    }

    uart_app->rx_ready = false;
    furi_hal_serial_dma_rx_stop(uart_app->serial_handle);
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
    uart_app->serial_id = FuriHalSerialIdUsart;
    uart_app->rx_ready = false;
    uart_app->rx_events = 0;
    fn_init_uart(uart_app->baudrate, uart_app);

    uart_app->thread = furi_thread_alloc_ex("FN UART thread", 1 * 1024, uart_process, uart_app);
    uart_app->state = UARTModeIdle;
    furi_thread_set_priority(uart_app->thread, FuriThreadPriorityIsr);
    return uart_app;
}

void fn_uart_start_thread(UARTApp* app)
{
    furi_check(app, TAG" app null");
    furi_check(app->thread, TAG" app->thread");
    furi_check(furi_thread_get_state(app->thread) != FuriThreadStateRunning, TAG" Thread nr");
    furi_thread_start(app->thread);
}

void fn_uart_stop_thread(UARTApp* app)
{
    furi_check(app, TAG" fn_uart_stop_thread | app");
    furi_check(app->thread, TAG" fn_uart_stop_thread | app->thread");
    furi_thread_flags_set(furi_thread_get_id(app->thread), UARTThreadEventStop);
    furi_thread_join(app->thread);
    FURI_LOG_D(TAG, "uart thread stopped");
}

void fn_uart_app_free(UARTApp* app) {
    furi_check(app, TAG" fn_uart_app_free | app");
    furi_check(app->thread, TAG"fn_uart_app_free | app->thread");
    fn_deinit_uart(app);
    furi_thread_free(app->thread);
    free(app);
}

bool fn_uart_trx(UARTApp* app, uint8_t* tx_buff, size_t tx_buff_size, uint32_t timeout) {
    FURI_LOG_D(TAG, "START TRX");
    furi_check(app, TAG" fn_uart_trx | app");
    furi_check(tx_buff_size > 0, TAG"fn_uart_trx | tx_buff_size");
    furi_check(timeout > 0, TAG" fn_uart_trx | timeout");
    furi_check(app->serial_handle, TAG" fn_uart_trx | app->serial_handle");
    bool result = true;
    FURI_LOG_D(TAG, "tx_buff[3] = %x", tx_buff[3]);
    if(app->state != UARTModeIdle)
    {
        FURI_LOG_D(TAG, "!UARTModeIdle. MODE=%d", app->state);
        return false;
    }

    // Ждём, пока поток UART взведёт DMA на приём. Без этого быстрый ответ ФН
    // теряется: запрос уходит раньше, чем включается приёмник.
    uint32_t wait_started = furi_get_tick();
    while(!app->rx_ready) {
        if(furi_get_tick() - wait_started > furi_ms_to_ticks(100)) {
            FURI_LOG_E(TAG, "RX DMA is not ready, abort TRX");
            return false;
        }
        furi_delay_ms(1);
    }

    if(timeout != app->timeout) app->timeout = timeout;

    app->state = UARTModeTx;
    if(furi_thread_flags_set(furi_thread_get_id(app->thread), UARTThreadEventRxStart) != FuriStatusOk ) result = false;
    FURI_LOG_D(TAG, "Start tx");
    furi_hal_serial_tx(app->serial_handle, tx_buff, tx_buff_size);
    FURI_LOG_D(TAG, "Stop tx");
    app->state = UARTModeRx;
    FURI_LOG_D(TAG, "END TRX %d", result);

    return result;
}

void fn_uart_set_rx_callback(UARTApp* app, void (*rx_cb)(uint8_t* buf, size_t len, void* context), void* context) {
    furi_check(app, TAG" fn_uart_set_rx_callback | app ");
    app->rx_cmlt_cb = rx_cb;
    app->cb_ctx = context;
}
