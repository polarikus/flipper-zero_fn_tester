#include "../../fn_test_app_i.h"
#include "fn_scene.h"

/* Заглушка Фазы 2: экран результата подключается в US3/US4 (T065, T069). */
void fn_scene_result_on_enter(void* context) {
    furi_assert(context);
    FNApp* app = context;
    fn_view_text_set_model(app->view_text, &app->text_model);
    view_dispatcher_switch_to_view(app->view_dispatcher, FNAppViewText);
}

bool fn_scene_result_on_event(void* context, SceneManagerEvent event) {
    furi_assert(context);
    UNUSED(context);
    UNUSED(event);
    return false;
}

void fn_scene_result_on_exit(void* context) {
    furi_assert(context);
    UNUSED(context);
}
