#include "../../fn_test_app_i.h"
#include "fn_scene.h"

/* Заглушка Фазы 2: сценарий подтверждения подключается в US3 (T062). */
void fn_scene_confirm_on_enter(void* context) {
    furi_assert(context);
    FNApp* app = context;
    fn_view_confirm_set(app->view_confirm, FN_STR_CONFIRM_TITLE, FN_STR_CONFIRM_BODY);
    view_dispatcher_switch_to_view(app->view_dispatcher, FNAppViewConfirm);
}

bool fn_scene_confirm_on_event(void* context, SceneManagerEvent event) {
    furi_assert(context);
    UNUSED(context);
    UNUSED(event);
    return false;
}

void fn_scene_confirm_on_exit(void* context) {
    furi_assert(context);
    UNUSED(context);
}
