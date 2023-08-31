//
// Created by Игорь Данилов on 12.03.2023.
//
#include "../fn_test_app_i.h"

static void
    fn_test_scene_timeout_widget_callback(GuiButtonType result, InputType type, void* context) {
    FNApp* app = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(app->view_dispatcher, result);
    }
}

void fn_test_scene_timeout_on_enter(void* context) {
    FNApp* app = context;
    widget_add_button_element(
        app->widget, GuiButtonTypeLeft, "Back", fn_test_scene_timeout_widget_callback, app);
    widget_add_string_element(
        app->widget, 64, 9, AlignCenter, AlignBottom, FontPrimary, "Error read answer from FN");
    widget_add_string_element(
        app->widget, 64, 21, AlignCenter, AlignBottom, FontSecondary, "Timeout wait data from FN");
    view_dispatcher_switch_to_view(app->view_dispatcher, FNTestViewWidget);
}

bool fn_test_scene_timeout_on_event(void* context, SceneManagerEvent event) {
    FNApp* app = context;
    bool success = false;
    if(event.type == SceneManagerEventTypeBack) {
        success = true;
        scene_manager_search_and_switch_to_previous_scene(app->scene_manager, FNAppSceneStart);
    } else if(event.type == SceneManagerEventTypeCustom) {
        success = true;
        if(event.event == GuiButtonTypeLeft) {
            scene_manager_search_and_switch_to_previous_scene(app->scene_manager, FNAppSceneStart);
        }
    }
    return success;
}
void fn_test_scene_timeout_on_exit(void* context) {
    FNApp* app = context;
    widget_reset(app->widget);
}