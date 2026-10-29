#include "my_app_i.h"

static bool my_app_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    MyApp* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool my_app_back_event_callback(void* context) {
    furi_assert(context);
    MyApp* app = context;
    // true — событие обработано (переход на предыдущую сцену),
    // false — ViewDispatcher остановится и приложение завершится.
    return scene_manager_handle_back_event(app->scene_manager);
}

static MyApp* my_app_alloc(void) {
    MyApp* app = malloc(sizeof(MyApp));

    app->gui = furi_record_open(RECORD_GUI);

    app->view_dispatcher = view_dispatcher_alloc();
    app->scene_manager = scene_manager_alloc(&my_app_scene_handlers, app);

    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, my_app_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, my_app_back_event_callback);

    view_dispatcher_attach_to_gui(
        app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    app->submenu = submenu_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, MyAppViewSubmenu, submenu_get_view(app->submenu));

    app->widget = widget_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, MyAppViewWidget, widget_get_view(app->widget));

    scene_manager_next_scene(app->scene_manager, MyAppSceneStart);

    return app;
}

static void my_app_free(MyApp* app) {
    furi_assert(app);

    // Сначала убираем views из dispatcher-а, потом освобождаем модули.
    view_dispatcher_remove_view(app->view_dispatcher, MyAppViewSubmenu);
    view_dispatcher_remove_view(app->view_dispatcher, MyAppViewWidget);

    view_dispatcher_free(app->view_dispatcher);
    scene_manager_free(app->scene_manager);

    submenu_free(app->submenu);
    widget_free(app->widget);

    furi_record_close(RECORD_GUI);
    free(app);
}

int32_t my_app_app(void* p) {
    UNUSED(p);

    MyApp* app = my_app_alloc();
    view_dispatcher_run(app->view_dispatcher);
    my_app_free(app);

    return 0;
}
