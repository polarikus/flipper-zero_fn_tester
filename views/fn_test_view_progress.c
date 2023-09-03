#include "fn_test_view_progress.h"
#include "fn_test_icons.h"
#include <gui/elements.h>

struct FNProgressView {
    View* view;
    IconAnimation* icon;
    FNProgressViewCallback callback;
    void* cb_ctx;
};

typedef struct {
    IconAnimation* icon;
    FNTestProgressViewMode mode;
} FNDetectViewModel;

View* fn_test_view_progress_get_view(FNProgressView* app) {
    return app->view;
}

static void fn_test_view_progress_draw_callback(Canvas* canvas, void* context) {
    FNDetectViewModel* model = context;
    canvas_set_font(canvas, FontPrimary);
    //canvas_draw_icon_animation(canvas, 0, 0, model->icon);
    if(model->mode == FNProgressViewTypeWaitData) {
        canvas_draw_str_aligned(canvas, 64, 26, AlignLeft, AlignCenter, "Waiting data");
        canvas_draw_str_aligned(canvas, 64, 36, AlignLeft, AlignCenter, "from FN...");
    } else if(model->mode == FNProgressViewTypeFlash) {
        canvas_draw_str_aligned(canvas, 64, 26, AlignLeft, AlignCenter, "Flashing");
        canvas_draw_str_aligned(canvas, 64, 36, AlignLeft, AlignCenter, "MGM...");
    }
}

static void fn_test_view_progress_enter_callback(void* context) {
    FNProgressView* app = context;
    UNUSED(app);
}

static void fn_test_view_progress_exit_callback(void* context) {
    FNProgressView* app = context;
    UNUSED(app);
}

FNProgressView* fn_test_view_progress_alloc() {
    FNProgressView* app = malloc(sizeof(FNProgressView));
    app->view = view_alloc();
    view_set_context(app->view, app);
    view_allocate_model(app->view, ViewModelTypeLocking, sizeof(FNDetectViewModel));
    view_set_draw_callback(app->view, fn_test_view_progress_draw_callback);
    view_set_enter_callback(app->view, fn_test_view_progress_enter_callback);
    view_set_exit_callback(app->view, fn_test_view_progress_exit_callback);
    return app;
}

void fn_test_view_progress_free(FNProgressView* app) {
    view_free(app->view);
    free(app);
}

void fn_test_view_progress_set_mode(FNProgressView* app, FNTestProgressViewMode mode) {
    with_view_model(
        app->view, FNDetectViewModel * model, { model->mode = mode; }, true);
}
