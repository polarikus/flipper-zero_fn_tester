#include "../my_app_i.h"

void my_app_scene_about_on_enter(void* context) {
    furi_assert(context);
    MyApp* app = context;

    widget_reset(app->widget);
    widget_add_string_element(
        app->widget, 64, 10, AlignCenter, AlignTop, FontPrimary, "My App");
    widget_add_string_multiline_element(
        app->widget,
        64,
        32,
        AlignCenter,
        AlignCenter,
        FontSecondary,
        "Template with\nscenes and views");

    view_dispatcher_switch_to_view(app->view_dispatcher, MyAppViewWidget);
}

bool my_app_scene_about_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    // Back не поглощаем: SceneManager сам вернётся на предыдущую сцену.
    return false;
}

void my_app_scene_about_on_exit(void* context) {
    furi_assert(context);
    MyApp* app = context;
    widget_reset(app->widget);
}
