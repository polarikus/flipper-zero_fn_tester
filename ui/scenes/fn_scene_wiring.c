#include "../../fn_test_app_i.h"
#include "fn_scene.h"

void fn_scene_wiring_on_enter(void* context) {
    furi_assert(context);
    FNApp* app = context;
    fn_view_text_set_image(app->view_text, &I_Wiring_UART_128x64);
    view_dispatcher_switch_to_view(app->view_dispatcher, FNAppViewText);
}

bool fn_scene_wiring_on_event(void* context, SceneManagerEvent event) {
    furi_assert(context);
    UNUSED(context);
    UNUSED(event);
    return false;
}

void fn_scene_wiring_on_exit(void* context) {
    furi_assert(context);
    UNUSED(context);
}
