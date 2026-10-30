---
name: Flipper Zero App Development
description: Разработка и ревью приложений (FAP) для Flipper Zero — ufbt, манифест application.fam, Furi API, HAL, GUI (ViewPort, ViewDispatcher, ViewHolder, SceneManager, модули), работа с потоками и хранилищем, best practices, отладка и юнит-тесты. Использовать при любых задачах про прошивку Flipper Zero и внешние приложения.
---

# Разработка приложений для Flipper Zero

Этот скилл описывает, как писать внешние приложения (`.fap`) для Flipper Zero под
актуальную прошивку (ветка `dev` репозитория `flipperdevices/flipperzero-firmware`).

## Как пользоваться скиллом

1. Определи тип задачи и прочитай нужный reference (файлы рядом с `SKILL.md`):
   - API ядра Furi, потоки, таймеры, память — `references/api-furi.md`
   - GUI, экраны, view/сцены, рисование — `references/api-gui.md`
   - Сервисы (storage, notification, power, dialogs, serial, IR/SubGHz/NFC…) — `references/api-services.md`
   - Манифест, сборка ufbt, ассеты, отладка — `references/manifest-and-build.md`
   - Правила и антипаттерны — `references/best-practices.md`
   - Тесты — `references/testing.md`
   - Автотесты UI на устройстве по USB (RPC): снимки экрана, нажатия, перезапуск
     приложения, сохранность данных на SD — `references/rpc-ui-testing.md`
2. Возьми заготовку из `examples/`:
   - `examples/scene-app/` — канонический каркас приложения со сценами (копируй и переименовывай `my_app`)
   - `examples/view-port-app/` — минимальное приложение на одном ViewPort
   - `examples/host-test/` — юнит-тесты чистой логики на хосте
   - `examples/rpc-ui-test/` — снимки экрана и нажатия по USB для UI-сценариев
3. Собери и запусти: `ufbt launch` (см. ниже).

## Железо и ограничения (важно для решений)

- MCU: STM32WB55, Cortex-M4F, **256 КБ RAM**, 1 МБ flash, экран **128×64** (1 бит на пиксель).
- FAP загружается с SD-карты **в RAM целиком** (код + данные), поэтому доступная куча
  меньше, чем у встроенных приложений. Экономь память: `stack_size` 1–2 КБ на поток,
  крупные буферы — в куче, а не на стеке.
- Периферия: экран, кнопки, вибро, подсветка, динамик, ИК (приём/передача), Sub-1 GHz
  (CC1101), 125 кГц RFID, NFC (ST25R3916), iButton, GPIO, I2C, SPI, USART/LPUART, USB, BLE.
- Прошивка не «изолирует» сбой: `furi_check`/`furi_crash` аварийно завершают приложение
  (пользователь видит сообщение о падении). Это нормальный способ ловить нарушения инвариантов.

## Быстрый старт

```bash
mkdir -p applications_user/my_app && cd applications_user/my_app
ufbt create APPID=my_app      # только если каталог пустой
ufbt                          # сборка → dist/my_app.fap
ufbt launch                   # сборка + загрузка по USB + запуск
ufbt cli                      # CLI-сессия: log, log debug, top, free, storage ...
ufbt cdb                      # compile_commands.json для clangd/CLion
ufbt format                   # clang-format
ufbt lint                     # статический анализ
```

Сборка — **только `ufbt`**. Система сборки из корня репозитория прошивки (`./fbt`, `scons`)
для внешних приложений не используется. Все команды выполняются из каталога приложения.

## Структура приложения

```
my_app/
├── application.fam        # манифест: appid, entry_point, requires, stack_size, fap_*
├── my_app.c               # entry point + alloc/free/run
├── my_app.h               # публичные типы
├── my_app_i.h             # приватный заголовок: структура app, view id, инклюды
├── scenes/                # сцены: *_scene_config.h, *_scene.c
├── views/                 # собственные View (если нужны)
├── images/                # PNG для fap_icon_assets (генерируется my_app_icons.h)
├── files/                 # произвольные файлы для fap_file_assets (APP_ASSETS_PATH)
└── lib/                   # приватные библиотеки (fap_private_libs)
```

`appid` — только `[a-z0-9_]`; он же имя `.fap`. `entry_point` — имя C-функции
`int32_t fn(void* p)`, объявленное в манифесте отдельно.

## Выбор подхода к UI

| Сложность | Инструмент | Когда |
|---|---|---|
| Простое сообщение/подтверждение | `dialogs/dialogs.h` (`dialog_message_*`) | 1–2 экрана, нет своей логики отрисовки |
| Один экран, кастомная отрисовка/игра | `ViewPort` + `Canvas` | нужен полный контроль, 60 FPS не нужен |
| Один готовый модуль (TextBox, меню) | `ViewHolder` + модуль | один экран без навигации |
| Иерархия экранов | `ViewDispatcher` + `SceneManager` + модули/свои View | стандарт для серьёзных приложений |

Правило: **не смешивай** ViewDispatcher и ViewHolder на одном View; View из ViewDispatcher
нельзя одновременно держать в ViewStack/ViewHolder.

## Каркас приложения (сцены)

```c
// my_app.c
int32_t my_app_app(void* p) {
    UNUSED(p);
    MyApp* app = my_app_alloc();
    view_dispatcher_run(app->view_dispatcher);  // блокируется до view_dispatcher_stop()
    my_app_free(app);
    return 0;
}
```

`my_app_alloc()`:
1. `furi_record_open(RECORD_GUI)`.
2. `view_dispatcher_alloc()` + `scene_manager_alloc(&my_app_scene_handlers, app)`.
3. `view_dispatcher_set_event_callback_context`, `..._set_custom_event_callback`,
   `..._set_navigation_event_callback`, при необходимости `..._set_tick_event_callback`.
4. `view_dispatcher_attach_to_gui(..., ViewDispatcherTypeFullscreen)`
   (`ViewDispatcherTypeWindow` — со статус-баром).
5. Создать модули, `view_dispatcher_add_view(...)`.
6. `scene_manager_next_scene(app->scene_manager, MyAppSceneStart)`.

`my_app_free()` — строго обратный порядок: `view_dispatcher_remove_view()` для каждого view →
`view_dispatcher_free()` → `scene_manager_free()` → `*_free()` модулей → `furi_record_close()` → `free(app)`.

Callback-и:
```c
static bool my_app_custom_event_callback(void* context, uint32_t event) {
    MyApp* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}
static bool my_app_back_event_callback(void* context) {
    MyApp* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}
```
Если navigation-callback вернул `false`, ViewDispatcher останавливается и приложение
завершается — так реализуется выход по Back из корневой сцены.

## Золотые правила

1. **GUI-поток нельзя блокировать.** В draw/input/view-callback нельзя `furi_delay_ms`,
   ждать UART/NFC, делать долгий I/O. Долгая работа — в отдельном потоке или через
   `FuriEventLoop`.
2. **Модель и View разделены.** Данные — в модели View/приложения; доступ к модели из
   другого потока — через `with_view_model`/`view_commit_model` (locking model) или
   lock-free модель с атомарными полями.
3. **Парность ресурсов.** Каждый `furi_record_open` — `furi_record_close`; каждый
   `*_alloc` — `*_free`; каждый `add_view` — `remove_view` до `view_dispatcher_free`.
4. **Проверяй вход.** `furi_assert` для контекста в callback-ах, `furi_check` для
   инвариантов, которые не должны нарушаться никогда.
5. **Никаких больших объектов на стеке.** Кадровый буфер, буферы файлов, парсеры — в куче
   или статически; стек FAP-потока обычно 1–2 КБ, у рабочих потоков — по потребности.
6. **Освобождай всё при выходе**, в том числе при ошибке/отмене: пользователь может выйти
   в любой момент, а FAP выгружается из RAM.
7. **Файлы — только через `Storage`** и всегда `storage_file_close()`, даже если open
   провалился. Данные приложения — в `APP_DATA_PATH("...")`, ассеты — `APP_ASSETS_PATH`.
8. **Пиши в лог через `FURI_LOG_*` с коротким тегом** (`#define TAG "MyApp"`), не `printf`
   в продакшене. Логи смотри через `ufbt cli` → `log info`/`log debug`.
9. **Не держи аппаратный ресурс дольше нужного**: `furi_hal_serial_control_acquire` →
   `release`, IR/SubGHz/NFC — парные acquire/release и `*_reset`/`*_deinit`.
10. **Не используй внутренние символы прошивки**: только заголовки из SDK. Если функции
    нет в API — её не будет в таблице символов, FAP не запустится.
11. **Строки — `FuriString`** для динамики и форматирования; фиксированные короткие строки —
    `char buf[N]` с `snprintf`. Не используй `strcpy`/`sprintf` без нужды.
12. **Соблюдай стиль прошивки**: `furi_`-префиксы, `snake_case`, `PascalCase` для типов,
    `clang-format` (`ufbt format`), 4 пробела, комментарии по делу.
13. **Проверяй память на устройстве**: `ufbt cli` → `free`, `top`, а в коде —
    `memmgr_get_free_heap()`/`furi_thread_get_stack_space()` при отладке.

## Частые ошибки

- Забыли `view_dispatcher_remove_view` → падение при выходе.
- Сцена не обрабатывает `SceneManagerEventTypeBack` там, где это нужно, или
  navigation-callback всегда возвращает `true` → приложение не выходит по Back.
- Блокирующее ожидание в `on_enter`/draw → зависание UI.
- `free()` до `furi_record_close()` или наоборот → use-after-free/утечка.
- Файл не закрыт при ошибке open → утечка дескриптора и сбой SD.
- Каталог приложения открывают как `APP_DATA_PATH("")` → путь `/data/` со слэшем,
  `storage_dir_open()` отдаёт `FSE_INVALID_NAME`; нужен `STORAGE_APP_DATA_PATH_PREFIX`.
- Слишком маленький `stack_size` → stack overflow и краш (проверять `top`).
- Использование устаревшего API (`view_dispatcher_enable_queue`, `furi_hal_uart_*`):
  в актуальной прошивке очередь всегда включена, UART заменён на `furi_hal_serial_*`.
- Ассеты не подключены: нет `fap_icon_assets`/`fap_file_assets` в манифесте.
- `fap_version` задан числом с плавающей точкой — запрещено, только строка `"1.0"` или кортеж.

## Проверка перед сдачей

- [ ] `ufbt` собирается без warning-ов (и без `-Werror`-падений).
- [ ] `ufbt launch` запускает приложение, Back корректно выходит.
- [ ] Нет утечек: после выхода `free` в CLI показывает исходный объём.
- [ ] Все ветки ошибок освобождают ресурсы.
- [ ] Логи помогают диагностике: ключевые переходы и ошибки логируются.
- [ ] Для чистой логики есть тесты (`references/testing.md`).
- [ ] UI-сценарии проверены на устройстве по RPC-снимкам (`references/rpc-ui-testing.md`),
      причём разбор кадра верный: **мелкий шрифт читается**. Если вместо букв «полоски»,
      а иконки выглядят нормально — виноват скрипт, а не прошивка (`§3.1`).
- [ ] Данные пользователя в `apps_data/<appid>` сохранены перед UI-прогоном и возвращены
      после с побайтной сверкой (`§7.2`); прогон начинался с перезапуска приложения (`§7.1`).
- [ ] UI помещается в 128×64, длинные строки обрезаются/скроллятся, кириллица
      отображается только если подключён шрифт с нужными глифами.

## Локальные примеры в этом репозитории

В `applications_user/` лежат рабочие приложения — полезны как примеры структуры,
но их API может быть устаревшим (проверяй по `references/`):

| Приложение | Чему учит |
|---|---|
| `UART_Terminal` | сцены + `variable_item_list` + UART-воркер (использует старый `furi_hal_uart`) |
| `unitemp-flipperzero` | собственные `View` с моделью, работа с датчиками |
| `bc_scanner_emulate` | сцены, фоновый поток, обмен с UART |
| `flipper-pc-monitor` | `ViewPort` + BLE-сервис |
| `fn_test` | воркер, UART, кастомные экраны прогресса |

## Полезные ссылки (актуальный upstream)

- Репозиторий: https://github.com/flipperdevices/flipperzero-firmware (ветка `dev`)
- Документация: `documentation/app_dev/` (`your_first_app_in_c`, `gui`, `furi_api`,
  `debugging_faps`), `documentation/AppManifests.md`, `documentation/UnitTests.md`
- Примеры: `applications/examples/` (`example_view_dispatcher`, `example_view_holder`,
  `example_event_loop`, `example_apps_assets`, `example_plugins`)
- Каталог приложений: https://lab.flipper.net/apps
