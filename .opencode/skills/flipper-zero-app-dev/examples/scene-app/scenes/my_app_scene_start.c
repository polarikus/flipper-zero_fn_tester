#include "../my_app_i.h"
#include "../my_app_custom_event.h"

typedef enum {
    MyAppSceneStartItemAbout,
    MyAppSceneStartItemExit,
} MyAppSceneStartItem;

static void my_app_scene_start_submenu_callback(void* context, uint32_t index) {
    furi_assert(context);
    MyApp* app = context;

    switch(index) {
    case MyAppSceneStartItemAbout:
        view_dispatcher_send_custom_event(app->view_dispatcher, MyAppCustomEventAbout);
        break;
    case MyAppSceneStartItemExit:
        view_dispatcher_send_custom_event(app->view_dispatcher, MyAppCustomEventExit);
        break;
    default:
        break;
    }
}

void my_app_scene_start_on_enter(void* context) {
    furi_assert(context);
    MyApp* app = context;

    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, "My App");
    submenu_add_item(
        app->submenu, "About", MyAppSceneStartItemAbout, my_app_scene_start_submenu_callback, app);
    submenu_add_item(
        app->submenu, "Exit", MyAppSceneStartItemExit, my_app_scene_start_submenu_callback, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, MyAppViewSubmenu);
}

bool my_app_scene_start_on_event(void* context, SceneManagerEvent event) {
    furi_assert(context);
    MyApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == MyAppCustomEventAbout) {
            scene_manager_next_scene(app->scene_manager, MyAppSceneAbout);
            consumed = true;
        } else if(event.event == MyAppCustomEventExit) {
            // Остановить dispatcher → view_dispatcher_run() вернётся, приложение выйдет.
            view_dispatcher_stop(app->view_dispatcher);
            consumed = true;
        }
    }
    // Back не обрабатываем: на корневой сцене scene_manager_previous_scene()
    // вернёт false, navigation-callback тоже false → приложение закроется.
    return consumed;
}

void my_app_scene_start_on_exit(void* context) {
    furi_assert(context);
    MyApp* app = context;
    submenu_reset(app->submenu);
}
