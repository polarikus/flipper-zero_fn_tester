/**
 * Минимальное приложение на одном ViewPort:
 * - рисует кадр в draw-callback (GUI-поток)
 * - обновляет счётчик по таймеру внутри FuriEventLoop
 * - выходит по Back
 */
#include <furi.h>
#include <gui/gui.h>
#include <gui/elements.h>
#include <input/input.h>

#define TAG "ViewPortApp"

typedef struct {
    Gui* gui;
    ViewPort* view_port;
    FuriEventLoop* event_loop;
    FuriEventLoopTimer* timer;
    uint32_t ticks;
} ViewPortApp;

static void view_port_app_draw(Canvas* canvas, void* context) {
    ViewPortApp* app = context;

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_frame(canvas, 0, 0, 128, 64);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 24, AlignCenter, AlignCenter, "ViewPort demo");
    canvas_set_font(canvas, FontSecondary);
    char buf[32];
    snprintf(buf, sizeof(buf), "ticks: %lu", app->ticks);
    canvas_draw_str_aligned(canvas, 64, 42, AlignCenter, AlignCenter, buf);
    elements_button_center(canvas, "Wait");
}

static void view_port_app_input(InputEvent* event, void* context) {
    ViewPortApp* app = context;

    if(event->type == InputTypeShort && event->key == InputKeyBack) {
        FURI_LOG_I(TAG, "exit requested");
        furi_event_loop_stop(app->event_loop);
    }
}

static void view_port_app_tick(void* context) {
    ViewPortApp* app = context;
    app->ticks++;
    view_port_update(app->view_port);
}

int32_t view_port_app(void* p) {
    UNUSED(p);

    ViewPortApp app = {0};
    app.gui = furi_record_open(RECORD_GUI);

    app.view_port = view_port_alloc();
    view_port_draw_callback_set(app.view_port, view_port_app_draw, &app);
    view_port_input_callback_set(app.view_port, view_port_app_input, &app);
    gui_add_view_port(app.gui, app.view_port, GuiLayerFullscreen);

    app.event_loop = furi_event_loop_alloc();
    app.timer = furi_event_loop_timer_alloc(
        app.event_loop, view_port_app_tick, FuriEventLoopTimerTypePeriodic, &app);
    furi_event_loop_timer_start(app.timer, furi_ms_to_ticks(500));

    view_port_update(app.view_port);
    furi_event_loop_run(app.event_loop);

    furi_event_loop_timer_free(app.timer);
    furi_event_loop_free(app.event_loop);

    gui_remove_view_port(app.gui, app.view_port);
    view_port_free(app.view_port);
    furi_record_close(RECORD_GUI);

    return 0;
}
