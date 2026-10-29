# Пример: UI-тест FAP по USB (RPC, protobuf)

Минимальный помощник для автоматизации приёмочных сценариев на реальном устройстве:
снимки экрана, нажатия, ожидание состояния по картинке, чтение/запись файлов на SD.

Подробное описание приёмов, ограничений и подводных камней —
`references/rpc-ui-testing.md`.

## Установка

```bash
python3.11 -m venv ~/.venvs/flipper-rpc
~/.venvs/flipper-rpc/bin/pip install flipperzero-protobuf pillow
```

## Быстрый старт

```bash
# картинка для глаз
~/.venvs/flipper-rpc/bin/python flipper_rpc.py /dev/cu.usbmodemflip_XXXX menu.png
# кадр текстом: мелкий шрифт должен читаться, иначе разбор кадра неверный
~/.venvs/flipper-rpc/bin/python flipper_rpc.py /dev/cu.usbmodemflip_XXXX --ascii
```

`menu.png` — текущий кадр экрана, увеличенный в 4 раза.

Если вместо букв видны полоски, а иконки/логотип выглядят нормально — испорчена **не**
прошивка и не приложение, а раскладка кадра в скрипте. Верная формула и типичные неверные
варианты — `references/rpc-ui-testing.md`, §3.1.

## Пример сценария

Сценарий «нажать Start, дождаться конца отсчёта, проверить результат»:

```python
from flipper_rpc import connect, press, save_snapshot, wait_for_frac

fp = connect("/dev/cu.usbmodemflip_XXXX")

press(fp, "OK", wait=1.0)            # войти в Presets
press(fp, "DOWN", wait=0.2)          # выбрать второй пресет
press(fp, "OK", wait=1.0)            # открыть карточку
press(fp, "OK", wait=0.0)            # Start

rect = (40, 92, 17, 28)              # плашка "Time is up!" на экране DONE
elapsed = wait_for_frac(fp, rect, 0.6, timeout=70)
print("отсчёт завершён через", elapsed, "с")

save_snapshot(fp, "done.png")        # глазами проверить вёрстку
```

Порог `0.6` и прямоугольник подбираются один раз по ASCII-дампу кадра — как это делать,
описано в `references/rpc-ui-testing.md` (§5).

## Чистый прогон и сохранность данных

```python
from flipper_rpc import backup_dir, connect, press, restart_app, restore_dir

PORT = "/dev/cu.usbmodemflip_XXXX"
FAP = "/ext/apps/Tools/my_app.fap"
DATA = "/ext/apps_data/my_app"

backup_dir(connect(PORT), DATA, "presets_bak")   # копия данных пользователя
restart_app(PORT, FAP)                           # RPC закрыт: только pyserial + CLI

fp = connect(PORT)
press(fp, "DOWN", wait=0.2)
press(fp, "OK", wait=1.0)
# … сценарий …

restore_dir(fp, DATA, "presets_bak")             # вернуть и сверить побайтно
```

`restart_app` — единственный надёжный способ получить «первый экран и исходное состояние
модулей»: `rpc_app_exit`/`rpc_app_start` для приложения, запущенного через `ufbt launch`,
отвечают `ERROR_APP_NOT_RUNNING`/`ERROR_APP_SYSTEM_LOCKED`.

## Что важно помнить

- Порт передавать явно: `connect(port)`.
- RPC и `ufbt cli` используют один CDC-порт — одновременно не работают.
- Пути к данным приложения снаружи: `/ext/apps_data/<appid>` (алиас `/data` не
  разворачивается для RPC/CLI), без завершающего слэша. Внутри приложения и в
  `file_browser` те же файлы видны как `/data/<appid>/...` — это нормально.
- Позиции курсора в `number_input` переживают смену сцен: между сценариями перезапускай
  приложение.
- Подсветку, звук, вибро и извлечение SD-карты снимками не проверить — это остаётся
  ручным чек-листом.
