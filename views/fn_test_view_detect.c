#include "fn_test_view_detect.h"
#include "fn_test_icons.h"
#include <gui/elements.h>

struct FNDetectView {
    View* view;
    IconAnimation* icon;
    FNDetectViewCallback callback;
    void* cb_ctx;
};

typedef struct {
    IconAnimation* icon;
} FNDetectViewModel;

View* fn_test_view_detect_get_view(FNDetectView* app) {
    return app->view;
}

static void fn_test_view_detect_draw_callback(Canvas* canvas, void* context) {
    FNDetectViewModel* model = context;
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_icon_animation(canvas, 0, 0, model->icon);
    canvas_draw_str_aligned(canvas, 64, 26, AlignLeft, AlignCenter, "Detecting");
    canvas_draw_str_aligned(canvas, 64, 36, AlignLeft, AlignCenter, "FN...");
}

static void fn_test_view_detect_enter_callback(void* context) {
    FNDetectView* app = context;
    with_view_model(
        app->view, FNDetectViewModel * model, { icon_animation_start(model->icon); }, false);
}

static void fn_test_view_detect_exit_callback(void* context) {
    FNDetectView* app = context;
    with_view_model(
        app->view, FNDetectViewModel * model, { icon_animation_stop(model->icon); }, false);
}

FNDetectView* fn_test_view_detect_alloc() {
    FNDetectView* app = malloc(sizeof(FNDetectView));
    app->view = view_alloc();
    view_set_context(app->view, app);
    view_allocate_model(app->view, ViewModelTypeLocking, sizeof(FNDetectViewModel));
    with_view_model(
        app->view,
        FNDetectViewModel * model,
        {
            model->icon = icon_animation_alloc(&A_FNLooking_64x64);
            view_tie_icon_animation(app->view, model->icon);
        },
        false);
    view_set_draw_callback(app->view, fn_test_view_detect_draw_callback);
    view_set_enter_callback(app->view, fn_test_view_detect_enter_callback);
    view_set_exit_callback(app->view, fn_test_view_detect_exit_callback);
    return app;
}

void fn_test_view_detect_free(FNDetectView* app) {
    with_view_model(
        app->view, FNDetectViewModel * model, { icon_animation_free(model->icon); }, false);
    view_free(app->view);
    free(app);
}
