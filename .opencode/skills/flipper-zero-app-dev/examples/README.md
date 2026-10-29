# Примеры

Все примеры собраны и проверены через `ufbt` (SDK API 87–88.x; актуальная версия —
в `targets/f7/api_symbols.csv` ветки `dev`).

| Каталог | Что показывает |
|---|---|
| `scene-app/` | Каркас приложения: `SceneManager` + `ViewDispatcher` + модули `submenu`/`widget`, навигация, выход по Back |
| `view-port-app/` | Один `ViewPort`: рисование на `Canvas`, ввод, `FuriEventLoop` + периодический таймер |
| `host-test/` | Юнит-тесты чистой логики на хосте (`make test`, ASan/UBSan) |

## Как использовать `scene-app`

1. Скопируй каталог в `applications_user/<твой_проект>`.
2. Переименуй `my_app` → свой `appid`:
   - `application.fam`: `appid`, `entry_point`, `name`, `fap_icon`, `fap_*`;
   - имена файлов `my_app.c`, `my_app.h`, `my_app_i.h`, `scenes/my_app_*`;
   - префиксы функций/типов (`MyApp`, `my_app_`, `MyAppScene`, `MyAppView`).
3. Добавь иконку `my_app.png` (10×10, 1-битная) или убери `fap_icon`.
4. `ufbt launch`.

## Как использовать `host-test`

Скопируй `logic/` и `tests/` в проект, добавь свои тесты в `tests/test_main.c`
и запускай `make test` локально и в CI. Тестируемый код не должен включать `furi.h`.
