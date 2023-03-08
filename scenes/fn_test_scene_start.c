#include "../fn_test_app_i.h"

typedef enum {
    FNTestSceneStartSubmenuIndexFNInfo,
    FNTestSceneStartSubmenuIndexLastDocInfo,
    FNTestSceneStartSubmenuIndexRegInfo,
    FNTestSceneStartSubmenuIndexSessionInfo,
    FNTestSceneStartSubmenuIndexTestSell,
    FNTestSceneStartSubmenuIndexCloseSession,
    FNTestSceneStartSubmenuIndexCloseFN,
    FNTestSceneStartSubmenuIndexFlashMGM,
    FNTestSceneStartSubmenuIndexWiring,
    FNTestSceneStartSubmenuIndexAbout,
    FNTestSceneStartSubmenuIndexTest
} FNTestSceneStartSubmenuIndex;

static void fn_test_scene_start_submenu_callback(void* context, uint32_t index) {
    FNApp * app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void fn_test_scene_start_on_enter(void* context) {
    FNApp* app = context;
    submenu_add_item(
        app->submenu,
        "FN Info",
        FNTestSceneStartSubmenuIndexFNInfo,
        fn_test_scene_start_submenu_callback,
        app);
    submenu_add_item(
        app->submenu,
        "Wiring",
        FNTestSceneStartSubmenuIndexWiring,
        fn_test_scene_start_submenu_callback,
        app);
    submenu_add_item(
        app->submenu,
        "About",
        FNTestSceneStartSubmenuIndexAbout,
        fn_test_scene_start_submenu_callback,
        app);
#if APP_FN_DEBUG
    submenu_add_item(
        app->submenu,
        "Test",
        FNTestSceneStartSubmenuIndexTest,
        fn_test_scene_start_submenu_callback,
        app);
#endif
    submenu_set_selected_item(
        app->submenu, scene_manager_get_scene_state(app->scene_manager, FNAppSceneStart));
    view_dispatcher_switch_to_view(app->view_dispatcher, FNTestViewSubmenu);
}

bool fn_test_scene_start_on_event(void* context, SceneManagerEvent event) {
    FNApp* app = context;
    bool success = false;
    if(event.type == SceneManagerEventTypeCustom) {
        scene_manager_set_scene_state(app->scene_manager, FNAppSceneStart, event.event);
        if(event.event == FNTestSceneStartSubmenuIndexWiring) {
            scene_manager_next_scene(app->scene_manager, FNAppSceneWiring);
            success = true;
        }else if(event.event == FNTestSceneStartSubmenuIndexAbout) {
            scene_manager_next_scene(app->scene_manager, FNAppSceneAbout);
            success = true;
        } else if(event.event == FNTestSceneStartSubmenuIndexFNInfo){
            scene_manager_next_scene(app->scene_manager, FNAppSceneDetect);
            success = true;
        } else if(event.event == FNTestSceneStartSubmenuIndexTest){
            scene_manager_next_scene(app->scene_manager, FNAppSceneTest);
            success = true;
        }
    }
    return success;
}

void fn_test_scene_start_on_exit(void* context) {
    FNApp* app = context;
    submenu_reset(app->submenu);
}
