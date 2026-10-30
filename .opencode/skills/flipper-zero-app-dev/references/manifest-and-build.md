# Манифест, сборка и отладка

## application.fam

Манифест — Python-фрагмент с вызовом `App(...)`. Обязательны `appid` и `apptype`.
`appid`: `^[a-z0-9_]+$`.

```python
App(
    appid="my_app",
    name="My App",
    apptype=FlipperAppType.EXTERNAL,
    entry_point="my_app_app",
    requires=["gui", "storage"],
    stack_size=2 * 1024,
    fap_version="1.0",
    fap_icon="my_app.png",
    fap_category="Tools",
    fap_description="Short description",
    fap_author="Author",
    fap_weburl="https://example.com",
    fap_icon_assets="images",
    fap_file_assets="files",
)
```

Основные поля:

| Поле | Значение |
|---|---|
| `appid` | id приложения (латиница, цифры, `_`) |
| `apptype` | `EXTERNAL` (обычный `.fap`), `MENUEXTERNAL` (`.fap` в главном меню), `PLUGIN` |
| `name` | имя в меню |
| `entry_point` | `int32_t fn(void* p)`; для C++ — `extern "C"` |
| `requires` | сервисы/приложения, нужные для запуска (`gui`, `storage`, `notification`, `bt`, ...) |
| `stack_size` | стек главного потока, байты; по умолчанию 2048 |
| `fap_version` | строка `"1.2"` или кортеж `(1, 2)`; **float запрещён** |
| `fap_icon` | PNG 10×10, 1-битный |
| `fap_category` | подкаталог в `/ext/apps/<Category>/`, влияет на меню |
| `fap_description`, `fap_author`, `fap_weburl` | метаданные |
| `fap_icon_assets` | каталог PNG → генерируется `<appid>_icons.h`, иконки `&I_name`, анимации `&A_name` |
| `fap_file_assets` | каталог произвольных файлов → доступны через `APP_ASSETS_PATH("...")` |
| `sources` | маски исходников, по умолчанию `["*.c*"]`; `"!dir"` исключает |
| `fap_libs` | доп. библиотеки из SDK (увеличивают размер `.fap`) |
| `fap_private_libs` | приватные библиотеки из `lib/<name>` (`Lib(name=..., sources=[...])`) |
| `fap_extbuild` | внешняя сборка (`ExtFile(path=..., command=...)`) |
| `targets` | список целей, по умолчанию `["all"]` |
| `conflicts`, `provides`, `order`, `cdefines` | зависимости, порядок, defines |

Один `.fam` может содержать несколько `App(...)` (например, приложение и его плагины).

## Ассеты

- Изображения: PNG 1-битные; кладутся в каталог из `fap_icon_assets`.
  Имена: `I_<name>.png` → `&I_name`, анимация — `A_<name>.png` (несколько кадров
  по конвенции `assets/ReadMe.md`).
- В коде: `#include "my_app_icons.h"` (имя = `appid`).
- Произвольные файлы (тексты, бинарники, шрифты): `fap_file_assets="files"` →
  `APP_ASSETS_PATH("sub/file.txt")`. Это **не** `fap_icon_assets`.
- Иконка в меню (`fap_icon`) не обязана совпадать с иконками внутри приложения.
- Ассеты встраиваются в `.fap`; не клади в них мегабайты без нужды.

## ufbt

```bash
ufbt                    # собрать приложение из текущего каталога → dist/<appid>.fap
ufbt launch             # собрать + загрузить по USB + запустить
ufbt faps               # собрать все найденные FAP
ufbt APPID=my_app       # явно выбрать приложение
ufbt UFBT_APP_DIR=/path/to/app
ufbt FORCE=1            # пересобрать принудительно
ufbt VERBOSE=1          # показать команды компиляции
```

Переменные окружения/параметры:

| Переменная | Назначение |
|---|---|
| `FLIP_PORT` | CDC-порт, если подключено несколько Flipper (`/dev/cu.usbmodemflip_*`) |
| `STRICT_FAP_IMPORT_CHECK` | строгая проверка импортируемых символов (по умолчанию `True`) |
| `SWD_TRANSPORT` | `auto`/`cmsis-dap`/`stlink`/`blackmagic_usb`/`blackmagic_wifi` |
| `SWD_TRANSPORT_SERIAL` | серийник программатора |
| `OPENOCD_OPTS` | доп. опции OpenOCD |
| `OTHER_ELF` | ELF для отладки |

SDK и прошивка:

```bash
ufbt update                      # обновить SDK/toolchain
ufbt update -c dev               # канал dev (также rc, release)
ufbt update -t flipperZero       # явная аппаратная цель
ufbt flash_usb                   # обновить прошивку по USB (self-update)
ufbt flash_usb_full              # полная переустановка прошивки
```

**Версия SDK ufbt должна соответствовать прошивке на устройстве.** Если при запуске
«API version mismatch» — обнови прошивку (`ufbt flash_usb`) или SDK (`ufbt update`).

## Разработка в IDE

```bash
ufbt vscode_dist     # подготовить конфигурацию VSCode
ufbt cdb             # compile_commands.json для clangd/CLion
eval $(ufbt -s env)  # окружение toolchain (arm-none-eabi-gcc, gdb и т.п.)
```

## Отладка

1. **Логи** — основной инструмент:

```bash
ufbt cli
# в CLI:
log info        # или log debug / log trace
log ?           # справка по уровням
```

`FURI_LOG_I(TAG, "...")` видно в `log info`; `FURI_LOG_D` — в `log debug`.
Остановка — `Ctrl+C`.

2. **Метрики на устройстве** в CLI: `free`, `free_blocks`, `top`, `info`,
   `device_info`, `sysctl`, `uptime`.

3. **SWD-отладка**:

```bash
ufbt flash       # прошить через программатор
ufbt debug       # GDB-сессия
```

Для отладки FAP нужна прошивка, собранная в том же типе (debug/release), и совпадающий
SDK. Точка останова в коде FAP ставится после загрузки `.fap` в RAM.
Подробнее: `documentation/app_dev/app_dev_debugging_faps.md`.

4. **Краши**: после падения приложение перезапускает меню; смотри лог (`log debug`),
   проверяй стек (`top` → Stack Min), `furi_check`-и, `view_dispatcher_remove_view`.

5. **Профилирование памяти**: `furi_thread_enable_heap_trace(thread)` +
   `memmgr_heap_get_thread_memory(furi_thread_get_id(thread))`.

## Частые проблемы сборки

| Симптом | Причина/решение |
|---|---|
| `missing manifest (application.fam)` | команда запущена не в каталоге приложения; `cd` или `UFBT_APP_DIR` |
| `Invalid appid` | appid не соответствует `[a-z0-9_]+` |
| `Duplicate app declaration` | одинаковый `appid` в двух манифестах |
| `Invalid fap_version` | версия float; используй `"1.0"` или `(1, 0)` |
| предупреждения о missing symbols | функция не экспортирована в API; замени на публичную или подключи библиотеку |
| приложение не видно в меню | проверь `fap_category`, `apptype`, файл `.fap` в `/ext/apps/...` |
| `.fap` не запускается, «API mismatch» | обнови прошивку (`ufbt flash_usb`) или SDK (`ufbt update`) |
| падение сразу после старта | неверный `entry_point`, маленький `stack_size`, обращение к NULL-модели |
| падение при выходе | забыт `view_dispatcher_remove_view`/`*_free`/`furi_record_close` |

## Структура типичного проекта

```
my_app/
├── application.fam
├── my_app.c / my_app.h / my_app_i.h
├── my_app_custom_event.h
├── scenes/
│   ├── my_app_scene.h
│   ├── my_app_scene_config.h
│   ├── my_app_scene_start.c
│   └── my_app_scene_about.c
├── views/                 # свои View (при необходимости)
├── images/                # fap_icon_assets
├── files/                 # fap_file_assets
├── lib/                   # fap_private_libs
└── dist/                  # результат сборки
```
