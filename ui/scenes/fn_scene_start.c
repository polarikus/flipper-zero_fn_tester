#include "../../fn_test_app_i.h"
#include "fn_scene.h"

#include <string.h>

/* Пункты меню в порядке FR-001: индекс совпадает с custom event от FnMenuView. */
typedef enum {
    FnMenuItemFnInfo = 0,
    FnMenuItemLifeInfo,
    FnMenuItemFlashMgm,
    FnMenuItemWiring,
    FnMenuItemAbout,
} FnMenuItem;

void fn_scene_start_on_enter(void* context) {
    furi_assert(context);
    FNApp* app = context;

    view_dispatcher_switch_to_view(app->view_dispatcher, FNAppViewMenu);
}

static void fn_scene_start_start_info(FNApp* app) {
    /* Перед новой операцией данные и результат прошлого чтения сбрасываются. */
    fn_info_reset(app->fn_info);
    memset(&app->op_result, 0, sizeof(app->op_result));
    app->pending_op = FnOpReadInfo;
    scene_manager_next_scene(app->scene_manager, FNAppSceneWait);
}

bool fn_scene_start_on_event(void* context, SceneManagerEvent event) {
    furi_assert(context);
    FNApp* app = context;

    if(event.type != SceneManagerEventTypeCustom) return false;

    switch(event.event) {
    case FnMenuItemFnInfo:
        fn_scene_start_start_info(app);
        return true;
    default:
        /* Остальные пункты подключаются в задачах US2-US5. */
        return false;
    }
}

void fn_scene_start_on_exit(void* context) {
    furi_assert(context);
    UNUSED(context);
    /* Выбранный пункт хранится в модели FnMenuView и сохраняется сам. */
}
