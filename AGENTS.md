# AGENTS.md — FN Tester (fn_test)

> **Обязательно:** при любой задаче по разработке, ревью, отладке или тестированию
> этого приложения сначала загружай скилл **`flipper-zero-app-dev`**
> (`.opencode/skills/flipper-zero-app-dev/SKILL.md`). В нём — актуальные
> соглашения по FAP, манифесту `application.fam`, Furi API, GUI/SceneManager,
> потокам, памяти и тестам на Flipper Zero. Если скилл недоступен, следуй
> правилам из этого файла.

## О проекте

Внешнее приложение (FAP) для **Flipper Zero**: тестер фискальных накопителей (ФН),
применяемых в кассовой технике (Россия). Приложение подключается к ФН по UART,
отправляет команды протокола ФН и показывает:

- информацию о ФН (серийный номер, фаза жизни, состояние смены, версия/тип ФФД,
  версия ПО, МГМ или боевой ФН);
- информацию о жизненном цикле (дата окончания, число и остаток регистраций,
  ресурс памяти);
- сброс МГМ (команда `60 16`).

| Параметр | Значение |
|---|---|
| `appid` | `fn_test` |
| Точка входа | `fn_test_app` (`fn_test_app.c`) |
| Тип | `FlipperAppType.EXTERNAL` |
| Категория | `GPIO` |
| Версия | `(0, 4)` — см. `application.fam` и `.catalog/CHANGELOG.md` |
| Стек приложения | `4 * 1024` |
| Приватная библиотека | `lib/fn` (`Lib(name="fn")`) |
| Иконки | `logo_fn.png` (иконка FAP), `icons/` (`fap_icon_assets`) |

Это **самостоятельный репозиторий приложения**, а не каталог дерева прошивки.
Корень репозитория — одновременно каталог приложения.

## Структура

```
flipper-zero_fn_tester/
├── application.fam            # манифест FAP
├── fn_test_app.c              # entry point, alloc/free, ViewDispatcher
├── fn_test_app_i.h            # структура FNApp, enum'ы режимов и view
├── fn_test_app.h              # публичный typedef FNApp
├── scenes/                    # сцены SceneManager
│   ├── fn_test_scene.c/.h     # handlers, регистрация сцен
│   ├── fn_test_scene_config.h # список сцен (макрос ADD_SCENE)
│   └── fn_test_scene_*.c      # start, fn_info, life_info, wiring, about,
│                              # detect, test, timeout, run_cmd
├── views/                     # кастомные View
│   ├── fn_test_view_detect.c/.h
│   └── fn_test_view_progress.c/.h
├── lib/fn/                    # приватная библиотека работы с ФН
│   ├── fn_worker.c/.h/_i.h    # воркер: поток + конечный автомат режимов
│   ├── fn_worker_modes.c      # обработчики режимов
│   ├── fn_commands.h          # коды команд протокола ФН
│   ├── fn_helpers.c/.h        # преобразования байтов/строк
│   ├── fn_tools.c/.h          # вспомогательные функции
│   ├── uart/                  # драйвер UART (HAL serial, DMA RX, таймауты)
│   ├── fn_objects/fn_info/    # парсинг и хранение данных ФН
│   ├── fn_objects/life_info/  # парсинг данных жизненного цикла
│   ├── errors/                # FNError: коды и строковые описания
│   └── data_types/            # общие типы, FFD-версии
├── unit_tests/                # юнит-тесты (minunit), работают на устройстве
├── .test_scripts/             # запуск тестов и ожидание Flipper по serial
├── icons/                     # ассеты fap_icon_assets
├── .catalog/                  # описание и CHANGELOG для каталога приложений
├── .github/ISSUE_TEMPLATE/    # шаблоны issue
├── .specify/                  # Spec Kit (шаблоны, конституция, скрипты)
└── .opencode/commands/        # слэш-команды /speckit.*
```

## Архитектура

Поток данных: **Scene → FNWorker → UART → парсер объекта ФН → Scene/View**.

- `fn_test_app_alloc()` открывает записи (`RECORD_GUI`, `RECORD_NOTIFICATION`,
  `RECORD_DIALOGS`), создаёт `ViewDispatcher`, `SceneManager`, `FNWorker`,
  view/модули и запускает первую сцену.
- Навигация — только через `SceneManager`; кастомные экраны — `views/`,
  стандартные — `submenu`, `widget`, `popup`.
- `FNWorker` (`lib/fn/fn_worker.c`) — отдельный поток (стек `10240`),
  получает режим (`FNWorkerMode*`) и общается с ФН через `lib/fn/uart`.
- UART (`lib/fn/uart/fn_uart.c`):
  - release: `FuriHalSerialIdUsart`, консоль VCP отключается на время работы;
  - `FURI_DEBUG`: `FuriHalSerialIdLpuart`, консоль остаётся доступной.
  - Скорость по умолчанию — `115200`, обмен пакетами с CRC; формат пакета
    и константы — в `fn_uart.h`.
- Результаты разбора складываются в `FNInfo` / `FNLifeInfo`
  (`lib/fn/fn_objects/`), ошибки — в `FNError` (`lib/fn/errors/`).
- Новые сцены добавляются в `scenes/fn_test_scene_config.h` (макрос `ADD_SCENE`),
  новые view — в `enum FNAppView` (`fn_test_app_i.h`) и в `view_dispatcher`.

## Сборка (только `ufbt`)

Проект собирается **исключительно** `ufbt` (uFBT). Не использовать `./fbt`,
`scons` и файлы сборки из дерева прошивки.

```bash
cd /Users/igordanilov/CLionProjects/flipper/flipper-zero_fn_tester

ufbt                  # собрать приложение из текущего каталога
ufbt APPID=fn_test    # явно указать appid
ufbt FORCE=1          # принудительная пересборка
ufbt VERBOSE=1        # полные команды компиляции/линковки
ufbt cdb              # compile_commands.json для CLion/clangd
ufbt format           # clang-format (.clang-format в корне)
ufbt lint             # статический анализ
ufbt update           # обновить SDK/toolchain (release / -c dev / -c rc)
```

Результат: `dist/fn_test.fap`. SDK и toolchain лежат в `~/.ufbt/current`
(локально установлен канал `release`).

## Запуск и логи

```bash
ufbt launch                                  # собрать, залить и запустить по USB
ufbt launch FLIP_PORT=/dev/cu.usbmodemflip_Rogour1   # если подключено несколько
ufbt cli                                     # CLI-сессия по USB
```

В CLI Flipper полезно: `log debug`, `log`, `free`, `top`, `device_info`, `help`.

## Тесты

Тесты живут на устройстве: FAP содержит minunit-тесты, а хост-скрипт запускает
их через CLI и разбирает отчёт.

### Состав

- `unit_tests/minunit.h`, `minunit_vars*.h` — фреймворк minunit (не менять без
  необходимости).
- `unit_tests/test_index.c` — реестр `unit_tests[]`, `fn_register_tests()` /
  `fn_unregister_tests()`: регистрируют CLI-команду `fn_test`.
- `unit_tests/lib_tests/…` — сами тесты (`MU_TEST`, `mu_assert*`):
  `fn_helpers`, `fn_info`, `fn_life_info`, `fn_uart`, `fn_worker`.
- `unit_tests/base_test_vars.h`, `fn_mgm_test_params.h` — тестовые константы
  (например, серийник/ПО МГМ).
- `.test_scripts/units.py` — прогон тестов на устройстве.
- `.test_scripts/await_flipper.py` — ожидание появления Flipper по serial.

### Тестовый профиль сборки

Тесты включаются отдельным профилем: `FN_TESTS=1`. Переменная читается в
`application.fam` и добавляет `-DFN_TESTS` в `cdefines`. В обычной сборке
(без переменной) команды `fn_test` нет — это ожидаемо, а не ошибка.

В тестовом профиле:

- `fn_register_tests(app->worker)` регистрирует CLI-команду `fn_test`;
- `fn_init_uart()` использует **USART (пины 13/14)**, как и в release;
- консоль VCP **не отключается**, иначе команду `fn_test` некуда отправить.

On-device тесты переиспользуют воркер и UART запущенного приложения
(`fn_test_get_worker()`), а не создают вторые: порт уже занят приложением.
Тесты, которым нужен реальный ФН (`fn_uart_trx_test`,
`fn_worker_fn_detect_start_test`, `fn_worker_fn_life_info_test`), при
отсутствии ответа печатают `[SKIP]` и не валят прогон.

### Запуск

```bash
# один раз: pyserial (Homebrew Python защищён PEP 668)
python3 -m pip install --user --break-system-packages pyserial

# собрать и залить именно тестовый профиль
FN_TESTS=1 ufbt launch FLIP_PORT=/dev/cu.usbmodemflip_Rogour1

# имя — суффикс из /dev/cu.usbmodemflip_<NAME>1
python3 .test_scripts/await_flipper.py Rogour
python3 .test_scripts/units.py Rogour

# можно прогнать только один набор
python3 .test_scripts/units.py Rogour fn_helpers
```

`units.py <name>` находит порт через `await_flipper.flp_serial_by_name()`,
шлёт в CLI `fn_test` и разбирает строки отчёта:

- `Failed tests: N`
- `Consumed: N ms`
- `Leaked: N`
- `Status: PASSED|FAILED`

Код возврата `0` — все тесты прошли, `1` — есть падения, команда не
зарегистрирована или отчёт не распознан. Скрипт печатает весь вывод CLI,
поэтому по логу видно, какой именно тест упал.

## Правила для агента

- **Перед началом работы загружать скилл `flipper-zero-app-dev`** — он основной
  источник рекомендаций по разработке FAP; при расхождении с этим файлом
  приоритет у явных требований пользователя и кода проекта.
- Сборка/запуск/отладка — только `ufbt`; не вызывать `./fbt` и не менять файлы
  прошивки. Все изменения — внутри этого репозитория.
- Язык документации, комментариев, логов и ответов — **русский**. Идентификаторы
  кода — английские.
- Соблюдать стиль проекта: `ufbt format`, `.clang-format`, `TAG` +
  `FURI_LOG_*`, `furi_check`/`furi_assert` для проверок, парные
  `malloc`/`free`.
- Память ограничена (STM32WB55: 256 КБ RAM): следить за `stack_size`,
  размерами буферов и утечками; проверять `free`/`top` в CLI.
- Не ломать release-сборку: тестовый код держать под `FURI_DEBUG` (или
  отдельным тестовым профилем сборки), а не включать в обычный FAP.
- При добавлении нового сервиса (`furi_record_open(RECORD_*)`) обновлять
  `requires` в `application.fam`.
- При изменении поведения/API обновлять `fap_version` в `application.fam`
  и `.catalog/CHANGELOG.md`.
- `.specify/`, `.opencode/commands/` — инструменты Spec Kit; не считать их
  частью кода приложения и не править без запроса.
