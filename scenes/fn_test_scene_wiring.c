#include "../fn_test_app_i.h"


void fn_test_scene_wiring_on_enter(void* context) {
    FNApp* app = context;
    widget_add_icon_element(app->widget, 0, 0, &I_Wiring_UART_128x64);
    view_dispatcher_switch_to_view(app->view_dispatcher, FNTestViewWidget);
}

bool fn_test_scene_wiring_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}
void fn_test_scene_wiring_on_exit(void* context) {
    FNApp* app = context;
    widget_reset(app->widget);
}
