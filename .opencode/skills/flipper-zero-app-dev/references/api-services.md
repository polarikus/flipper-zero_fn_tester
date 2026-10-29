# Сервисы и библиотеки

Все сервисы — records (`furi_record_open/close`), если не указано иное.
Полный список экспортируемых символов для FAP — `targets/f7/api_symbols.csv`.

## Storage (файловая система)

```c
#include <storage/storage.h>

Storage* storage = furi_record_open(RECORD_STORAGE);
```

Пути:

| Макрос | Куда ведёт |
|---|---|
| `APP_DATA_PATH("file")` | `/data/file` — приватный каталог приложения (создаётся сам) |
| `APP_ASSETS_PATH("file")` | `/assets/file` — файлы из `fap_file_assets` |
| `EXT_PATH("apps/...")` | SD-карта |
| `INT_PATH("...")` | внутренняя flash (осторожно, мало места) |
| `ANY_PATH("...")` | `/int` или `/ext` |

**Ловушка с каталогом приложения.** `APP_DATA_PATH("")` разворачивается в `/data/` —
с завершающим слэшем, и `storage_dir_open()`/`storage_dir_exists()` на таком пути
возвращают `FSE_INVALID_NAME` (проверено на 1.5.1). Обращение к *каталогу* приложения
пиши без слэша: `#define MY_APP_DIR STORAGE_APP_DATA_PATH_PREFIX` (публичный макрос
`"/data"`) или `APP_DATA_PATH(".")`-подобного шаблона не используй. Для файлов внутри
каталога `APP_DATA_PATH("file.tea")` работает как обычно.

Открытие/чтение/запись:

```c
File* file = storage_file_alloc(storage);
if(storage_file_open(file, APP_DATA_PATH("state.bin"), FSAM_READ_WRITE, FSOM_OPEN_ALWAYS)) {
    uint8_t buf[64];
    size_t n = storage_file_read(file, buf, sizeof(buf));
    storage_file_write(file, buf, n);
    storage_file_seek(file, 0, true);
    storage_file_truncate(file);
    storage_file_sync(file);
}
storage_file_close(file);     // ВСЕГДА, даже если open() вернул false
storage_file_free(file);
```

- Режимы: `FSAM_READ`, `FSAM_WRITE`, `FSAM_READ_WRITE`; `FSOM_OPEN_EXISTING`,
  `FSOM_OPEN_ALWAYS`, `FSOM_OPEN_APPEND`, `FSOM_CREATE_NEW`, `FSOM_CREATE_ALWAYS`.
- Ошибки: `storage_file_get_error(file)` + `storage_file_get_error_desc(file)`,
  `storage_error_get_desc(err)`.
- Каталоги: `storage_dir_open/read/close`, `storage_dir_exists`,
  `storage_common_mkdir/remove/rename/copy/stat/timestamp/fs_info`.
- `storage_file_exists`, `storage_dir_exists`, `storage_common_exists`,
  `storage_get_next_filename` — для уникальных имён.
- `storage_simply_mkdir`, `storage_simply_remove`, `storage_simply_remove_recursive` —
  удобные обёртки.
- События SD: `storage_get_pubsub(storage)` → `StorageEventTypeCardMount/Unmount/...`.

Потоки (удобнее для построчного чтения):

```c
#include <toolbox/stream/file_stream.h>
#include <toolbox/stream/buffered_file_stream.h>

Stream* stream = buffered_file_stream_alloc(storage);
if(buffered_file_stream_open(stream, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
    FuriString* line = furi_string_alloc();
    while(stream_read_line(stream, line)) {
        // ...
    }
    furi_string_free(line);
}
buffered_file_stream_close(stream);
stream_free(stream);
```

`buffered_file_stream_sync()` — сбросить буфер на диск (важно после записи).
`file_stream` — небуферизованный вариант, `string_stream`/`buffer_stream` — в памяти.

Сохранение структур (с магией и версией):

```c
#include <toolbox/saved_struct.h>
typedef struct { uint32_t magic; uint8_t version; int32_t value; } MyConfig;
bool saved_struct_save(path, &cfg, sizeof(cfg), MAGIC, VERSION);
bool saved_struct_load(path, &cfg, sizeof(cfg), MAGIC, VERSION);
```

## FlipperFormat (FFF)

Текстовый формат прошивки (используется в `.sub`, `.ir`, `.nfc`, `.rfid`):

```c
#include <flipper_format/flipper_format.h>

FlipperFormat* ff = flipper_format_file_alloc(storage);
if(flipper_format_file_open_existing(ff, path)) {
    FuriString* tmp = furi_string_alloc();
    uint32_t version = 0;
    if(!flipper_format_read_header(ff, tmp, &version)) { /* ... */ }
    flipper_format_read_uint32(ff, "Frequency", &freq, 1);
    flipper_format_read_string(ff, "Name", tmp);
    furi_string_free(tmp);
}
flipper_format_file_close(ff);
flipper_format_free(ff);
```

Функции: `read/write_header`, `read/write_uint32/int32/bool/float/hex/string`,
`read/write_hex_uint64`, `key_exist`, `get_value_count`, `rewind`, `seek_to_end`,
`delete_key`, `update_*`, `set_strict_mode`.

## Notification (вибро/звук/LED/подсветка)

```c
#include <notification/notification.h>
#include <notification/notification_messages.h>

NotificationApp* notif = furi_record_open(RECORD_NOTIFICATION);
notification_message(notif, &sequence_success);        // не блокирует
notification_message_block(notif, &sequence_double_vibro); // ждать окончания
notification_message(notif, &sequence_reset_rgb);
furi_record_close(RECORD_NOTIFICATION);
```

Готовые последовательности: `sequence_success`, `sequence_error`, `sequence_semi_success`,
`sequence_single_vibro`, `sequence_double_vibro`, `sequence_audiovisual_alert`,
`sequence_blink_*`, `sequence_set_*_255`, `sequence_reset_*`, `sequence_display_backlight_on/off`,
`sequence_empty`.

Своя последовательность — массив указателей на `NotificationMessage`:

```c
static const NotificationMessage msg_vibro_on = {
    .type = NotificationMessageTypeVibro,
    .data.vibro.on = true,
};
static const NotificationMessage msg_delay = {
    .type = NotificationMessageTypeDelay,
    .data.delay.length = 100,
};
static const NotificationSequence seq_my = { &msg_vibro_on, &msg_delay, NULL };
```

Всегда возвращай подсветку/цвет в исходное состояние (`sequence_reset_*`), иначе
приложение оставит устройство в «странном» виде.

## Power

```c
Power* power = furi_record_open(RECORD_POWER);
PowerInfo info;
power_get_info(power, &info);       // info.charge, is_charging, voltage_gauge, ...
power_reboot(power, PowerBootModeNormal);
power_off(power);
power_enable_otg(power, true);
furi_record_close(RECORD_POWER);
```

- `furi_hal_power_insomnia_enter()/exit()` — не давать устройству засыпать на время
  длительной операции (парные вызовы!).
- События батареи: `power_get_pubsub(power)` → `PowerEventTypeBatteryLevelChanged` и др.
- Проверка заряда перед тяжёлой работой: `furi_hal_power_get_pct()`,
  `furi_hal_power_is_charging()`.

## Input (сырые события)

ViewPort/View получают ввод сами; для глобального перехвата:

```c
FuriPubSub* input = furi_record_open(RECORD_INPUT_EVENTS);
FuriPubSubSubscription* sub = furi_pubsub_subscribe(input, input_cb, ctx);
...
furi_pubsub_unsubscribe(input, sub);
furi_record_close(RECORD_INPUT_EVENTS);
```

`input_cb(const InputEvent* event, void* ctx)`; `input_get_key_name()`,
`input_get_type_name()` — для логов.

## Dialogs

См. `references/api-gui.md` (сообщения и file browser). `DialogsApp` синхронный —
не вызывай из GUI-callback-а и не держи открытым во время работы.

## Loader (запуск других приложений)

```c
Loader* loader = furi_record_open(RECORD_LOADER);
loader_start_with_gui_error(loader, "nfc", NULL);       // по appid
loader_start(loader, "/ext/apps/Tools/xyz.fap", args, error);
loader_lock(loader); loader_unlock(loader);
furi_record_close(RECORD_LOADER);
```

`loader_signal()` — отправить сигнал текущему приложению (используется системой,
не для межпроцессного общения).

## Serial (USART/LPUART)

Актуальный API — `furi_hal_serial_*` (старый `furi_hal_uart_*` устарел).

```c
#include <furi_hal_serial.h>
#include <furi_hal_serial_control.h>

FuriHalSerialHandle* serial = furi_hal_serial_control_acquire(FuriHalSerialIdUsart);
furi_hal_serial_init(serial, 115200);
furi_hal_serial_tx(serial, data, len);
furi_hal_serial_async_rx_start(serial, rx_cb, context, false);
// rx_cb: furi_hal_serial_async_rx(handle) — забрать байт
furi_hal_serial_async_rx_stop(serial);
furi_hal_serial_deinit(serial);
furi_hal_serial_control_release(serial);
```

- `furi_hal_serial_control_is_busy(id)` — порт может быть занят CLI/GPIO-приложением.
- Для потока данных удобно: callback → `FuriStreamBuffer` → рабочий поток/EventLoop.
- Не вызывай `furi_hal_serial_*` из ISR, кроме разрешённых операций.
- Пины UART: см. `furi_hal_resources.h` (`gpio_usart_tx/rx`) и документацию GPIO.

## Expansion и внешние модули

```c
#include <expansion/expansion.h>
Expansion* exp = furi_record_open(RECORD_EXPANSION);
expansion_disable(exp);      // освободить GPIO/UART для своих нужд
...
expansion_enable(exp);
furi_record_close(RECORD_EXPANSION);
```

Модули (WiFi devboard и др.): `expansion_module_*` — смотри `applications/services/expansion`.

## USB / BLE

- USB HID: `furi_hal_usb_hid.h` — `furi_hal_usb_set_config(&usb_hid, NULL)`.
- USB CDC: `furi_hal_usb_cdc.h` — виртуальный COM-порт.
- BLE: высокоуровневый `Bt*` (`<bt/bt_service/bt.h>`) — `bt_disconnect()`,
  `bt_profile_start()`, `bt_profile_restore_default()`; низкоуровневый `furi_hal_bt.h` —
  GATT, advertising.
- Для BLE-приложений проверяй, что BT включён, и корректно отключайся при выходе.

## IR / Sub-1 GHz / NFC / RFID / iButton

Публичные библиотеки (`lib/...`, заголовки доступны в SDK):

- IR: `infrared_worker` (`<infrared/worker/infrared_worker.h>`), `infrared_signal`,
  `furi_hal_infrared`; приём/передача асинхронные, сигнал нужно освобождать.
- Sub-1 GHz: `subghz_tx_rx_worker`, `subghz_devices`, `furi_hal_subghz`; обязательно
  `subghz_tx_rx_worker_stop` и `furi_hal_subghz_sleep` при выходе.
- NFC: `nfc`/`nfc_scanner`/`mf_classic` и т.д.; следи за `nfc_release`/`nfc_scanner_free`.
- 125 кГц RFID: `lfrfid_worker`, `lfrfid_raw_worker`, `protocol_dict`.
- iButton: `ibutton_worker`, `one_wire` (`lib/one_wire`).
- ИК/SubGHz/NFC — «тяжёлые» сервисы: не держи дольше необходимого, иначе другие
  приложения не смогут ими пользоваться.

Проверяй фактическую доступность символов: если функции нет в `api_symbols.csv`,
она не попадёт в FAP.

## Плагины (flipper_application)

Для приложений, загружающих сторонние `.fal`/плагины:

```c
#include <flipper_application/flipper_application.h>
#include <loader/firmware_api/firmware_api.h>

FlipperApplication* app = flipper_application_alloc(storage, firmware_api_interface);
flipper_application_preload(app, APP_DATA_PATH("plugins/plugin.fal"));
flipper_application_map_to_memory(app);
const FlipperAppPluginDescriptor* desc = flipper_application_plugin_get_descriptor(app);
flipper_application_free(app);
```

- Плагин отдаёт `FlipperAppPluginDescriptor` через `get_api()` и версионирует API.
- Для встроенных плагинов: `apptype=FlipperAppType.PLUGIN`, `fal_embedded=True` у хоста.
- Документация: `documentation/AppsOnSDCard.md`, `applications/examples/example_plugins*`.

## Прочие полезные библиотеки toolbox

| Заголовок | Что даёт |
|---|---|
| `toolbox/path.h` | `path_extract_filename/extension/dirname`, `path_concat` |
| `toolbox/dir_walk.h` | рекурсивный обход каталогов с фильтром |
| `toolbox/stream/*` | потоки: файл, буфер, строка, кэш |
| `toolbox/hex.h` | hex-конвертация |
| `toolbox/crc32_calc.h` | CRC32 |
| `toolbox/md5_calc.h` | MD5 |
| `toolbox/version.h` | версии/сравнение |
| `toolbox/simple_array.h` | динамический массив структур с callback-ами |
| `toolbox/float_tools.h` | сравнение float |
| `toolbox/strint.h` | строка ↔ число |
| `toolbox/bit_buffer.h` | битовые буферы (протоколы) |
| `toolbox/pretty_format.h` | форматирование чисел/байт |
| `toolbox/property.h` | парсинг свойств |
| `toolbox/args.h` | разбор аргументов CLI |
| `lib/datetime/datetime.h` | дата/время, форматирование |
| `lib/print/print.h` | printf-подобный вывод в буфер/поток |
| `lib/mlib/m-*` | контейнеры (list, array, string, variant) |
