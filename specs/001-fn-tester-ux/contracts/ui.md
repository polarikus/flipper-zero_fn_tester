# Контракт: UI (экраны, сцены, вёрстка, анимация)

**Слой**: `ui/` (`views/`, `scenes/`, `fn_strings.h`, `fn_font.*`, `fn_ui.h`,
`fn_text_model.h`, `fn_format.*`)
**Источник требований**: FR-001…FR-029, US1–US5, `data-model.md` §5, `research.md` D7.

## 1. Состав экранов и view

| View ID (`FNAppView`) | View | Назначение |
|---|---|---|
| `FNAppViewMenu` | `FnMenuView` | главное меню, 5 пунктов |
| `FNAppViewWait` | `FnWaitView` | определение ФН, ожидание данных, очистка МГМ |
| `FNAppViewText` | `FnTextView` | информация, результат, ошибка, справка |
| `FNAppViewConfirm` | `FnConfirmView` | подтверждение очистки МГМ |

Стандартные модули (`submenu`, `widget`, `popup`, `dialog_ex`, `text_input`) не
регистрируются в `view_dispatcher` и не создаются в `FNApp`.

## 2. API view (шаблон)

```c
// Общий шаблон: alloc / free / get_view / set_callback
FnMenuView* fn_view_menu_alloc(void);
void        fn_view_menu_free(FnMenuView* view);
View*       fn_view_menu_get_view(FnMenuView* view);

typedef void (*FnMenuCallback)(void* ctx, uint8_t index);
void fn_view_menu_set_callback(FnMenuView* view, FnMenuCallback cb, void* ctx);
void fn_view_menu_set_selected(FnMenuView* view, uint8_t index);
uint8_t fn_view_menu_get_selected(const FnMenuView* view);

// Текст
typedef enum { FnTextStyleNormal, FnTextStyleHeader, FnTextStyleWarning } FnTextStyle;
FnTextView* fn_view_text_alloc(void);
void        fn_view_text_free(FnTextView* view);
View*       fn_view_text_get_view(FnTextView* view);
void        fn_view_text_clear(FnTextView* view);                       // сброс секций
void        fn_view_text_add_section(FnTextView* view, const char* title);
void        fn_view_text_add_line(FnTextView* view, FnTextStyle style,
                                  const char* fmt, ...);                // перенос по ширине
void        fn_view_text_commit(FnTextView* view);                      // пересчёт высоты, scroll=0
void        fn_view_text_set_model(FnTextView* view, const FnTextModel* model); // рендер модели из fn_format
void        fn_view_text_set_image(FnTextView* view, const Icon* icon); // полноэкранная иконка (распиновка)

// Ожидание
typedef enum { FnWaitModeDetect, FnWaitModeRead, FnWaitModeFlash } FnWaitMode;
FnWaitView* fn_view_wait_alloc(void);
void        fn_view_wait_free(FnWaitView* view);
View*       fn_view_wait_get_view(FnWaitView* view);
void        fn_view_wait_set(FnWaitView* view, FnWaitMode mode,
                             const char* title, const char* subtitle, bool cancelable);

// Подтверждение
typedef void (*FnConfirmCallback)(void* ctx, bool confirmed);
FnConfirmView* fn_view_confirm_alloc(void);
void           fn_view_confirm_free(FnConfirmView* view);
View*          fn_view_confirm_get_view(FnConfirmView* view);
void           fn_view_confirm_set(FnConfirmView* view, const char* title, const char* body);
void           fn_view_confirm_set_callback(FnConfirmView* view, FnConfirmCallback cb, void* ctx);
```

Правила:
- callback-и view не вызывают `scene_manager_*`; они отправляют custom event в
  dispatcher (через контекст `FNApp`).
- Все строки — из `ui/fn_strings.h`; литералы в сценах запрещены (кроме
  технических значений: SN, версия ФФД, версия ПО, URL).
- `malloc` во view — только в `alloc`; в draw/input аллокаций нет.
- `ui/fn_format.c` заполняет `FnTextModel` и НЕ вызывает методы view; сцена
  передаёт модель в `fn_view_text_set_model()`. Файлы `ui/fn_text_model.h`,
  `ui/fn_format.c`, `ui/fn_strings.h` не включают `gui/`, `furi` и `fn_view_*`
  — это условие их host-тестируемости (`tests/host`, `check_layers.sh`).

## 3. Сцены и переходы

| Сцена | Вход | Выход |
|---|---|---|
| `FNAppSceneStart` | меню | `Wait`, `Wiring`, `About`; Back → выход из приложения |
| `FNAppSceneWait` | операция `Detect`/`ReadInfo`/`ReadLife`/`FlashMgm` | `Info`, `Life`, `Confirm` (если МГМ), `Result`; Back (только отменяемые) → `Start` |
| `FNAppSceneConfirm` | выбран пункт «Очистка МГМ» и ФН — МГМ | `Wait(Flash)` при подтверждении; `Start` при отказе/Back |
| `FNAppSceneInfo` | `ReadInfo` Ok | `Start` (Back/Left) |
| `FNAppSceneLife` | `ReadLife` Ok | `Start` (Back/Left) |
| `FNAppSceneResult` | ошибка/нет ответа/успех очистки/«не МГМ» | `Start` (Back/Left) |
| `FNAppSceneWiring` | меню | `Start` |
| `FNAppSceneAbout` | меню | `Start` |

Поток «Очистка МГМ»:

```text
Start ──> Wait(Detect)
            ├─ NoResponse/ошибка ──> Result(ошибка) ──> Start
            ├─ Ok и !is_mgm     ──> Result(«недоступно») ──> Start
            └─ Ok и is_mgm      ──> Confirm
                                      ├─ Отмена/Back ──> Start
                                      └─ Подтверждено ──> Wait(Flash) ──> Result ──> Start
```

Инварианты сцен:
- Каждая сцена в `on_enter` вызывает `view_dispatcher_switch_to_view` и
  (при необходимости) запускает операцию; в `on_exit` — останавливает
  анимацию, при отменяемой операции вызывает `fn_worker_cancel()`.
- `on_event` обрабатывает `SceneManagerEventTypeCustom` с
  `FNCustomEventWorkerResult`; проверяет `result.id`.
- Сцена не хранит указатели на данные, которые освобождает другая сцена.

## 4. Back: матрица

| Экран | Состояние | Действие Back |
|---|---|---|
| Меню | — | выход из приложения |
| Ожидание (`Detect`/`Read`) | идёт операция | `fn_worker_cancel()`, переход в меню ≤1 с (SC-003) |
| Ожидание (`Flash`) | идёт очистка | **игнорируется**, ждём результат/30 с (FR-015) |
| Подтверждение | — | отказ, переход в меню, команда не отправлена |
| Информация / Результат / Справка | любая прокрутка | сразу в меню (FR-025) |
| Сцена результата «не МГМ» | — | в меню (FR-024) |

## 5. Вёрстка 128×64

`ui/fn_ui.h`:

| Константа | Значение | Назначение |
|---|---|---|
| `FN_UI_MARGIN_X` | 2 | левый/правый отступ |
| `FN_UI_HEADER_H` | 12 | заголовок экрана (инверсная плашка) |
| `FN_UI_CONTENT_Y` | 13 | начало контента |
| `FN_UI_CONTENT_H` | 50 | высота области контента |
| `FN_UI_LINE_H` | 9 | строка основного шрифта |
| `FN_UI_SMALL_H` | 7 | строка мелкого шрифта |
| `FN_UI_SECTION_GAP` | 3 | отступ между секциями |
| `FN_UI_SCROLLBAR_W` | 3 | ширина скроллбара |
| `FN_UI_TEXT_W` | 120 | доступная ширина текста |

Правила:
- Текст рисуется только чёрным по белому (без XOR, кроме анимации).
- Длинная строка переносится по словам; если слово шире `FN_UI_TEXT_W` —
  переносится посимвольно, но не обрезается.
- Заголовок секции не остаётся внизу экрана без хотя бы одной строки
  (перенос на следующую позицию).
- Прокрутка: шаг `FN_UI_LINE_H`, `elements_scrollbar` справа, если
  `content_height > FN_UI_CONTENT_H`.
- Отдельного «статус-бара» нет (`ViewDispatcherTypeFullscreen`).

## 6. Шрифт

| Параметр | Значение |
|---|---|
| Файлы | `ui/fn_font.c`, `ui/fn_font.h` (сгенерированный u8g2-массив) |
| Глифы | ASCII 0x20–0x7E, кириллица А–я, Ё/ё, `№`, `—`, `…`, `«»` |
| Размеры | основной (высота 9 px), мелкий (высота 7 px) |
| API | `canvas_set_custom_u8g2_font(canvas, fn_font_primary())` |
| Кодировка строк | исходники в UTF-8; шрифт генерируется с Unicode-таблицей (u8g2), иначе коды символов не совпадут с глифами |
| Бюджет | ≤6 КБ на оба размера |
| Генерация | BDF → `bdfconv` (u8g2), команда и исходник описаны в `docs/ui.md` |

Запрещено использовать стандартные `FontPrimary/FontSecondary` для
произвольного русского текста: штатные шрифты не гарантируют полного набора
кириллицы. На этапе R6 это проверяется контрольной строкой
«АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ» на экране; при отсутствии глифов весь
русский текст рисуется только `fn_font`. Технические значения (SN, `1.2`,
URL) могут рисоваться тем же шрифтом.

## 7. Анимация подключения ФН

| Требование | Реализация |
|---|---|
| Иконка | `A_FNLooking_64x64` (существующий ассет, не менять) |
| Где | `FnWaitView`, режимы `Detect`, `Read`, `Flash` |
| Запуск | `view_set_enter_callback` → `icon_animation_start` |
| Остановка | `view_set_exit_callback` → `icon_animation_stop` |
| Привязка | `view_tie_icon_animation` в `alloc` |
| Освобождение | `icon_animation_free` в `free` |
| Рисование | `canvas_draw_icon_animation(canvas, 0, 0, model->icon)` в draw |
| Перерисовка | модель не меняет иконку; кадры анимации обновляет GUI |

Инварианты:
- Анимация не запускается повторно при обновлении текста (`fn_view_wait_set`),
  чтобы не сбрасывать фазу.
- При переходе на результат/меню анимация остановлена (нет «замороженных»
  кадров и лишней нагрузки).
- Анимация не блокирует GUI-поток: никаких `furi_delay_ms` в draw/enter/exit.

## 8. Содержимое экранов (блоки)

### 8.1 «Информация об ФН» (US1)

Секции (порядок фиксирован, блоки не перемешиваются):

1. **«ФН (30h)»**: серийный номер; этап; состояние смены; текущий документ для
   ОФД и «данные получены / не получены»; для этапа 2 — номер смены и чека;
   номер и дата последнего ФД (этапы 2–3).
2. **«Предупреждения»** (только при `warn_flags != 0`): известные флаги
   по-русски, затем `Прочие предупреждения: 0xNN`.
3. **«ФФД (3Ah)»**: текущая и максимальная версия.
4. **«ПО ФН (33h)»**: версия; признак «МГМ (тестовый) / боевой».

Правило: отсутствующие данные — строка «нет данных», а не пропуск блока
(кроме блока предупреждений, который скрывается при нулевых флагах).
Для этапов 1 («готов к регистрации») и 4 («архив закрыт») блок «Последний ФД»
не выводится вовсе — документов ещё/уже нет, пустые значения показывать
запрещено (US1, сценарий 3).

### 8.2 «Информация о фазе жизни ФН» (US2)

1. **«Срок действия (32h)»**: дата окончания (или «не активизирован»);
   выполнено регистраций; осталось регистраций.
2. **«Остаток срока (3Bh)»**: дней до окончания или «нет данных».
3. **«Ресурс памяти (3Dh)»**: 5-летний ресурс (документы), 30-дневный ресурс
   (КБ), при `has_marking` — «Область уведомлений о маркировке: N %».

Блоки 2 и 3 показываются всегда (независимо от ФФД и этапа), значения —
«нет данных» при отказе (FR-011, FR-028).

### 8.3 «Ожидание»

- `Detect`: «Поиск ФН…», подзаголовок «Проверьте подключение».
- `Read`: «Ожидание ответа ФН…», подзаголовок «Назад — вернуться в меню».
- `Flash`: «Очистка МГМ…», подзаголовок «Может занять до 30 секунд,
  не прерывайте».

### 8.4 «Результат»

- Успех очистки: «Очистка МГМ выполнена успешно».
- Отказ ФН: русский заголовок кода + подсказка + «Код: 0xNN».
- Нет ответа: «ФН не отвечает» + «Проверьте подключение и повторите».
- Неизвестный код: «Неизвестный код ответа ФН: 0xNN».
- Не МГМ: «Очистка недоступна: ФН не является МГМ».
- Битый кадр: «Повреждённый ответ ФН».

### 8.5 «Распиновка» и «О приложении» (US5)

- Распиновка — существующая иконка `I_Wiring_UART_128x64` через
  `fn_view_text_set_image()` (полноэкранная картинка, секции не рисуются);
  связь с ФН не нужна, Back работает как на остальных экранах.
- «О приложении»: название, версия `fap_version`, автор, URL проекта, краткое
  русское описание возможностей.

## 9. Каталог строк

`ui/fn_strings.h` содержит все пользовательские строки; обязательный минимум:

| Группа | Примеры |
|---|---|
| Меню | `FN_STR_MENU_FN_INFO`, `FN_STR_MENU_LIFE_INFO`, `FN_STR_MENU_FLASH_MGM`, `FN_STR_MENU_WIRING`, `FN_STR_MENU_ABOUT` |
| Поля | `FN_STR_SN`, `FN_STR_STAGE`, `FN_STR_SESSION`, `FN_STR_DOC`, `FN_STR_FFD`, `FN_STR_FW`, `FN_STR_MGM` |
| Значения | `FN_STR_YES`, `FN_STR_NO`, `FN_STR_NO_DATA`, `FN_STR_DOC_NONE`, `FN_STR_DOC_DATA_RECEIVED`, `FN_STR_DOC_NO_DATA` |
| Ожидание | `FN_STR_WAIT_DETECT`, `FN_STR_WAIT_READ`, `FN_STR_WAIT_FLASH`, `FN_STR_WAIT_FLASH_HINT` |
| Результат | `FN_STR_RESULT_OK`, `FN_STR_NO_RESPONSE`, `FN_STR_BAD_FRAME`, `FN_STR_NOT_MGM`, `FN_STR_UNKNOWN_CODE_FMT` |
| Ошибки | тексты — из `fn_error_title/hint`, не дублируются в UI |

Проверки (host-тест): все строки непустые; нет строк с латиницей в
пользовательских подписях, кроме разрешённого списка (SN, ФФД, URL, «Код: 0xNN»).

## 10. Приёмка UI

| Сценарий | Критерий |
|---|---|
| US1 | экран ожидания с анимацией → данные на русском; Back → меню |
| US2 | блоки 32h/3Bh/3Dh видны независимо от ФФД; «нет данных» при отказе |
| US3 | подтверждение обязательно; отказ → меню; успех/отказ/нет ответа — понятный текст; Back во время flash игнорируется |
| US4 | Back во время чтения → меню ≤1 с; ошибка/таймаут → результат; повторная операция успешна |
| US5 | распиновка и «О приложении» открываются без ФН и возвращают в меню |
| Читаемость | 100 % строк читаются на 128×64 при RPC-снимке; нет «полосок» вместо букв |
| Прокрутка | все обязательные поля доступны; заголовки не оторваны от строк |
