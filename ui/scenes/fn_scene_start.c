#include "../../fn_test_app_i.h"
#include "fn_scene.h"

void fn_scene_start_on_enter(void* context) {
    furi_assert(context);
    FNApp* app = context;

    view_dispatcher_switch_to_view(app->view_dispatcher, FNAppViewMenu);
}

bool fn_scene_start_on_event(void* context, SceneManagerEvent event) {
    furi_assert(context);
    FNApp* app = context;
    UNUSED(app);
    UNUSED(event);

    /* Переходы по пунктам меню подключаются в задачах историй US1-US5. */
    return false;
}

void fn_scene_start_on_exit(void* context) {
    furi_assert(context);
    UNUSED(context);
    /* Выбранный пункт хранится в модели FnMenuView и сохраняется сам. */
}
