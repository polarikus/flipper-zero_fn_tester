# Furi API и HAL

Всё ядро доступно через `#include <furi.h>` (он же тянет `furi_hal_gpio.h` и `math.h`).
Аппаратные абстракции — через `#include <furi_hal.h>`.

## Базовые макросы и проверки

```c
UNUSED(x);                 // подавить warning о неиспользуемом параметре
COUNT_OF(array);           // размер массива
MIN(a, b); MAX(a, b);      // из common_defines.h
furi_assert(cond);         // только debug-сборка: проверка + crash
furi_check(cond);          // всегда: при нарушении — crash приложения
furi_crash("msg");         // аварийно завершить приложение
furi_halt("msg");          // остановить систему
```

- `furi_assert` уместен в callback-ах (`furi_assert(context)`), `furi_check` — для
  инвариантов, нарушение которых означает баг.
- `FURI_CRITICAL_ENTER()/FURI_CRITICAL_EXIT()` — короткие критические секции
  (не более десятков инструкций, без вызовов, способных блокироваться).

Логи:

```c
#define TAG "MyApp"
FURI_LOG_E(TAG, "error %d", code);
FURI_LOG_W(TAG, "warn");
FURI_LOG_I(TAG, "info");
FURI_LOG_D(TAG, "debug");
FURI_LOG_T(TAG, "trace");
```

Уровень по умолчанию — info; на устройстве: `log debug` / `log trace` в CLI.
`FURI_LOG_*` использует `printf`-форматирование, следи за размером строки и стоимостью
вызовов в горячем пути.

Статусы и таймауты:

```c
typedef enum { FuriStatusOk, FuriStatusError, FuriStatusErrorTimeout,
               FuriStatusErrorResource, FuriStatusErrorParameter,
               FuriStatusErrorISR } FuriStatus;
// FuriWaitForever = 0xFFFFFFFF, FuriFlagWaitAny = 0
```

## Records (сервисы)

Сервисы — синглтоны, доступ по имени через `furi_record_open/close`:

```c
Gui* gui = furi_record_open(RECORD_GUI);
...
furi_record_close(RECORD_GUI);
```

| Record | Заголовок | Назначение |
|---|---|---|
| `RECORD_GUI` | `<gui/gui.h>` | экран, ViewPort-стек |
| `RECORD_STORAGE` | `<storage/storage.h>` | SD/внутренняя ФС |
| `RECORD_NOTIFICATION` | `<notification/notification.h>` | вибро, звук, LED, подсветка |
| `RECORD_DIALOGS` | `<dialogs/dialogs.h>` | системные диалоги и file browser |
| `RECORD_INPUT_EVENTS` | `<input/input.h>` | pubsub событий кнопок |
| `RECORD_POWER` | `<power/power_service/power.h>` | батарея, OTG, выключение |
| `RECORD_LOADER` | `<loader/loader.h>` | запуск других приложений |
| `RECORD_BT` | `<bt/bt_service/bt.h>` | Bluetooth |
| `RECORD_EXPANSION` | `<expansion/expansion.h>` | внешние модули |

Открывай только нужное и закрывай в обратном порядке. Record нельзя «держать» после
выхода из приложения.

## Потоки

```c
FuriThread* furi_thread_alloc_ex(const char* name, uint32_t stack_size,
                                 FuriThreadCallback cb, void* context);
void furi_thread_set_priority(FuriThread* t, FuriThreadPriority p);
void furi_thread_start(FuriThread* t);
bool furi_thread_join(FuriThread* t);    // дождаться завершения
void furi_thread_free(FuriThread* t);
```

Приоритеты: `Idle 0`, `Init 4`, `Lowest 14`, `Low 15`, `Normal 16` (по умолчанию),
`High 17`, `Highest 18`, `Isr 31` (deferred ISR, максимальный).

Паттерн корректной остановки:

```c
typedef struct {
    FuriThread* thread;
    volatile bool running;
} Worker;

static int32_t worker_thread(void* context) {
    Worker* w = context;
    while(w->running) {
        uint32_t flags = furi_thread_flags_wait(WORKER_STOP, FuriFlagWaitAny, 100);
        if(flags & WORKER_STOP) break;
        // ... полезная работа ...
    }
    return 0;
}

void worker_stop(Worker* w) {
    w->running = false;
    furi_thread_flags_set(furi_thread_get_id(w->thread), WORKER_STOP);
    furi_thread_join(w->thread);
}
```

- Поток должен сам завершаться; `furi_thread_free` не убивает работающий поток.
- `furi_thread_set_appid()` и `furi_thread_set_name()` помогают в `top`.
- Стек: 1–2 КБ для простого потока, больше при глубокой рекурсии/больших локальных
  буферах. Следи за `top` → `Stack Min`.
- `furi_thread_get_stack_space(furi_thread_get_current_id())` — свободный стек.

## FuriEventLoop (рекомендуемый способ ждать события)

```c
FuriEventLoop* loop = furi_event_loop_alloc();

FuriMessageQueue* queue = furi_message_queue_alloc(8, sizeof(MyEvent));
furi_event_loop_subscribe_message_queue(
    loop, queue, FuriEventLoopEventIn, my_queue_callback, app);

FuriEventLoopTimer* timer = furi_event_loop_timer_alloc(
    loop, my_tick_callback, FuriEventLoopTimerTypePeriodic, app);
furi_event_loop_timer_start(timer, 100);

furi_event_loop_run(loop);     // блокируется до furi_event_loop_stop(loop)

furi_event_loop_timer_free(timer);
furi_event_loop_unsubscribe(loop, queue);
furi_message_queue_free(queue);
furi_event_loop_free(loop);
```

- События: `FuriEventLoopEventIn`, `FuriEventLoopEventOut`, флаги
  `FuriEventLoopEventFlagEdge`, `FuriEventLoopEventFlagOnce`.
- Поддерживаются `FuriMessageQueue`, `FuriStreamBuffer`, `FuriSemaphore`, `FuriMutex`,
  `FuriEventFlag`, а также thread flags (`furi_event_loop_subscribe_thread_flags`).
- `furi_event_loop_pend_callback(loop, cb, ctx)` — выполнить callback в потоке цикла
  (аналог `furi_timer_pending_callback`).
- Callback-и выполняются в потоке, который вызвал `furi_event_loop_run`; не блокируй его.
- `ViewDispatcher` внутри использует собственный `FuriEventLoop`;
  `view_dispatcher_get_event_loop()` / `view_dispatcher_alloc_ex(loop)` — для интеграции.

## Таймеры (независимо от EventLoop)

```c
FuriTimer* t = furi_timer_alloc(cb, FuriTimerTypePeriodic, context); // или FuriTimerTypeOnce
furi_timer_start(t, furi_ms_to_ticks(100));
furi_timer_stop(t);
furi_timer_free(t);
furi_timer_pending_callback(cb, context, 0);  // отложенный вызов из timer-потока
```

Callback выполняется в отдельном timer-потоке; долгая работа в нём ломает другие таймеры.

## Примитивы синхронизации

```c
FuriMutex* m = furi_mutex_alloc(FuriMutexTypeNormal); // или FuriMutexTypeRecursive
furi_mutex_acquire(m, FuriWaitForever);
furi_mutex_release(m);
furi_mutex_free(m);

FuriSemaphore* s = furi_semaphore_alloc(max_count, initial_count);
furi_semaphore_acquire(s, FuriWaitForever);
furi_semaphore_release(s);

FuriEventFlag* f = furi_event_flag_alloc();
furi_event_flag_set(f, FLAG_A);
furi_event_flag_wait(f, FLAG_A | FLAG_B, FuriFlagWaitAny, FuriWaitForever);

FuriMessageQueue* q = furi_message_queue_alloc(16, sizeof(Msg));
furi_message_queue_put(q, &msg, FuriWaitForever);
furi_message_queue_get(q, &msg, 100);

FuriStreamBuffer* sb = furi_stream_buffer_alloc(1024, 1);
furi_stream_buffer_send(sb, data, len, FuriWaitForever);
furi_stream_buffer_receive(sb, buf, len, 100);
```

- В ISR используй таймаут `0` и только ISR-безопасные вызовы.
- Все таймауты (`..._get/acquire/wait`) и интервалы таймеров задаются **в тиках**:
  `furi_ms_to_ticks(ms)`; `FuriWaitForever` — ждать бесконечно.
- `api_lock.h` — простой «замок» на `FuriEventFlag` для ожидания выхода из приложения
  (`api_lock_alloc_locked`, `api_lock_wait_unlock_and_free`, `api_lock_unlock`).

## Строки `FuriString`

```c
FuriString* s = furi_string_alloc();
furi_string_printf(s, "value=%d", 42);
furi_string_cat_str(s, "\n");
const char* c = furi_string_get_cstr(s);   // валидно, пока строка жива
furi_string_free(s);
```

Основное: `alloc_set_str`, `alloc_printf`, `reserve`, `reset`, `size`, `empty`,
`get_cstr`, `set_str`, `printf`, `cat_printf`, `cmp`, `search_char`, `replace_all_str`,
`start_with_str`, `end_with_str`, `trim`, `left/right/mid`, `utf8_length`.
`get_cstr` возвращает указатель на внутренний буфер — не сохраняй его после изменения
или освобождения строки.

## PubSub

```c
FuriPubSub* pubsub = storage_get_pubsub(storage);
FuriPubSubSubscription* sub = furi_pubsub_subscribe(pubsub, my_cb, context);
...
furi_pubsub_unsubscribe(pubsub, sub);
```

`my_cb(const void* message, void* context)` вызывается в потоке издателя — не блокируй.
События: `StorageEvent` (`CardMount`, `CardUnmount`, ...), `PowerEvent`
(`BatteryLevelChanged`, `Charging`, ...).

## Память

- `malloc/free/calloc/realloc`, `aligned_alloc/aligned_free` (для DMA).
- `memmgr_get_free_heap()`, `memmgr_get_minimum_free_heap()`, `memmgr_heap_get_max_free_block()`
  — диагностика.
- `furi_thread_enable_heap_trace()` + `memmgr_heap_get_thread_memory(id)` — кто ест кучу.
- Фрагментация важна: избегай множества мелких разноразмерных alloc/free в цикле,
  переиспользуй буферы, предпочитай `FuriString` с `reserve` или фиксированные массивы.

## Kernel и задержки

```c
uint32_t furi_get_tick(void);            // тики
uint32_t furi_kernel_get_tick_frequency(void);
furi_delay_ms(10);                       // только в своем потоке, не в GUI/ISR
furi_delay_us(100);
furi_delay_until_tick(tick);
furi_kernel_lock()/unlock();             // осторожно, только для коротких секций
```

В GUI-callback-ах и ISR задержки запрещены.

## HAL: что где искать

| Область | Заголовок | Ключевые вызовы |
|---|---|---|
| GPIO | `furi_hal_gpio.h` | `furi_hal_gpio_init/init_simple`, `write`, `read`, `add_int_callback` |
| I2C | `furi_hal_i2c.h` | `furi_hal_i2c_acquire/release`, `tx/rx`, `is_device_ready` |
| SPI | `furi_hal_spi.h` | `furi_hal_spi_acquire/release`, `tx/rx`, `config` |
| ADC | `furi_hal_adc.h` | `furi_hal_adc_acquire`, `read`, `release` |
| USART/LPUART | `furi_hal_serial.h`, `furi_hal_serial_control.h` | `*_control_acquire`, `serial_init`, `serial_tx`, `serial_async_rx_start`, `*_release` |
| Динамик | `furi_hal_speaker.h` | `furi_hal_speaker_acquire/release`, `start/stop`, `set_volume` |
| Вибро | `furi_hal_vibro.h` | `furi_hal_vibro_on/off` |
| Подсветка | `furi_hal_light.h` | `furi_hal_light_set`, `furi_hal_light_blink_start/stop` |
| Питание | `furi_hal_power.h` | `get_pct`, `is_charging`, `insomnia_enter/exit`, `enable_otg` |
| Random | `furi_hal_random.h` | `furi_hal_random_get`, `furi_hal_random_fill_buf` |
| IR | `furi_hal_infrared.h`, `lib/infrared` | `furi_hal_infrared_async_tx/rx_*` |
| Sub-1 GHz | `furi_hal_subghz.h`, `lib/subghz` | `furi_hal_subghz_*`, `subghz_tx_rx_worker` |
| NFC | `furi_hal_nfc.h`, `lib/nfc` | `furi_hal_nfc_*`, `nfc_*` |
| 125 кГц RFID | `furi_hal_rfid.h`, `lib/lfrfid` | `furi_hal_rfid_*`, `lfrfid_*` |
| iButton | `furi_hal_ibutton.h`, `lib/ibutton` | `furi_hal_ibutton_*`, `ibutton_*` |
| USB | `furi_hal_usb.h`, `furi_hal_usb_hid.h`, `furi_hal_usb_cdc.h` | `furi_hal_usb_set_config`, HID/CDC |
| BLE | `furi_hal_bt.h`, `lib/bt` | `furi_hal_bt_*`, `bt_*` |
| RTC | `furi_hal_rtc.h` | `furi_hal_rtc_get_datetime/set_datetime` |
| Region | `furi_hal_region.h` | `furi_hal_region_get` |
| Expansion | `<expansion/expansion.h>` | `expansion_disable/enable`, модули |

Правило: перед использованием периферии — `acquire`, после — `release`/`deinit`.
Проверяй `furi_hal_*_is_busy`/возвращаемые значения и корректно реагируй на занятость
(например, UART занят CLI или GPIO-приложением).

## Внешние символы (важно)

FAP линкуется только с символами из таблицы API прошивки
(`targets/<target>/api_symbols.csv`). Внутренние функции и структуры недоступны.
Если нужной функции нет — используй публичный API, дублируй логику у себя или
подключай библиотеку через `fap_libs` / `fap_private_libs`.
