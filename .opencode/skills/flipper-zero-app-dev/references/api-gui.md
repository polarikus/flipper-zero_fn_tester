# GUI: Canvas, ViewPort, View, ViewDispatcher, SceneManager

Экран: **128×64**, монохром (1 бит/пиксель). GUI-сервис живёт в отдельном потоке;
все draw/input callback-и вызываются **в GUI-потоке**, их нельзя блокировать.

## Слои и ViewPort

`ViewPort` — прямоугольная область, которая умеет рисовать (`draw_callback`) и принимать
ввод (`input_callback`). Несколько ViewPort-ов образуют стек отрисовки (снизу вверх).

```c
#include <gui/gui.h>

static void app_draw_callback(Canvas* canvas, void* context) {
    App* app = context;
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignCenter, "Hello");
}

static void app_input_callback(InputEvent* event, void* context) {
    App* app = context;
    if(event->type == InputTypeShort && event->key == InputKeyBack) {
        // попросить приложение завершиться (флаг/событие/остановка цикла)
    }
}

ViewPort* vp = view_port_alloc();
view_port_draw_callback_set(vp, app_draw_callback, app);
view_port_input_callback_set(vp, app_input_callback, app);
Gui* gui = furi_record_open(RECORD_GUI);
gui_add_view_port(gui, vp, GuiLayerFullscreen);
view_port_update(vp);            // запросить перерисовку
...
gui_remove_view_port(gui, vp);   // обязательно перед free
view_port_free(vp);
furi_record_close(RECORD_GUI);
```

- `GuiLayerFullscreen` — весь экран без статус-бара; `GuiLayerWindow` — со статус-баром
  (полезная высота меньше 64); `GuiLayerStatusBarLeft/Right` — для виджетов статус-бара.
- `view_port_enabled_set(vp, false)` — временно скрыть.
- `gui_direct_draw_acquire/release` — монопольная отрисовка (осторожно, отключает ввод
  и отрисовку остальных).
- Из другого потока: `view_port_update` потокобезопасен; `gui_add/remove_view_port` — тоже.

## Canvas

Система координат — (0,0) слева сверху, x вправо, y вниз.

```c
canvas_clear(canvas);
canvas_set_color(canvas, ColorBlack);      // ColorWhite, ColorBlack, ColorXOR
canvas_set_font(canvas, FontPrimary);      // FontSecondary, FontKeyboard, FontBigNumbers
canvas_set_font_direction(canvas, CanvasDirectionLeftToRight);
canvas_invert_color(canvas);
```

Рисование:

```c
canvas_draw_str(canvas, x, y, "text");                  // y — базовая линия!
canvas_draw_str_aligned(canvas, x, y, AlignCenter, AlignCenter, "text");
uint16_t w = canvas_string_width(canvas, "text");
size_t h = canvas_current_font_height(canvas);

canvas_draw_dot(canvas, x, y);
canvas_draw_line(canvas, x1, y1, x2, y2);
canvas_draw_box(canvas, x, y, w, h);        // залитый
canvas_draw_frame(canvas, x, y, w, h);      // рамка
canvas_draw_rbox(canvas, x, y, w, h, r);    // скруглённый залитый
canvas_draw_rframe(canvas, x, y, w, h, r);
canvas_draw_circle(canvas, x, y, r);        // контур
canvas_draw_disc(canvas, x, y, r);          // залитый
canvas_draw_triangle(canvas, x, y, base, height, dir);

canvas_draw_icon(canvas, x, y, &I_icon);            // из assets_icons.h / app_icons.h
canvas_draw_icon_ex(canvas, x, y, &I_icon, IconRotation90);
canvas_draw_bitmap(canvas, x, y, w, h, data);       // XBM, mode: canvas_set_bitmap_mode
canvas_draw_xbm(canvas, x, y, w, h, data);
canvas_draw_glyph(canvas, x, y, ch);
```

Готовые элементы (`<gui/elements.h>`): `elements_progress_bar`,
`elements_progress_bar_with_text`, `elements_scrollbar`, `elements_frame`,
`elements_button_left/right/up/down/center`, `elements_multiline_text_aligned`,
`elements_string_fit_width`, `elements_text_box`, `elements_bubble`.

Правила вёрстки:

- Отступ от краёв 2–4 px; не рисуй под статус-баром, если слой `Window`.
- Длинные строки обрезай заранее (`elements_string_fit_width`) или переноси
  (`elements_multiline_text`), а не полагайся на клиппинг.
- Шрифты 1-битные: сглаживания нет; мелкий текст — только `FontSecondary`.
- Кириллица работает только если в шрифте есть глифы (в стандартных есть ограниченный
  набор); для полной поддержки — свой u8g2-шрифт (`canvas_set_custom_u8g2_font`).
- Обновляй экран только при изменениях, не в цикле без задержки.

## View и модель

`View` — обёртка над ViewPort с моделью и callback-ами. Модель бывает:
`ViewModelTypeNone`, `ViewModelTypeLockFree` (атомарные поля) и `ViewModelTypeLocking`
(мьютекс, GUI-поток блокируется на время доступа).

```c
View* view = view_alloc();
view_set_draw_callback(view, draw_cb);        // Canvas*, model
view_set_input_callback(view, input_cb);      // InputEvent*, model
view_set_custom_callback(view, custom_cb);
view_set_previous_callback(view, previous_cb);
view_allocate_model(view, ViewModelTypeLocking, sizeof(Model));
with_view_model(
    view,
    Model* model,
    { model->counter++; },
    true);                                    // true → перерисовать
view_free(view);
```

- В `draw_cb` нельзя менять модель «в обход» `with_view_model`.
- `view_commit_model(view, true)` из другого потока безопасно запрашивает перерисовку.
- `view_set_enter_callback`/`view_set_exit_callback` — момент показа/скрытия.
- `view_set_orientation`/`view_port_set_orientation` — поворот.

## ViewDispatcher

Держит набор View и переключает их; внутри — `FuriEventLoop` и очереди событий.

```c
ViewDispatcher* vd = view_dispatcher_alloc();
view_dispatcher_set_event_callback_context(vd, app);
view_dispatcher_set_custom_event_callback(vd, custom_cb);      // bool (void*, uint32_t)
view_dispatcher_set_navigation_event_callback(vd, back_cb);    // bool (void*)
view_dispatcher_set_tick_event_callback(vd, tick_cb, 100);     // мс, опционально

view_dispatcher_attach_to_gui(vd, gui, ViewDispatcherTypeFullscreen);
view_dispatcher_add_view(vd, ViewIdMain, widget_get_view(widget));
view_dispatcher_switch_to_view(vd, ViewIdMain);
view_dispatcher_send_custom_event(vd, MyEvent);   // из любого потока

view_dispatcher_run(vd);   // блокируется, пока не view_dispatcher_stop(vd)

view_dispatcher_remove_view(vd, ViewIdMain);
view_dispatcher_free(vd);
```

- `view_dispatcher_enable_queue()` — **deprecated**, не вызывай: очереди включены всегда.
- Если navigation-callback вернул `false`, dispatcher останавливается (выход из приложения).
- `view_dispatcher_alloc_ex(loop)` — использовать внешний `FuriEventLoop`;
  `view_dispatcher_get_event_loop(vd)` — встроенный.
- `ViewDispatcherTypeWindow` — со статус-баром, `Fullscreen` — без, `Desktop` — внутренний.
- `view_dispatcher_switch_to_view(vd, VIEW_NONE)` — скрыть всё; `VIEW_IGNORE` — ignore back.

## ViewHolder (один экран)

```c
ViewHolder* vh = view_holder_alloc();
view_holder_attach_to_gui(vh, gui);
view_holder_set_view(vh, text_box_get_view(text_box));
view_holder_set_back_callback(vh, back_cb, context);
view_holder_set_free_callback(vh, free_cb, context);
...
view_holder_set_view(vh, NULL);   // или заменить
view_holder_free(vh);
```

`ViewHolder` — адаптер View → ViewPort. Подходит для одного экрана без навигации;
сам по себе не блокирует поток — жди события через `FuriEventLoop`/`api_lock`.
Нельзя одновременно держать View в ViewDispatcher и ViewHolder.

## SceneManager

Сцена = экран + логика. Каркас на макросах (см. `examples/scene-app`):

```c
// my_app_scene_config.h
ADD_SCENE(my_app, start, Start)
ADD_SCENE(my_app, about, About)

// my_app_scene.h
#define ADD_SCENE(prefix, name, id) MyAppScene##id,
typedef enum {
#include "my_app_scene_config.h"
    MyAppSceneNum,
} MyAppScene;
#undef ADD_SCENE
extern const SceneManagerHandlers my_app_scene_handlers;
// + генерация объявлений on_enter/on_event/on_exit тем же макросом
```

Обработчики:

```c
const SceneManagerHandlers my_app_scene_handlers = {
    .on_enter_handlers = my_app_scene_on_enter_handlers,
    .on_event_handlers = my_app_scene_on_event_handlers,
    .on_exit_handlers = my_app_scene_on_exit_handlers,
    .scene_num = MyAppSceneNum,
};
```

`on_enter` — создать/настроить view и переключиться на него; `on_exit` — сбросить view,
остановить активность сцены; `on_event` — обработать custom/back/tick и вернуть `true`,
если событие поглощено.

```c
bool my_app_scene_start_on_event(void* context, SceneManagerEvent event) {
    MyApp* app = context;
    bool consumed = false;
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == MyAppEventAbout) {
            scene_manager_next_scene(app->scene_manager, MyAppSceneAbout);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        // consumed = false → выход из приложения, если сцена корневая
    }
    return consumed;
}
```

- `scene_manager_next_scene`, `scene_manager_previous_scene`,
  `scene_manager_search_and_switch_to_previous_scene` — навигация.
- `scene_manager_handle_back_event` в navigation-callback; `false` → выход.
- `scene_manager_set_scene_state/get_scene_state` — сохранить состояние сцены
  (например, выбранный пункт).
- `scene_manager_handle_tick_event` — если включён tick-callback dispatcher-а.
- `scene_manager_stop()` — остановить сцены (используется при выходе/перезапуске).
- Кастомные события нумеруй в отдельном enum и не пересекай с индексами пунктов меню
  без явного соглашения.

## Модули (готовые View)

Заголовки: `<gui/modules/...>`, у каждого `alloc/free` и `*_get_view()`.

| Модуль | Назначение | Ключевые функции |
|---|---|---|
| `menu` | вертикальное меню с иконками | `menu_add_item(menu, label, icon, index, cb, ctx)` |
| `submenu` | список пунктов | `submenu_add_item`, `submenu_add_item_ex`, `submenu_set_header` |
| `variable_item_list` | список с настройкой значений | `variable_item_list_add`, `variable_item_set_current_value_text/index` |
| `widget` | статичный экран | `widget_add_string_element`, `..._multiline`, `..._text_box`, `..._button`, `..._icon`, `..._rect/circle/line` |
| `popup` | сообщение/подтверждение с таймаутом | `popup_set_header/text/icon`, `popup_set_timeout`, `popup_set_callback` |
| `dialog_ex` | диалог с 3 кнопками | `dialog_ex_set_header/text`, `..._set_left/center/right_button_text`, `dialog_ex_set_result_callback` |
| `text_input` | ввод строки | `text_input_set_result_callback(input, cb, ctx, buf, size, clear)`, `text_input_set_validator` |
| `number_input` | ввод числа | `number_input_set_result_callback(input, cb, ctx, value, min, max)` |
| `byte_input` | ввод байтов | `byte_input_set_result_callback(input, cb, changed_cb, ctx, bytes, count)` |
| `text_box` | скроллируемый текст | `text_box_set_text`, `text_box_set_font` (`TextBoxFontText`/`TextBoxFontHex`), `text_box_set_focus` (`TextBoxFocusStart/End`) |
| `loading` | индикатор загрузки | `loading_alloc/get_view` |
| `empty_screen` | пустой экран | `empty_screen_alloc/get_view` |
| `button_menu` | горизонтальные кнопки | `button_menu_add_item` |
| `button_panel` | матрица кнопок/иконок | `button_panel_reserve`, `button_panel_add_item` |
| `file_browser` | выбор файла (View) | `file_browser_alloc`, `file_browser_configure` |

Валидаторы: `<gui/modules/validators.h>` — `validator_is_file_alloc_init`,
`validator_is_file_callback`.

Строковые поля ввода (`text_input`) требуют **статического/долгоживущего** буфера:
модуль хранит указатель, а не копию.

## Диалоги (без ViewDispatcher)

`DialogsApp` — синхронный API: пока пользователь не нажмёт кнопку, код не продолжится.

```c
DialogsApp* dialogs = furi_record_open(RECORD_DIALOGS);

DialogMessage* msg = dialog_message_alloc();
dialog_message_set_header(msg, "Title", 64, 10, AlignCenter, AlignTop);
dialog_message_set_text(msg, "Text", 64, 30, AlignCenter, AlignTop);
dialog_message_set_buttons(msg, "No", NULL, "Yes");
DialogMessageButton result = dialog_message_show(dialogs, msg);
dialog_message_free(msg);

dialog_message_show_storage_error(dialogs, "SD error");
furi_record_close(RECORD_DIALOGS);
```

File browser:

```c
FuriString* path = furi_string_alloc_set(APP_DATA_PATH(""));
DialogsFileBrowserOptions options;
dialog_file_browser_set_basic_options(&options, ".txt", NULL);
if(dialog_file_browser_show(dialogs, path, path, &options)) { /* выбрано */ }
furi_string_free(path);
```

## Ввод

`InputEvent` (`<input/input.h`): `key` (`InputKeyUp/Down/Right/Left/Ok/Back`) и `type`
(`InputTypePress/Release/Short/Long/Repeat`). Обычно достаточно `Short` и `Long`.
`InputEvent` из View/ViewPort приходит в GUI-потоке. На устройстве без GUI-фокуса
клавиши не придут.

## Иконки и анимации

- Иконки из `fap_icon_assets` доступны как `&I_name` после `#include "my_app_icons.h"`.
- Анимированные иконки — имя с префиксом `A_`, шрифт/названия по конвенции assets.
- `icon_animation_alloc(&A_name)`, `icon_animation_start/stop`,
  `icon_animation_set_update_callback`, `view_tie_icon_animation(view, anim)`.
- Иконка приложения `fap_icon` — 10×10 PNG, 1-битная палитра.

## Полезные ограничения GUI

- Никаких `furi_delay_ms` и долгих операций в callback-ах GUI.
- Не создавай/не удаляй View во время draw; делай это в `on_enter`/по событию.
- `canvas_draw_str` использует baseline — для рамок и выравнивания удобнее
  `canvas_draw_str_aligned`.
- Все строки в API — `const char*`; для форматирования используй `snprintf` в буфер
  или `FuriString` + `furi_string_get_cstr`.
