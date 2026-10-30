#include "../../fn_test_app_i.h"
#include "fn_scene.h"

void fn_scene_wait_on_enter(void* context) {
    furi_assert(context);
    FNApp* app = context;

    fn_view_wait_set(
        app->view_wait, FnWaitModeDetect, FN_STR_WAIT_DETECT, FN_STR_WAIT_DETECT_HINT, true);
    view_dispatcher_switch_to_view(app->view_dispatcher, FNAppViewWait);
}

bool fn_scene_wait_on_event(void* context, SceneManagerEvent event) {
    furi_assert(context);
    FNApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom && event.event == FNAppCustomEventWorkerResult) {
        if(app->op_result.id != app->op_id) {
            FURI_LOG_W(TAG, "поздний результат операции %u", (unsigned)app->op_result.id);
            return true;
        }
        scene_manager_search_and_switch_to_previous_scene(app->scene_manager, FNAppSceneStart);
        consumed = true;
    }
    return consumed;
}

void fn_scene_wait_on_exit(void* context) {
    furi_assert(context);
    FNApp* app = context;

    if(fn_worker_is_busy(app->worker)) {
        fn_worker_cancel(app->worker);
    }
}
